/* ビットマップファイル入出力
物理のかぎしっぽより
後半で独自関数追加 */

#ifndef BITMAP_H_INCLUDED
#define BITMAP_H_INCLUDED
#include <cstdio>
#include <cstring>

struct Rgb_t{
	public: unsigned char r;
	public: unsigned char g;
	public: unsigned char b;
};

class ImageBase_t{
	protected: unsigned int width;
	protected: unsigned int height;

	public: unsigned int GetWidth(void);	/* width を取り出す */
	public: unsigned int GetHeight(void);	/* height を取り出す */
};

class Image_t : public ImageBase_t{
	private: struct Rgb_t *data;

	/* 点 (x, y) の ColorId (0:赤, 1:緑, 2:青) の色を取り出す */
	public: unsigned char GetData(int x, int y, int ColorId);
	public: unsigned char GetData_NoSecure(int x, int y, int ColorId);

	/* 点 (x, y) の ColorId (0:赤, 1:緑, 2:青) に色 Color を代入 */
	public: void SetData(int x, int y, int ColorId, unsigned char  Color);
	public: void SetData_NoSecure(int x, int y, int ColorId, unsigned char Color);

	/* Image を作成し、 RGB 情報も width * height 分だけ動的に取得する。
	成功すれば 0 を、失敗すれば 0 以外を返す */
	public: int Create_Image(int width, int height);

	/* dataを解放する */
	public: void Free_Image(void);

	/* filename の Bitmap file を読み込み、高さと幅、 RGB 情報をこの class に入れる。
	成功すれば 0 を、失敗すれば 0 以外を返す */
	public: int Read_Bmp(char *filename);

	/* 書き込みに成功すれば0を失敗すれば0以外を返す。
	成功すれば 0 を、失敗すれば 0 以外を返す */
	public: int Write_Bmp(char *filename);

	/* 画像領域を確保しているか確認 (true:確保済み, false:確保していない) */
	public: bool CheckReserve(void);

	/* Grayscale 化 */
	int MakeIntoGrayscale(void);

	public: Image_t &operator=(Image_t &Src);

	/* constractor */
	public: Image_t(void);
	/* destractor */
	public: ~Image_t(void);
};

/* NTCS によるグレースケール */
int RGB2Grayscale(int r, int g, int b);

#endif /* BITMAP_H_INCLUDED */
