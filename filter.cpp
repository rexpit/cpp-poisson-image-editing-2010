
#pragma warning( disable : 4996 )	/* fopen の警告抑制 (VC++) */
#include "filter.h"
#include <cstdio>

typedef unsigned char byte;
typedef unsigned int uint;

/* ↓ここから ImageSig_t */

int ImageSig_t::GetData(int x, int y, int ColorId) {
	if(x < 0 || x >= (int)this->width || y < 0 || y >= (int)this->height) { return 0; }
	return GetData_NoSecure(x, y, ColorId);
}

int ImageSig_t::GetData_NoSecure(int x, int y, int ColorId) {
	int Idx, Color;
	Idx = y * (int)this->width + x;
	switch(ColorId){
		case 0:
			Color = this->data[Idx].r;
			break;
		case 1:
			Color = this->data[Idx].g;
			break;
		case 2:
			Color = this->data[Idx].b;
			break;
		default:
			Color = 0;
			break;
	}
	return Color;
}

void ImageSig_t::SetData(int x, int y, int ColorId, int Color) {
	if(x < 0 || x >= (int)this->width || y < 0 || y >= (int)this->height) { return; }
	SetData_NoSecure(x, y, ColorId, Color);
	return;
}

void ImageSig_t::SetData_NoSecure(int x, int y, int ColorId, int Color) {
	int Idx = y * (int)this->width + x;
	switch(ColorId){
		case 0:
			this->data[Idx].r = Color;
			break;
		case 1:
			this->data[Idx].g = Color;
			break;
		case 2:
			this->data[Idx].b = Color;
			break;
		default:
			break;
	}
	return;
}

int ImageSig_t::Create_Image(int width, int height) {
	this->Free_Image();
	try{
		this->data = new RgbSig_t[width * height];
	}catch(...){
		this->data = NULL;
		return 1;
	}

	this->width = width;
	this->height = height;

	return 0;
}

void ImageSig_t::Free_Image(void) {
	if(this->data != NULL){
		delete [] this->data;
		this->data = NULL;
	}
	return;
}

int ImageSig_t::LapFilter(Image_t &src, int x1, int y1) {
	const char dx[] = {0,-1,0,1,0}, dy[] = {-1,0,0,0,1}, h[] = {1,1,-4,1,1};
	int x, y, mx, my, i, j, x2, y2, sum[3];

	if(this->data == NULL){ return 1; }
	x2 = x1 + (int)this->width - 1; y2 = y1 + (int)this->height - 1;
	if(x1 < 0 || x2 >= (int)src.GetWidth() || y1 < 0 || y2 >= (int)src.GetHeight()){ return 1; /* 失敗 */ }

	for(y = 0; y < (int)this->height; ++y){
		for(x = 0; x < (int)this->width; ++x){
			sum[0] = sum[1] = sum[2] = 0;
			for(i = 0; i < 5; ++i){
				mx = x + dx[i];
				my = y + dy[i];
				if(x1 + mx < 0 || x1 + mx >= (int)src.GetWidth()){	/* 画像外のとき */
					mx = x;
				}
				if(y1 + my < 0 || y1 + my >= (int)src.GetHeight()){	/* 画像外のとき */
					my = y;
				}
				for(j = 0; j < 3; ++j){
					sum[j] += h[i] * (int)src.GetData_NoSecure(x1 + mx, y1 + my, j);
				}
			}
			for(i = 0; i < 3; ++i){
				this->SetData_NoSecure(x, y, i, sum[i]);
			}
		}
	}

	return 0;
}

int ImageSig_t::AmplifyFilter(double n) {
	int x, y, w, h, idx;

	if(this->data == NULL){ return 1; }
	w = (int)this->width; h = (int)this->height;
	for(y = 0; y < h; ++y){
		for(x = 0; x < w; ++x){
			idx = y * w + x;
			this->data[idx].r = (int)(n * (double)this->data[idx].r);
			this->data[idx].g = (int)(n * (double)this->data[idx].g);
			this->data[idx].b = (int)(n * (double)this->data[idx].b);
		}
	}

	return 0;
}

