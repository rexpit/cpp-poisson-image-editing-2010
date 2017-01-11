/* ビットマップファイル入出力
物理のかぎしっぽより
後半で独自関数追加 */

#ifndef BITMAP_H_INCLUDED
#define BITMAP_H_INCLUDED

struct RGB {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

class ImageBase{
protected:
	unsigned int width;
	unsigned int height;

public:
	unsigned int getWidth(void);	/* width を取り出す */
	unsigned int getHeight(void);	/* height を取り出す */
};

class Image : public ImageBase{
private:
	struct RGB *data;

public:
	/* 点 (x, y) の colorId (0:赤, 1:緑, 2:青) の色を取り出す */
	unsigned char getData(int x, int y, int colorId);
	unsigned char getData_NoSecure(int x, int y, int colorId);

	/* 点 (x, y) の colorId (0:赤, 1:緑, 2:青) に色 color を代入 */
	void setData(int x, int y, int colorId, unsigned char color);
	void setData_NoSecure(int x, int y, int colorId, unsigned char color);

	/* Image を作成し、 RGB 情報も width * height 分だけ動的に取得する。
	成功すれば 0 を、失敗すれば 0 以外を返す */
	int create_Image(int width, int height);

	/* dataを解放する */
	void free_Image(void);

	/* filename の Bitmap file を読み込み、高さと幅、 RGB 情報をこの class に入れる。
	成功すれば 0 を、失敗すれば 0 以外を返す */
	int read_Bmp(const char *filename);

	/* 書き込みに成功すれば0を失敗すれば0以外を返す。
	成功すれば 0 を、失敗すれば 0 以外を返す */
	int write_Bmp(const char *filename);

	/* 画像領域を確保しているか確認 (true:確保済み, false:確保していない) */
	bool checkReserve(void);

	/* Grayscale 化 */
	int makeIntoGrayscale(void);

	Image &operator=(Image &src);

	/* constractor */
	Image(void);
	/* destractor */
	~Image(void);
};

/* NTCS によるグレースケール */
int rGB2Grayscale(int r, int g, int b);

#endif /* BITMAP_H_INCLUDED */
