/* ビットマップファイル入出力
物理のかぎしっぽより */

#include "bitmap.h"
#include <cstdio>
#include <cstring>
#pragma warning( disable : 4996 )	/* fopen の警告抑制 */

#define FILEHEADERSIZE 14	/* ファイルヘッダのサイズ */
#define INFOHEADERSIZE 40	/* 情報ヘッダのサイズ */
#define HEADERSIZE (FILEHEADERSIZE+INFOHEADERSIZE)

typedef unsigned char byte;
typedef unsigned int uint;


/* ↓ここから ImageBase_t */

unsigned int ImageBase::getWidth(void){
	return this->width;
}

unsigned int ImageBase::getHeight(void){
	return this->height;
}

/* ↑ここまで ImageBase_t */


/* ↓ここから Image_t */

byte Image::getData(int x, int y, int colorId){
	if(x < 0) { x = 0; }
	if(x >= static_cast<int>(this->width)) { x = static_cast<int>(this->width) - 1; }
	if(y < 0) { y = 0; }
	if(y >= static_cast<int>(this->height)) { y = static_cast<int>(this->height) - 1; }
	return getData_NoSecure(x, y, colorId);
}

byte Image::getData_NoSecure(int x, int y, int colorId){
	const int idx = y * static_cast<int>(this->width) + x;
	switch(colorId){
		case 0:
			return this->data[idx].r;
		case 1:
			return this->data[idx].g;
		case 2:
			return this->data[idx].b;
		default:
			break;
	}
	return 0;
}

void Image::setData(int x, int y, int colorId, byte color){
	if(x < 0 || x >= static_cast<int>(this->width) || y < 0 || y >= static_cast<int>(this->height)) { return; }
	setData_NoSecure(x, y, colorId, color);
	return;
}

void Image::setData_NoSecure(int x, int y, int colorId, byte color){
	const int idx = y * static_cast<int>(this->width) + x;
	switch(colorId){
		case 0:
			this->data[idx].r = static_cast<byte>(color);
			break;
		case 1:
			this->data[idx].g = static_cast<byte>(color);
			break;
		case 2:
			this->data[idx].b = static_cast<byte>(color);
			break;
		default:
			break;
	}
	return;
}

int Image::create_Image(int width, int height){
	this->free_Image();
	try{
		this->data = new RGB[width * height];
	}catch(...){
		this->data = NULL;
		return 1;
	}
	this->width = width;
	this->height = height;
	return 0;
}

void Image::free_Image(void){
	if(this->data != NULL){
		delete [] this->data;
		this->data = NULL;
	}
	return;
}

bool Image::checkReserve(void){
	return this->data != NULL;
}

Image &Image::operator=(Image &src){
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
				this->setData(x, y, i, src.getData(x, y, i));
			}
		}
	}
	return *this;
}

Image::Image(void){
	this->data = NULL;
}

Image::~Image(void){
	this->free_Image();
}

int Image::read_Bmp(const char *filename)
{
	int real_width;	/* Data 上の1行分の Byte 数 */
	uint width, height;	/* 画像の横と縦の Pixel 数 */
	uint color;	/* 何 Bit の Bitmap file であるか */
	FILE *fp = NULL;
	byte *headerBuf;	/* Header 情報を取り込む */
	byte *bmp_line_data = NULL;	/* 画像 Data 1行分 */

	if((fp = fopen(filename, "rb")) == NULL){
		return 1;
	}

	try{
		headerBuf = new byte[HEADERSIZE];
	}catch(...){
		fclose(fp);
		return 1;
	}
	fread(headerBuf, sizeof(byte), HEADERSIZE / sizeof(byte), fp); /* ヘッダ部分全てを取り込む */

	/* 最初の2バイトがBM(Bitmapファイルの印)であるか */
	if(strncmp((char *)headerBuf, "BM", 2)){
		delete [] headerBuf;
		fclose(fp);
		return 1;
	}

	memcpy(&width, headerBuf + 18, sizeof(width));	/* 画像の見た目上の幅を取得 */
	memcpy(&height, headerBuf + 22, sizeof(height));	/* 画像の高さを取得 */
	memcpy(&color, headerBuf + 28, sizeof(uint));	/* 何bitのBitmapであるかを取得 */

	delete [] headerBuf;

	/* 24bitで無ければ終了 */
	if(color != 24){
		return 1;
	}

	/* RGB情報は画像の1行分が4byteの倍数で無ければならないためそれに合わせている */
	real_width = width * 3 + width % 4;

	/* 画像の1行分のRGB情報を取ってくるためのバッファを動的に取得 */
	try {
		bmp_line_data = new byte[real_width];
	}catch(...){
		return 1;
	}

	/* RGB情報を取り込むためのバッファを動的に取得 */
	if(this->create_Image(width, height)){
		delete [] bmp_line_data;
		fclose(fp);
		return 1;
	}

	/* BitmapファイルのRGB情報は左下から右へ、下から上に並んでいる */
	for(int y = 0; y < static_cast<int>(height); ++y){
		fread(bmp_line_data, 1, real_width, fp);
		for(int x = 0; x < (int)width; ++x){
			for(int i = 0; i < 3; ++i){
				this->setData(x, height - y - 1, i, bmp_line_data[x * 3 + (2 - i)]);
			}
		}
	}

	delete [] bmp_line_data;
	fclose(fp);

	return 0;
}