int ImageSig_t::SelectStrongerGradientAndMix(class Image_t &SrcImg1, int X11, int Y11, class Image_t &SrcImg2, int X21, int Y21) {
	class ImageSig_t DiffX, DiffY;
	int x, y, i, diffX1, diffY1, diffX2, diffY2;

	if(this->data == NULL){ return 1; }
	const int w = this->width, h = this->height;
	if(X11 < 0 || X11 + w - 1 >= (int)SrcImg1.GetWidth() || Y11 < 0 || Y11 + h - 1 >= (int)SrcImg1.GetHeight()){
		return 1;
	}
	if(X21 < 0 || X21 + w - 1 >= (int)SrcImg2.GetWidth() || Y21 < 0 || Y21 + h - 1 >= (int)SrcImg2.GetHeight()){
		return 1;
	}
	/* grad の差分は +1 から、 div の差分は -1 からやるため、 1 pixel 余分に取る。 */
	if(DiffX.Create_Image(1 + w, 1 + h)){
		return 1;
	}
	if(DiffY.Create_Image(1 + w, 1 + h)){
		return 1;
	}

	/* grad の計算・選択 */
	for(y = -1; y < h; ++y){
		for(x = -1; x < w; ++x){
			for(i = 0; i < 3; ++i){
				int sum1 = 0, sum2 = 0;
				diffX1 = SrcImg1.GetData(X11 + x + 1, Y11 + y, i) - SrcImg1.GetData(X11 + x, Y11 + y, i);
				diffY1 = SrcImg1.GetData(X11 + x, Y11 + y + 1, i) - SrcImg1.GetData(X11 + x, Y11 + y, i);
				sum1 += diffX1 * diffX1 + diffY1 * diffY1;
				diffX2 = SrcImg2.GetData(X21 + x + 1, Y21 + y, i) - SrcImg2.GetData(X21 + x, Y21 + y, i);
				diffY2 = SrcImg2.GetData(X21 + x, Y21 + y + 1, i) - SrcImg2.GetData(X21 + x, Y21 + y, i);
				sum2 += diffX2 * diffX2 + diffY2 * diffY2;
				if(sum1 > sum2){	/* |∇f1| > |∇f2| */
					DiffX.SetData(x + 1, y + 1, i, diffX1);
					DiffY.SetData(x + 1, y + 1, i, diffY1);
				}else{	/* |∇f1| ≦ |∇f2| */
					DiffX.SetData(x + 1, y + 1, i, diffX2);
					DiffY.SetData(x + 1, y + 1, i, diffY2);
				}
			}
		}
	}

	for(y = 0; y < h; ++y){
		for(x = 0; x < w; ++x){
			/* div の計算 */
			for(i = 0; i < 3; ++i){
				this->SetData(x, y, i, DiffX.GetData(1 + x, 1 + y, i) - DiffX.GetData(x, 1 + y, i)
					+ DiffY.GetData(1 + x, 1 + y, i) - DiffY.GetData(1 + x, y, i));
			}
		}
	}

	return 0;
}

bool ImageSig_t::CheckReserve(void){
	return this->data != NULL;
}

ImageSig_t &ImageSig_t::operator=(ImageSig_t &Src){
	int x, y, i, w, h; bool flagTemp = true;
	if(!Src.CheckReserve()){
		throw "Cannot copy.";
	}
	if(this->data != NULL){
		if(this->width == Src.GetWidth() && this->height == Src.GetHeight()){
			flagTemp = false;
		}
	}
	if(flagTemp){
		if(this->Create_Image((int)Src.GetWidth(), (int)Src.GetHeight())){
			throw "Cannot copy.";
		}
	}
	w = (int)this->width; h = (int)this->height;
	for(y = 0; y < h; ++y){
		for(x = 0; x < w; ++x){
			for(i = 0; i < 3; ++i){
				this->SetData_NoSecure(x, y, i, Src.GetData_NoSecure(x, y, i));
			}
		}
	}
	return *this;
}

ImageSig_t::ImageSig_t(void){
	this->data = NULL;
}

ImageSig_t::~ImageSig_t(void){
	this->Free_Image();
}

/* ↑ここまで ImageSig_t */

