
#ifndef	BITMASK_H
#define	BITMASK_H

#include "bitmap.h"

class BitMask_t : public ImageBase_t{
	public: int x1, y1;	/* 左上端点座標 */
	private: unsigned int TruePixelNum;	/* マスクのピクセル数 */
	private: unsigned char *data;

	/* 点 (x, y) の色を取り出す */
	public: unsigned char GetData(int x, int y);
	public: unsigned char GetData_NoSecure(int x, int y);

	/* 点 (x, y) に色 Color を代入 */
	public: void SetData(int x, int y, unsigned char Color);
	public: void SetData_NoSecure(int x, int y, unsigned char Color);

	/* 単一色画像データを取得。
	成功すれば 0 を、失敗すれば 0 以外を返す */
	public: int Create_Image(int width, int height);

	/* 単一色画像データの解放 */
	public: void Free_Image(void);

	/* カラー画像の明るい部分を偽(0)、暗い部分を真(1)とする。 */
	public: int Img2Mask(class Image_t &srcImg);

	/* BitMask の data をすべて 1 にする。 */
	public: void Fill_BitMask(void);

	public: unsigned int GetTruePixelNum(void);

	/* 画像領域を確保しているか確認 (true:確保済み, false:確保していない) */
	public: bool CheckReserve(void);

	public: BitMask_t &operator=(BitMask_t &Src);

	public: BitMask_t(void);
	public: ~BitMask_t(void);
};

#endif	/* BITMASK_H */
