/* ビットマップファイル入出力
物理のかぎしっぽより */

#include "bitmap.h"
#pragma warning( disable : 4996 )	/* fopen の警告抑制 */

#define FILEHEADERSIZE 14	/* ファイルヘッダのサイズ */
#define INFOHEADERSIZE 40	/* 情報ヘッダのサイズ */
#define HEADERSIZE (FILEHEADERSIZE+INFOHEADERSIZE)

typedef unsigned char byte;
typedef unsigned int uint;


/* ↓ここから ImageBase_t */

unsigned int ImageBase_t::GetWidth(void){
	return this->width;
}

unsigned int ImageBase_t::GetHeight(void){
	return this->height;
}

/* ↑ここまで ImageBase_t */


/* ↓ここから Image_t */

byte Image_t::GetData(int x, int y, int ColorId){
	if(x < 0) { x = 0; }
	if(x >= (int)this->width) { x = (int)this->width - 1; }
	if(y < 0) { y = 0; }
	if(y >= (int)this->height) { y = (int)this->height - 1; }
	return GetData_NoSecure(x, y, ColorId);
}

byte Image_t::GetData_NoSecure(int x, int y, int ColorId){
	int Idx = y * (int)this->width + x, Color;
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

void Image_t::SetData(int x, int y, int ColorId, byte Color){
	if(x < 0 || x >= (int)this->width || y < 0 || y >= (int)this->height) { return; }
	SetData_NoSecure(x, y, ColorId, Color);
	return;
}

void Image_t::SetData_NoSecure(int x, int y, int ColorId, byte Color){
	int Idx = y * (int)this->width + x;
	switch(ColorId){
		case 0:
			this->data[Idx].r = (byte)Color;
			break;
		case 1:
			this->data[Idx].g = (byte)Color;
			break;
		case 2:
			this->data[Idx].b = (byte)Color;
			break;
		default:
			break;
	}
	return;
}

int Image_t::Create_Image(int width, int height){
	this->Free_Image();
	try{
		this->data = new Rgb_t[width * height];
	}catch(...){
		this->data = NULL;
		return 1;
	}
	this->width = width;
	this->height = height;
	return 0;
}

void Image_t::Free_Image(void){
	if(this->data != NULL){
		delete [] this->data;
		this->data = NULL;
	}
	return;
}

bool Image_t::CheckReserve(void){
	return this->data != NULL;
}

Image_t &Image_t::operator=(Image_t &Src){
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
				this->SetData(x, y, i, Src.GetData(x, y, i));
			}
		}
	}
	return *this;
}

Image_t::Image_t(void){
	this->data = NULL;
}

Image_t::~Image_t(void){
	this->Free_Image();
}

int Image_t::Read_Bmp(char *filename)
{
	int x, y, i;
	int real_width;	/* Data 上の1行分の Byte 数 */
	uint width, height;	/* 画像の横と縦の Pixel 数 */
	uint color;	/* 何 Bit の Bitmap file であるか */
	FILE *fp = NULL;
	byte *HeaderBuf;	/* Header 情報を取り込む */
	byte *bmp_line_data = NULL;	/* 画像 Data 1行分 */

	if((fp = fopen(filename, "rb")) == NULL){
		return 1;
	}

	try{
		HeaderBuf = new byte[HEADERSIZE];
	}catch(...){
		fclose(fp);
		return 1;
	}
	fread(HeaderBuf, sizeof(byte), HEADERSIZE / sizeof(byte), fp); /* ヘッダ部分全てを取り込む */

	/* 最初の2バイトがBM(Bitmapファイルの印)であるか */
	if(strncmp((char *)HeaderBuf, "BM", 2)){
		delete [] HeaderBuf;
		fclose(fp);
		return 1;
	}

	memcpy(&width, HeaderBuf + 18, sizeof(width));	/* 画像の見た目上の幅を取得 */
	memcpy(&height, HeaderBuf + 22, sizeof(height));	/* 画像の高さを取得 */
	memcpy(&color, HeaderBuf + 28, sizeof(uint));	/* 何bitのBitmapであるかを取得 */

	delete [] HeaderBuf;

	/* 24bitで無ければ終了 */
	if(color != 24){
		return 1;
	}

	/* RGB情報は画像の1行分が4byteの倍数で無ければならないためそれに合わせている */
	real_width = width*3 + width%4;

	/* 画像の1行分のRGB情報を取ってくるためのバッファを動的に取得 */
	try{
		bmp_line_data = new byte[real_width];
	}catch(...){
		return 1;
	}

	/* RGB情報を取り込むためのバッファを動的に取得 */
	if(this->Create_Image(width, height)){
		delete [] bmp_line_data;
		fclose(fp);
		return 1;
	}

	/* BitmapファイルのRGB情報は左下から右へ、下から上に並んでいる */
	for(y = 0; y < (int)height; ++y){
		fread(bmp_line_data, 1, real_width, fp);
		for(x = 0; x < (int)width; ++x){
			for(i = 0; i < 3; ++i){
				this->SetData(x, height - y - 1, i, bmp_line_data[x * 3 + (2 - i)]);
			}
		}
	}

	delete [] bmp_line_data;
	fclose(fp);

	return 0;
}

int Image_t::Write_Bmp(char *filename)
{
	int x, y, i;
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

	real_width = (int)this->GetWidth() * 3 + (int)this->GetWidth() % 4;

	/* ここからヘッダ作成 */
	file_size = (int)this->GetHeight() * real_width + HEADERSIZE;
	offset_to_data = HEADERSIZE;
	info_header_size = INFOHEADERSIZE;
	planes = 1;
	color = 24;
	compress = 0;
	data_size = (int)this->GetHeight() * real_width;
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
	temp = this->GetWidth();
	memcpy(header_buf + 18, &temp, sizeof(temp));	/* width */
	temp = this->GetHeight();
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
	for(y = 0; y < (int)this->GetHeight(); y++){
		for(x = 0; x < (int)this->GetWidth(); x++){
			for(i = 0; i < 3; ++i){
				bmp_line_data[x * 3 + (2 - i)] = this->GetData(x, this->GetHeight() - y - 1, i);
			}
		}
		/* RGB情報を4バイトの倍数に合わせている */
		for(i = this->GetWidth() * 3; i < real_width; i++){
			bmp_line_data[i] = 0;
		}
		fwrite(bmp_line_data, sizeof(byte), real_width, fp);
	}

	delete [] bmp_line_data;
	fclose(fp);

	return 0;
}

/* ↑ここまで Image_t */


int RGB2Grayscale(int r, int g, int b){
	return ( ( (r + (g << 1) ) << 1) + b ) / 7;
	/* 正しくは Y = ( 0.298912 * R + 0.586611 * G + 0.114478 * B ) だが整数でやりたくてこう近似した。 */
}

int Image_t::MakeIntoGrayscale(){
	int x, y, w, h, i, color;
	if(!this->CheckReserve()){
		return 1;
	}
	w = (int)this->GetWidth(); h = (int)this->GetHeight();
	for(y = 0; y < h; ++y){
		for(x = 0; x < w; ++x){
			color = RGB2Grayscale(this->GetData(x, y, 0), this->GetData(x, y, 1), this->GetData(x, y, 2));
			for(i = 0; i < 3; ++i){
				this->SetData(x, y, i, color);
			}
		}
	}
	return 0;
}