int SolvePoisson(class Image_t &ColorImg, class ImageSig_t &LapImg, int x1, int y1, class BitMask_t &RegionMask){
	const char dx[] = {0,-1,1,0}, dy[] = {-1,0,0,1};
	int x, y, count, i, j, mx, my, periodNum, permissibleError;
	int sum[3], LapW, LapH, ColW, ColH;
	class ImageSig_t TempImg, TempImgBefore;	/* 計算用 */
	bool flagEnd, flagRegion;

	/* 実験用 */
	int ComparisonCount = 0;	/* Laplacian の比較回数 */

	if( (x1 < 0) || ( (x1 + (int)LapImg.GetWidth() - 1) >= (int)ColorImg.GetWidth()) || (y1 < 0) || ( (y1 + (int)LapImg.GetHeight() - 1) >= (int)ColorImg.GetHeight()) ){
		return 1;
	}

	if(LapImg.GetWidth() != RegionMask.GetWidth() || LapImg.GetHeight() != RegionMask.GetHeight()){
		return 1;
	}

	/* 計算用領域確保 */
	if(TempImg.Create_Image((int)LapImg.GetWidth() + 2, (int)LapImg.GetHeight() + 2)){ return 1; }
	if(TempImgBefore.Create_Image((int)LapImg.GetWidth() + 2, (int)LapImg.GetHeight() + 2)){ return 1; }

	/* 領域に初期条件を設定 */
	LapW = (int)LapImg.GetWidth();
	LapH = (int)LapImg.GetHeight();
	ColW = (int)ColorImg.GetWidth();
	ColH = (int)ColorImg.GetHeight();
	for(y = -1; y < LapH + 1; ++y){
		for(x = -1; x < LapW + 1; ++x){
			mx = x1 + x; my = y1 + y;
			/* (x,y) が対象領域であるか判定 */
			flagRegion = false;
			if(x >= 0 && x < LapW && y >= 0 && y < LapH){
				if(RegionMask.GetData_NoSecure(x, y) != 0){
					flagRegion = true;
				}
			}
			if(flagRegion){	/* (x,y)が対象領域のとき、初期条件0とする。 */
				/* 計算領域に設定 */
				for(i = 0; i < 3; ++i){
					TempImg.SetData_NoSecure(1 + x, 1 + y, i, 0);
				}
			}else{	/* (x,y)が対象領域でないとき、対象画像 (境界条件) とする。 */
				if(mx < 0){	/* 画像の外を画像の端と同じとする。 */
					mx = x1;
				}else if(mx >= ColW){
					mx = x1 + ColW - 1;
				}
				if(my < 0){
					my = y1;
				}else if(my >= ColH){
					my = y1 + ColH - 1;
				}
				for(i = 0; i < 3; ++i){
					TempImg.SetData_NoSecure(1 + x, 1 + y, i, ColorImg.GetData(mx, my, i));
				}
			}
		}
	}

	/* 漸化式を解く */
	periodNum = maxNum((int)sqrt((double)RegionMask.GetTruePixelNum()) / 2 , 1);	/* 比較する周期 */
	permissibleError = 0;	/* 前の画像との許容誤差 */
	for(count = 0, flagEnd = false; !flagEnd;){
		for(y = 0; y < LapH; ++y){
			for(x = 0; x < LapW; ++x){
				if(RegionMask.GetData_NoSecure(x, y) != 0){	/* 対象領域である場合のみ漸化式を解く */
					sum[0] = sum[1] = sum[2] = 0;
					for(i = 0; i < 4; ++i){
						mx = 1 + x + dx[i]; my = 1 + y + dy[i];
						for(j = 0; j < 3; ++j){
							sum[j] += TempImg.GetData_NoSecure(mx, my, j);
						}
					}
					for(i = 0; i < 3; ++i){
						sum[i] = (sum[i] - LapImg.GetData_NoSecure(x, y, i) + 2) / 4; /* +2は四捨五入のため */
						TempImg.SetData_NoSecure(1 + x, 1 + y, i, sum[i]);
					}
				}
			}
		}

		/* 周期が来たら、 Laplacianを調べる */
		if((count = (count + 1) % periodNum) == 0){
			++ComparisonCount;	/* 実験用 */
			/* 前回と比較 */
			sum[0] = sum[1] = sum[2] = 0;
			for(y = 0; y < LapH; ++y){
				for(x = 0; x < LapW; ++x){
					if(RegionMask.GetData_NoSecure(x, y) != 0){
						for(i = 0; i < 3; ++i){
							sum[i] += abs(TempImg.GetData_NoSecure(1 + x, 1 + y, i) - TempImgBefore.GetData_NoSecure(1 + x, 1 + y, i));
						}
					}
				}
			}
			try{ TempImgBefore = TempImg; }catch(...){ return 1; }
			if(sum[0] <= permissibleError && sum[1] <= permissibleError && sum[2] <= permissibleError){
				flagEnd = true;
			}else if(ComparisonCount >= 100){	/* 比較が 100 回に達したら強制終了 */
					flagEnd = true;
			}
		}
	}

	/* 複写 */
	for(y = 0; y < LapH; ++y){
		for(x = 0; x < LapW; ++x){
			if(RegionMask.GetData(x, y) != 0){
				for(i = 0; i < 3; ++i){
					ColorImg.SetData_NoSecure(x1 + x, y1 + y, i, (byte)maxNum(minNum(TempImg.GetData_NoSecure(1 + x, 1 + y, i), 255), 0));
				}
			}
		}
	}

	/* 実験用 */
	printf("比較回数 (漸化式を %d 回解く毎に一度) : %d\n", periodNum, ComparisonCount);

	return 0;
}

