
#include "bitmask.h"

typedef unsigned char byte;
typedef unsigned int uint;

/* 単一色画像データを取得 */
int BitMask_t::Create_Image(int width, int height){
	this->Free_Image();
	try{
		this->data = new byte[width * height];
	}catch(...){
		return 1;
	}

	this->width = (uint)width;
	this->height = (uint)height;
	this->TruePixelNum = 0;

	return 0;
}

/* 単一色画像データの解放 */
void BitMask_t::Free_Image(void){
	if(this->data != NULL){
		delete [] this->data;
		this->data = NULL;
	}
	return;
}

byte BitMask_t::GetData(int x, int y) {
	if(this->data == NULL) { return 0; }
	if(x < 0 || x >= (int)this->width || y < 0 || y >= (int)this->height) { return 0; }
	return GetData_NoSecure(x, y);
}

byte BitMask_t::GetData_NoSecure(int x, int y) {
	return this->data[y * (int)this->width + x];
}

void BitMask_t::SetData(int x, int y, byte Color){
	if(this->data == NULL) { return; }
	if(x < 0 || x >= (int)this->width || y < 0 || y >= (int)this->height) { return; }
	this->SetData_NoSecure(x, y, Color);
}

void BitMask_t::SetData_NoSecure(int x, int y, byte Color){
	this->data[y * (int)this->width + x] = Color;
}

/* カラー画像の明るい部分を偽(0)、暗い部分を真(1)とする。 */
int BitMask_t::Img2Mask(class Image_t &srcImg)
{
	int x, y, x1, y1, x2, y2, count;

	if(!srcImg.CheckReserve()){
		return 1;
	}
	x1 = (int)srcImg.GetWidth() - 1; y1 = (int)srcImg.GetHeight() - 1; x2 = 0; y2 = 0;

	for(y = 0; y < (int)srcImg.GetHeight(); ++y){
		for(x = 0; x < (int)srcImg.GetWidth(); ++x){
			if(RGB2Grayscale(srcImg.GetData(x, y, 0), srcImg.GetData(x, y, 1), srcImg.GetData(x, y, 2)) < 128){
				if(x < x1) { x1 = x; }
				if(y < y1) { y1 = y; }
				if(x > x2) { x2 = x; }
				if(y > y2) { y2 = y; }
			}
		}
	}

	if(this->Create_Image(x2 - x1 + 1, y2 - y1 + 1)){
		return 1;
	}
	this->x1 = x1; this->y1 = y1;

	count = 0;
	for(y = 0; y < (int)this->height; ++y){
		for(x = 0; x < (int)this->width; ++x){
			if(RGB2Grayscale(srcImg.GetData(x1 + x, y1 + y, 0), srcImg.GetData(x1 + x, y1 + y, 1), srcImg.GetData(x1 + x, y1 + y, 2)) < 128){
				this->SetData_NoSecure(x, y, 1);
				++count;
			}else{
				this->SetData_NoSecure(x, y, 0);
			}
		}
	}

	this->TruePixelNum = count;

	return 0;
}

void BitMask_t::Fill_BitMask(void){
	int i, end;
	if(this->data == NULL){ return; }
	end = (int)this->width * (int)this->height;
	for(i = 0; i < end; ++i){
		this->data[i] = 1;
	}
	this->TruePixelNum = this->width * this->height;
}

bool BitMask_t::CheckReserve(void){
	return this->data != NULL;
}

unsigned int BitMask_t::GetTruePixelNum(void){
	return this->TruePixelNum;
}

BitMask_t &BitMask_t::operator=(BitMask_t &Src){
	int x, y, w, h; bool flagTemp = true;
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
			this->SetData_NoSecure(x, y, Src.GetData_NoSecure(x, y));
		}
	}
	return *this;
}

BitMask_t::BitMask_t(void){
	this->data = NULL;
}

BitMask_t::~BitMask_t(void){
	if(this->data != NULL){
		delete [] this->data;
		this->data = NULL;
	}
}
