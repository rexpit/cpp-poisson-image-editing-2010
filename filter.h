
#ifndef	FILTER_H
#define	FILTER_H

#include "bitmap.h"
#include "bitmask.h"

typedef short i16;	/* 符号付16bit整数 */

struct RGBSig{
	i16 r;
	i16 g;
	i16 b;
};

class ImageSig : public ImageBase{
private:
	struct RGBSig *data;

public:
	/* 点 (x, y) の colorId (0:赤, 1:緑, 2:青) の色を取り出す */
	int getData(int x, int y, int colorId);
	int getData_NoSecure(int x, int y, int colorId);
	/* 点 (x, y) の colorId (0:赤, 1:緑, 2:青) に色 color を代入 */
	void setData(int x, int y, int colorId, int color);
	void setData_NoSecure(int x, int y, int colorId, int color);

	/* 一時使用目的で符号付RGBデータを取得。
	成功すれば 0 を、失敗すれば 0 以外を返す */
	int create_Image(int width, int height);

	/* 一時使用目的で用意したRGBデータの解放 */
	void free_Image(void);

	/* 4近傍の Laplacian Filter */
	/* 戻り値: 0で成功, それ以外で失敗。 */
	int lapFilter(class Image *src, int x1, int y1);

	/* Filter を n 倍に増幅する */
	/* 戻り値: 0で成功, それ以外で失敗。 */
	int amplifyFilter(double n);

	/* 画像 SrcImg1 と SrcImg2 のうち、差分の大きい方を選択して混合したものから Laplacian Filter を生成。 */
	/* 戻り値: 0で成功, それ以外で失敗。 */
	int selectStrongerGradientAndMix(Image *srcImg1, int x11, int y11, Image *srcImg2, int x21, int y21);

	/* filename を読み込む。
	成功すれば 0 を、失敗すれば 0 以外を返す */
	int read_ImgSig(char *filename);

	/* filename に書き込む。
	成功すれば 0 を、失敗すれば 0 以外を返す */
	int write_ImgSig(char *filename);

	/* 画像領域を確保しているか確認 (true:確保済み, false:確保していない) */
	bool checkReserve(void);

	ImageSig &operator=(ImageSig &src);

	/* constractor */
	ImageSig(void);
	/* destractor */
	~ImageSig(void);
};

/* Poisson 方程式を解く */
int solvePoisson(Image *colorImg, ImageSig *lapImg, int x1, int y1, BitMask *regionMask);

/* 単純合成 */
int straightCompose(Image *destImg, int destX1, int destY1, Image *srcImg, int srcX1, int srcY1, BitMask *regionMask);

/* srcからdestへRGB情報をコピー */
//void imageSigCopy(ImageSig_t *dest, ImageSig_t *src);

#endif	/* FILTER_H */
