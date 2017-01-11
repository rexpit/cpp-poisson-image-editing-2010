
#pragma warning( disable : 4996 )	/* fopen の警告抑制 (VC++) */
#include "filter.h"
#include <cmath>	/* sqrt, abs のため */
#include <algorithm>	/* max, min のため */
#include <cstdio>

typedef unsigned char byte;
typedef unsigned int uint;

/* ↓ここから ImageSig_t */

int ImageSig::getData(int x, int y, int colorId) {
	if(x < 0 || x >= static_cast<int>(this->width) || y < 0 || y >= static_cast<int>(this->height)) { return 0; }
	return getData_NoSecure(x, y, colorId);
}

int ImageSig::getData_NoSecure(int x, int y, int colorId) {
	const int idx = y * static_cast<int>(this->width) + x;
	switch(colorId){
		case 0:
			return this->data[idx].r;
		case 1:
			return this->data[idx].g;
		case 2:
			return this->data[idx].b;
		default:
			return 0;
	}
}

void ImageSig::setData(int x, int y, int colorId, int color) {
	if(x < 0 || x >= static_cast<int>(this->width) || y < 0 || y >= static_cast<int>(this->height)) { return; }
	setData_NoSecure(x, y, colorId, color);
	return;
}

void ImageSig::setData_NoSecure(int x, int y, int colorId, int color) {
	const int idx = y * static_cast<int>(this->width) + x;
	switch(colorId){
		case 0:
			this->data[idx].r = color;
			break;
		case 1:
			this->data[idx].g = color;
			break;
		case 2:
			this->data[idx].b = color;
			break;
		default:
			break;
	}
	return;
}

int ImageSig::create_Image(int width, int height) {
	this->free_Image();
	try{
		this->data = new RGBSig[width * height];
	}catch(...){
		this->data = NULL;
		return 1;
	}

	this->width = width;
	this->height = height;

	return 0;
}

void ImageSig::free_Image(void) {
	if(this->data != NULL){
		delete [] this->data;
		this->data = NULL;
	}
	return;
}

int ImageSig::lapFilter(Image *src, int x1, int y1) {
	const char dx[] = {0,-1,0,1,0}, dy[] = {-1,0,0,0,1}, h[] = {1,1,-4,1,1};
	int sum[3];

	if(this->data == NULL){ return 1; }
	const int x2 = x1 + static_cast<int>(this->width) - 1,
		y2 = y1 + static_cast<int>(this->height) - 1;
	if(x1 < 0 || x2 >= static_cast<int>(src->getWidth()) || y1 < 0 || y2 >= static_cast<int>(src->getHeight())) {
		return 1; /* 失敗 */
	}

	for(int y = 0; y < (int)this->height; ++y){
		for(int x = 0; x < (int)this->width; ++x){
			sum[0] = sum[1] = sum[2] = 0;
			for(int i = 0; i < 5; ++i){
				int mx = x + dx[i];
				int my = y + dy[i];
				if(x1 + mx < 0 || x1 + mx >= static_cast<int>(src->getWidth())){	/* 画像外のとき */
					mx = x;
				}
				if(y1 + my < 0 || y1 + my >= static_cast<int>(src->getHeight())){	/* 画像外のとき */
					my = y;
				}
				for(int j = 0; j < 3; ++j){
					sum[j] += h[i] * static_cast<int>(src->getData_NoSecure(x1 + mx, y1 + my, j));
				}
			}
			for(int i = 0; i < 3; ++i){
				this->setData_NoSecure(x, y, i, sum[i]);
			}
		}
	}

	return 0;
}

int ImageSig::amplifyFilter(double n) {
	if(this->data == NULL){ return 1; }
	const int w = static_cast<int>(this->width), h = static_cast<int>(this->height);
	for(int y = 0; y < h; ++y) {
		for(int x = 0; x < w; ++x) {
			const int idx = y * w + x;
			this->data[idx].r = static_cast<int>(n * static_cast<double>(this->data[idx].r));
			this->data[idx].g = static_cast<int>(n * static_cast<double>(this->data[idx].g));
			this->data[idx].b = static_cast<int>(n * static_cast<double>(this->data[idx].b));
		}
	}

	return 0;
}

