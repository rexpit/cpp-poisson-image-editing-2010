
#ifndef	BITMASK_H
#define	BITMASK_H

#include "bitmap.h"

class BitMask : public ImageBase{
private:
	unsigned int truePixelNum;	/* マスクのピクセル数 */
	unsigned char *data;

public:
	int x1, y1;	/* 左上端点座標 */

	/* 点 (x, y) の色を取り出す */
	unsigned char getData(int x, int y);
	unsigned char getData_NoSecure(int x, int y);

	/* 点 (x, y) に色 color を代入 */
	void setData(int x, int y, unsigned char color);
	void setData_NoSecure(int x, int y, unsigned char color);

	/* 単一色画像データを取得。
	成功すれば 0 を、失敗すれば 0 以外を返す */
	int create_Image(int width, int height);

	/* 単一色画像データの解放 */
	void free_Image(void);

	/* カラー画像の明るい部分を偽(0)、暗い部分を真(1)とする。 */
	int img2Mask(Image *srcImg);

	/* BitMask の data をすべて 1 にする。 */
	void fill_BitMask(void);

	unsigned int getTruePixelNum(void);

	/* 画像領域を確保しているか確認 (true:確保済み, false:確保していない) */
	bool checkReserve(void);

	BitMask &operator=(BitMask &src);

	BitMask(void);
	~BitMask(void);
};

#endif	/* BITMASK_H */
