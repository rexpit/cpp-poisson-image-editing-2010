
#include "bitmask.h"
#define	NULL	0

typedef unsigned char byte;
typedef unsigned int uint;

/* 単一色画像データを取得 */
int BitMask::create_Image(int width, int height){
	this->free_Image();
	try{
		this->data = new byte[width * height];
	}catch(...){
		return 1;
	}

	this->width = static_cast<uint>(width);
	this->height = static_cast<uint>(height);
	this->truePixelNum = 0;

	return 0;
}

/* 単一色画像データの解放 */
void BitMask::free_Image(void){
	if(this->data != NULL){
		delete [] this->data;
		this->data = NULL;
	}
	return;
}

byte BitMask::getData(int x, int y) {
	if(this->data == NULL) { return 0; }
	if(x < 0 || x >= static_cast<int>(this->width) || y < 0 || y >= static_cast<int>(this->height)) { return 0; }
	return getData_NoSecure(x, y);
}

byte BitMask::getData_NoSecure(int x, int y) {
	return this->data[y * static_cast<int>(this->width) + x];
}

void BitMask::setData(int x, int y, byte color){
	if(this->data == NULL) { return; }
	if(x < 0 || x >= static_cast<int>(this->width) || y < 0 || y >= static_cast<int>(this->height)) { return; }
	this->setData_NoSecure(x, y, color);
}

void BitMask::setData_NoSecure(int x, int y, byte color){
	this->data[y * static_cast<int>(this->width) + x] = color;
}

/* カラー画像の明るい部分を偽(0)、暗い部分を真(1)とする。 */
int BitMask::img2Mask(Image *srcImg)
{
	if(!srcImg->checkReserve()){
		return 1;
	}
	int x1 = static_cast<int>(srcImg->getWidth()) - 1, y1 = static_cast<int>(srcImg->getHeight()) - 1, x2 = 0, y2 = 0;

	for(int y = 0; y < static_cast<int>(srcImg->getHeight()); ++y){
		for(int x = 0; x < static_cast<int>(srcImg->getWidth()); ++x){
			if(rGB2Grayscale(srcImg->getData(x, y, 0), srcImg->getData(x, y, 1), srcImg->getData(x, y, 2)) < 128){
				if(x < x1) { x1 = x; }
				if(y < y1) { y1 = y; }
				if(x > x2) { x2 = x; }
				if(y > y2) { y2 = y; }
			}
		}
	}

	if(this->create_Image(x2 - x1 + 1, y2 - y1 + 1)){
		return 1;
	}
	this->x1 = x1; this->y1 = y1;

	int count = 0;
	for(int y = 0; y < static_cast<int>(this->height); ++y){
		for(int x = 0; x < static_cast<int>(this->width); ++x){
			if(rGB2Grayscale(srcImg->getData(x1 + x, y1 + y, 0), srcImg->getData(x1 + x, y1 + y, 1), srcImg->getData(x1 + x, y1 + y, 2)) < 128){
				this->setData_NoSecure(x, y, 1);
				++count;
			}else{
				this->setData_NoSecure(x, y, 0);
			}
		}
	}

	this->truePixelNum = count;

	return 0;
}

void BitMask::fill_BitMask(void){
	if(this->data == NULL){ return; }
	const int end = static_cast<int>(this->width) * static_cast<int>(this->height);
	for(int i = 0; i < end; ++i){
		this->data[i] = 1;
	}
	this->truePixelNum = this->width * this->height;
}

bool BitMask::checkReserve(void){
	return this->data != NULL;
}

unsigned int BitMask::getTruePixelNum(void){
	return this->truePixelNum;
}

BitMask &BitMask::operator=(BitMask &src){
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
			this->setData_NoSecure(x, y, src.getData_NoSecure(x, y));
		}
	}
	return *this;
}

BitMask::BitMask(void){
	this->data = NULL;
}

BitMask::~BitMask(void){
	if(this->data != NULL){
		delete [] this->data;
		this->data = NULL;
	}
}