int Image::write_Bmp(const char *filename)
{
	FILE *fp;
	int real_width;
	byte *bmp_line_data;	/* 画像1行分のRGB情報を格納する */
	byte header_buf[HEADERSIZE];	/* ヘッダを格納する */
	uint file_size;
	uint offset_to_data;
	unsigned long info_header_size;
	uint planes;
	uint color;
	unsigned long compress;
	unsigned long data_size;
	long xppm;
	long yppm;
	uint temp;

	if((fp = fopen(filename, "wb")) == NULL){
		return 1;
	}

	real_width = static_cast<int>(this->getWidth()) * 3 + static_cast<int>(this->getWidth()) % 4;

	/* ここからヘッダ作成 */
	file_size = static_cast<int>(this->getHeight()) * real_width + HEADERSIZE;
	offset_to_data = HEADERSIZE;
	info_header_size = INFOHEADERSIZE;
	planes = 1;
	color = 24;
	compress = 0;
	data_size = static_cast<int>(this->getHeight()) * real_width;
	xppm = 1;
	yppm = 1;
	
	header_buf[0] = 'B';
	header_buf[1] = 'M';
	memcpy(header_buf + 2, &file_size, sizeof(file_size));
	header_buf[6] = 0;
	header_buf[7] = 0;
	header_buf[8] = 0;
	header_buf[9] = 0;
	memcpy(header_buf + 10, &offset_to_data, sizeof(file_size));
	header_buf[11] = 0;
	header_buf[12] = 0;
	header_buf[13] = 0;

	memcpy(header_buf + 14, &info_header_size, sizeof(info_header_size));
	header_buf[15] = 0;
	header_buf[16] = 0;
	header_buf[17] = 0;
	temp = this->getWidth();
	memcpy(header_buf + 18, &temp, sizeof(temp));	/* width */
	temp = this->getHeight();
	memcpy(header_buf + 22, &temp, sizeof(temp));	/* height */
	memcpy(header_buf + 26, &planes, sizeof(planes));
	memcpy(header_buf + 28, &color, sizeof(color));
	memcpy(header_buf + 30, &compress, sizeof(compress));
	memcpy(header_buf + 34, &data_size, sizeof(data_size));
	memcpy(header_buf + 38, &xppm, sizeof(xppm));
	memcpy(header_buf + 42, &yppm, sizeof(yppm));
	header_buf[46] = 0;
	header_buf[47] = 0;
	header_buf[48] = 0;
	header_buf[49] = 0;
	header_buf[50] = 0;
	header_buf[51] = 0;
	header_buf[52] = 0;
	header_buf[53] = 0;

	/* ヘッダの書き込み */
	fwrite(header_buf, sizeof(byte), HEADERSIZE, fp);

	try{
		bmp_line_data = new byte[real_width];
	}catch(...){
		fclose(fp);
		return 1;
	}

	/* RGB情報の書き込み */
	for(int y = 0; y < static_cast<int>(this->getHeight()); y++){
		for(int x = 0; x < static_cast<int>(this->getWidth()); x++){
			for(int i = 0; i < 3; ++i){
				bmp_line_data[x * 3 + (2 - i)] = this->getData(x, this->getHeight() - y - 1, i);
			}
		}
		/* RGB情報を4バイトの倍数に合わせている */
		for(int i = this->getWidth() * 3; i < real_width; i++){
			bmp_line_data[i] = 0;
		}
		fwrite(bmp_line_data, sizeof(byte), real_width, fp);
	}

	delete [] bmp_line_data;
	fclose(fp);

	return 0;
}

/* ↑ここまで Image_t */


int rGB2Grayscale(int r, int g, int b){
	return ( r * 2 + g * 4 + b ) / 7;
	/* 正しくは Y = ( 0.298912 * R + 0.586611 * G + 0.114478 * B ) だが整数でやりたくてこう近似した。 */
}

int Image::makeIntoGrayscale(){
	if(!this->checkReserve()){
		return 1;
	}
	const int w = static_cast<int>(this->getWidth()), h = static_cast<int>(this->getHeight());
	for(int y = 0; y < h; ++y){
		for(int x = 0; x < w; ++x){
			const int color = rGB2Grayscale(this->getData(x, y, 0), this->getData(x, y, 1), this->getData(x, y, 2));
			for(int i = 0; i < 3; ++i){
				this->setData(x, y, i, color);
			}
		}
	}
	return 0;
}