int ImageSig::selectStrongerGradientAndMix(class Image *srcImg1, int x11, int y11, class Image *srcImg2, int x21, int y21) {
	class ImageSig DiffX, DiffY;

	if(this->data == NULL){ return 1; }
	const int w = this->width, h = this->height;
	if(x11 < 0 || x11 + w - 1 >= static_cast<int>(srcImg1->getWidth()) || y11 < 0 || y11 + h - 1 >= static_cast<int>(srcImg1->getHeight())){
		return 1;
	}
	if(x21 < 0 || x21 + w - 1 >= static_cast<int>(srcImg2->getWidth()) || y21 < 0 || y21 + h - 1 >= static_cast<int>(srcImg2->getHeight())){
		return 1;
	}
	/* grad の差分は +1 から、 div の差分は -1 からやるため、 1 pixel 余分に取る。 */
	if(DiffX.create_Image(1 + w, 1 + h)){
		return 1;
	}
	if(DiffY.create_Image(1 + w, 1 + h)){
		return 1;
	}

	/* grad の計算・選択 */
	for(int y = -1; y < h; ++y){
		for(int x = -1; x < w; ++x){
			for(int i = 0; i < 3; ++i){
				int sum1 = 0, sum2 = 0;
				const int diffX1 = srcImg1->getData(x11 + x + 1, y11 + y, i) - srcImg1->getData(x11 + x, y11 + y, i),
					diffY1 = srcImg1->getData(x11 + x, y11 + y + 1, i) - srcImg1->getData(x11 + x, y11 + y, i);
				sum1 += diffX1 * diffX1 + diffY1 * diffY1;
				const int diffX2 = srcImg2->getData(x21 + x + 1, y21 + y, i) - srcImg2->getData(x21 + x, y21 + y, i),
					diffY2 = srcImg2->getData(x21 + x, y21 + y + 1, i) - srcImg2->getData(x21 + x, y21 + y, i);
				sum2 += diffX2 * diffX2 + diffY2 * diffY2;
				if(sum1 > sum2){	/* |∇f1| > |∇f2| */
					DiffX.setData(x + 1, y + 1, i, diffX1);
					DiffY.setData(x + 1, y + 1, i, diffY1);
				}else{	/* |∇f1| ≦ |∇f2| */
					DiffX.setData(x + 1, y + 1, i, diffX2);
					DiffY.setData(x + 1, y + 1, i, diffY2);
				}
			}
		}
	}

	for(int y = 0; y < h; ++y){
		for(int x = 0; x < w; ++x){
			/* div の計算 */
			for(int i = 0; i < 3; ++i){
				this->setData(x, y, i, DiffX.getData(1 + x, 1 + y, i) - DiffX.getData(x, 1 + y, i)
					+ DiffY.getData(1 + x, 1 + y, i) - DiffY.getData(1 + x, y, i));
			}
		}
	}

	return 0;
}

bool ImageSig::checkReserve(void){
	return this->data != NULL;
}

ImageSig &ImageSig::operator=(ImageSig &src){
	bool flagTemp = true;
	if(!src.checkReserve()){
		throw "Cannot copy.";
	}
	if(this->data != NULL){
		if(this->width == src.getWidth() && this->height == src.getHeight()){
			flagTemp = false;
		}
	}
	if(flagTemp){
		if(this->create_Image(static_cast<int>(src.getWidth()), static_cast<int>(src.getHeight()))){
			throw "Cannot copy.";
		}
	}
	const int w = static_cast<int>(this->width), h = static_cast<int>(this->height);
	for(int y = 0; y < h; ++y){
		for(int x = 0; x < w; ++x){
			for(int i = 0; i < 3; ++i){
				this->setData_NoSecure(x, y, i, src.getData_NoSecure(x, y, i));
			}
		}
	}
	return *this;
}

ImageSig::ImageSig(void){
	this->data = NULL;
}

ImageSig::~ImageSig(void){
	this->free_Image();
}

/* ↑ここまで ImageSig_t */