int StraightCompose(class Image_t &DestImg, int destX1, int destY1, class Image_t &SrcImg, int srcX1, int srcY1, class BitMask_t &RegionMask){
	int x, y, i;
	int MaskW = (int)RegionMask.GetWidth(), MaskH = (int)RegionMask.GetHeight(), DestW = (int)DestImg.GetWidth(), DestH = (int)DestImg.GetHeight(), SrcW = (int)SrcImg.GetWidth(), SrcH = (int)SrcImg.GetHeight();

	if( (destX1 < 0) || ( (destX1 + (int)RegionMask.GetWidth() - 1) >= (int)DestImg.GetWidth()) || (destY1 < 0) || ( (destY1 + (int)RegionMask.GetHeight() - 1) >= (int)DestImg.GetHeight()) ){
		return 1;
	}

	/* 複写 */
	for(y = 0; y < MaskH; ++y){
		for(x = 0; x < MaskW; ++x){
			if(RegionMask.GetData_NoSecure(x, y) != 0){
				for(i = 0; i < 3; ++i){
					DestImg.SetData_NoSecure(destX1 + x, destY1 + y, i, SrcImg.GetData_NoSecure(srcX1 + x, srcY1 + y, i));
				}
			}
		}
	}

	return 0;
}

int ImageSig_t::Write_ImgSig(char *filename)
{
	FILE *fp;
	byte *HeaderBuf;
	i16 pixelData[3];
	int x, y, i;

	if((fp = fopen(filename, "wb")) == NULL){
		return 1;
	}

	/* header */
	try{
		HeaderBuf = new byte[2 + (sizeof(this->width) + sizeof(this->height)) / sizeof(byte)];
	}catch(...){
		fclose(fp);
		return 1;
	}
	HeaderBuf[0] = (byte)'L';
	HeaderBuf[1] = (byte)'F';
	memcpy(HeaderBuf + 2, &this->width, sizeof(this->width));
	memcpy(HeaderBuf + 2 + sizeof(this->width) / sizeof(HeaderBuf[0]), &this->height, sizeof(this->height));

	fwrite(HeaderBuf, sizeof(HeaderBuf[0]), sizeof(HeaderBuf) / sizeof(HeaderBuf[0]), fp);
	delete [] HeaderBuf;

	/* data */
	for(y = 0; y < (int)this->height; ++y){
		for(x = 0; x < (int)this->height; ++x){
			for(i = 0; i < 3; ++i){
				pixelData[i] = this->GetData_NoSecure(x, y, i);
			}
			fwrite(pixelData, sizeof(i16), 3, fp);
		}
	}

	fclose(fp);

	return 0;
}

int ImageSig_t::Read_ImgSig(char *filename){
	FILE *fp;
	uint width, height;
	byte HeaderBuf[2 + (sizeof(width) + sizeof(height)) / sizeof(byte)];
	int x, y, i;
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

	if(this->Create_Image(width, height)){
		fclose(fp);
		return 1;
	}

	/* data 取得 */
	for(y = 0; y < (int)height; ++y){
		for(x = 0; x < (int)width; ++x){
			fread(pixelData, sizeof(i16), 3, fp);
			for(i = 0; i < 3; ++i){
				this->SetData_NoSecure(x, y, i, pixelData[i]);
			}
		}
	}

	fclose(fp);
	return 0;
}