int solvePoisson(Image *colorImg, ImageSig *lapImg, int x1, int y1, BitMask *regionMask){
	const char dx[] = {0,-1,1,0}, dy[] = {-1,0,0,1};
	int count, periodNum, permissibleError;
	int sum[3];
	class ImageSig tempImg, tempImgBefore;	/* 計算用 */
	bool flagEnd, flagRegion;

	/* 実験用 */
	int ComparisonCount = 0;	/* Laplacian の比較回数 */

	if ((x1 < 0) || ((x1 + static_cast<int>(lapImg->getWidth()) - 1) >= static_cast<int>(colorImg->getWidth())) || (y1 < 0) || ((y1 + static_cast<int>(lapImg->getHeight()) - 1) >= static_cast<int>(colorImg->getHeight()))) {
		return 1;
	}

	if(lapImg->getWidth() != regionMask->getWidth() || lapImg->getHeight() != regionMask->getHeight()){
		return 1;
	}

	/* 計算用領域確保 */
	if(tempImg.create_Image(static_cast<int>(lapImg->getWidth()) + 2, static_cast<int>(lapImg->getHeight()) + 2)){ return 1; }
	if (tempImgBefore.create_Image(static_cast<int>(lapImg->getWidth()) + 2, static_cast<int>(lapImg->getHeight()) + 2)) { return 1; }

	/* 領域に初期条件を設定 */
	const int lapW = static_cast<int>(lapImg->getWidth()),
		lapH = static_cast<int>(lapImg->getHeight()),
		colW = static_cast<int>(colorImg->getWidth()),
		colH = static_cast<int>(colorImg->getHeight());
	for(int y = -1; y < lapH + 1; ++y){
		for(int x = -1; x < lapW + 1; ++x){
			int mx = x1 + x, my = y1 + y;
			/* (x,y) が対象領域であるか判定 */
			flagRegion = false;
			if(x >= 0 && x < lapW && y >= 0 && y < lapH){
				if(regionMask->getData_NoSecure(x, y) != 0){
					flagRegion = true;
				}
			}
			if(flagRegion){	/* (x,y)が対象領域のとき、初期条件0とする。 */
				/* 計算領域に設定 */
				for(int i = 0; i < 3; ++i){
					tempImg.setData_NoSecure(1 + x, 1 + y, i, 0);
				}
			}else{	/* (x,y)が対象領域でないとき、対象画像 (境界条件) とする。 */
				if(mx < 0){	/* 画像の外を画像の端と同じとする。 */
					mx = x1;
				}else if(mx >= colW){
					mx = x1 + colW - 1;
				}
				if(my < 0){
					my = y1;
				}else if(my >= colH){
					my = y1 + colH - 1;
				}
				for(int i = 0; i < 3; ++i){
					tempImg.setData_NoSecure(1 + x, 1 + y, i, colorImg->getData(mx, my, i));
				}
			}
		}
	}

	/* 漸化式を解く */
	periodNum = std::max(static_cast<int>(std::sqrt(static_cast<double>(regionMask->getTruePixelNum()))) / 2, 1);	/* 比較する周期 */
	permissibleError = 0;	/* 前の画像との許容誤差 */
	for(count = 0, flagEnd = false; !flagEnd;){
		for(int y = 0; y < lapH; ++y){
			for(int x = 0; x < lapW; ++x){
				if(regionMask->getData_NoSecure(x, y) != 0){	/* 対象領域である場合のみ漸化式を解く */
					sum[0] = sum[1] = sum[2] = 0;
					for(int i = 0; i < 4; ++i){
						int mx = 1 + x + dx[i], my = 1 + y + dy[i];
						for(int j = 0; j < 3; ++j){
							sum[j] += tempImg.getData_NoSecure(mx, my, j);
						}
					}
					for(int i = 0; i < 3; ++i){
						sum[i] = (sum[i] - lapImg->getData_NoSecure(x, y, i) + 2) / 4; /* +2は四捨五入のため */
						tempImg.setData_NoSecure(1 + x, 1 + y, i, sum[i]);
					}
				}
			}
		}

		/* 周期が来たら、 Laplacianを調べる */
		count = (count + 1) % periodNum;
		if(count == 0) {
			++ComparisonCount;	/* 実験用 */
			/* 前回と比較 */
			sum[0] = sum[1] = sum[2] = 0;
			for(int y = 0; y < lapH; ++y){
				for(int x = 0; x < lapW; ++x){
					if(regionMask->getData_NoSecure(x, y) != 0){
						for(int i = 0; i < 3; ++i){
							sum[i] += std::abs(tempImg.getData_NoSecure(1 + x, 1 + y, i) - tempImgBefore.getData_NoSecure(1 + x, 1 + y, i));
						}
					}
				}
			}
			try{
				tempImgBefore = tempImg;
			} catch(...) {
				return 1;
			}
			if(sum[0] <= permissibleError && sum[1] <= permissibleError && sum[2] <= permissibleError){
				flagEnd = true;
			}else if(ComparisonCount >= 100){	/* 比較が 100 回に達したら強制終了 */
					flagEnd = true;
			}
		}
	}

	/* 複写 */
	for(int y = 0; y < lapH; ++y){
		for(int x = 0; x < lapW; ++x){
			if(regionMask->getData(x, y) != 0){
				for(int i = 0; i < 3; ++i){
					colorImg->setData_NoSecure(x1 + x, y1 + y, i, static_cast<byte>(std::max(std::min(tempImg.getData_NoSecure(1 + x, 1 + y, i), 255), 0)));
				}
			}
		}
	}

	/* 実験用 */
	printf("比較回数 (漸化式を %d 回解く毎に一度) : %d\n", periodNum, ComparisonCount);

	return 0;
}

int straightCompose(Image *destImg, int destX1, int destY1, Image *srcImg, int srcX1, int srcY1, BitMask *regionMask){
	const int maskW = static_cast<int>(regionMask->getWidth()), maskH = static_cast<int>(regionMask->getHeight()),
		destW = static_cast<int>(destImg->getWidth()), destH = static_cast<int>(destImg->getHeight()),
		srcW = static_cast<int>(srcImg->getWidth()), srcH = static_cast<int>(srcImg->getHeight());

	if( (destX1 < 0) || ( (destX1 + static_cast<int>(regionMask->getWidth()) - 1) >= static_cast<int>(destImg->getWidth())) || (destY1 < 0) || ( (destY1 + static_cast<int>(regionMask->getHeight()) - 1) >= static_cast<int>(destImg->getHeight())) ) {
		return 1;
	}

	/* 複写 */
	for(int y = 0; y < maskH; ++y) {
		for(int x = 0; x < maskW; ++x) {
			if(regionMask->getData_NoSecure(x, y) != 0) {
				for(int i = 0; i < 3; ++i){
					destImg->setData_NoSecure(destX1 + x, destY1 + y, i, srcImg->getData_NoSecure(srcX1 + x, srcY1 + y, i));
				}
			}
		}
	}

	return 0;
}

int ImageSig::write_ImgSig(char *filename)
{
	FILE *fp;
	byte *headerBuf;
	i16 pixelData[3];

	if((fp = fopen(filename, "wb")) == NULL) {
		return 1;
	}

	/* header */
	try{
		headerBuf = new byte[2 + (sizeof(this->width) + sizeof(this->height)) / sizeof(byte)];
	} catch (...) {
		fclose(fp);
		return 1;
	}
	headerBuf[0] = (byte)'L';
	headerBuf[1] = (byte)'F';
	memcpy(headerBuf + 2, &this->width, sizeof(this->width));
	memcpy(headerBuf + 2 + sizeof(this->width) / sizeof(headerBuf[0]), &this->height, sizeof(this->height));

	fwrite(headerBuf, sizeof(headerBuf[0]), sizeof(headerBuf) / sizeof(headerBuf[0]), fp);
	delete [] headerBuf;

	/* data */
	for(int y = 0; y < static_cast<int>(this->height); ++y){
		for(int x = 0; x < static_cast<int>(this->height); ++x){
			for(int i = 0; i < 3; ++i){
				pixelData[i] = this->getData_NoSecure(x, y, i);
			}
			fwrite(pixelData, sizeof(i16), 3, fp);
		}
	}

	fclose(fp);

	return 0;
}

int ImageSig::read_ImgSig(char *filename){
	FILE *fp;
	uint width, height;
	byte HeaderBuf[2 + (sizeof(width) + sizeof(height)) / sizeof(byte)];
	i16 pixelData[3];

	if((fp = fopen(filename, "rb")) == NULL){
		return 1;
	}

	fread(HeaderBuf, sizeof(HeaderBuf[0]), sizeof(HeaderBuf) / sizeof(HeaderBuf[0]), fp);
	/* 最初の2byteが "LF" か調べる */
	if(strncmp((char *)HeaderBuf, "LF", 2)){
		return 1;
	}

	/* width, height 取得 */
	memcpy(&width, HeaderBuf + 2, sizeof(width));
	memcpy(&height, HeaderBuf + 2 + sizeof(width) / sizeof(HeaderBuf[0]), sizeof(height));

	if(this->create_Image(width, height)){
		fclose(fp);
		return 1;
	}

	/* data 取得 */
	for(int y = 0; y < (int)height; ++y){
		for(int x = 0; x < (int)width; ++x){
			fread(pixelData, sizeof(i16), 3, fp);
			for(int i = 0; i < 3; ++i){
				this->setData_NoSecure(x, y, i, pixelData[i]);
			}
		}
	}

	fclose(fp);
	return 0;
}
