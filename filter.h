
#ifndef	FILTER_H
#define	FILTER_H

#include "bitmap.h"
#include "bitmask.h"
#include "mymath.h"
#include <cmath>	/* sqrt, abs のため */

typedef short i16;	/* 符号付16bit整数 */

struct RgbSig_t{
	i16 r;
	i16 g;
	i16 b;
};

class ImageSig_t : public ImageBase_t{
	private: struct RgbSig_t *data;

	/* 点 (x, y) の ColorId (0:赤, 1:緑, 2:青) の色を取り出す */
	public: int GetData(int x, int y, int ColorId);
	public: int GetData_NoSecure(int x, int y, int ColorId);
	/* 点 (x, y) の ColorId (0:赤, 1:緑, 2:青) に色 Color を代入 */
	public: void SetData(int x, int y, int ColorId, int Color);
	public: void SetData_NoSecure(int x, int y, int ColorId, int Color);

	/* 一時使用目的で符号付RGBデータを取得。
	成功すれば 0 を、失敗すれば 0 以外を返す */
	public: int Create_Image(int width, int height);

	/* 一時使用目的で用意したRGBデータの解放 */
	public: void Free_Image(void);

	/* 4近傍の Laplacian Filter */
	/* 戻り値: 0で成功, それ以外で失敗。 */
	public: int LapFilter(class Image_t &src, int x1, int y1);

	/* Filter を n 倍に増幅する */
	/* 戻り値: 0で成功, それ以外で失敗。 */
	public: int AmplifyFilter(double n);

	/* 画像 SrcImg1 と SrcImg2 のうち、差分の大きい方を選択して混合したものから Laplacian Filter を生成。 */
	/* 戻り値: 0で成功, それ以外で失敗。 */
	public: int SelectStrongerGradientAndMix(class Image_t &SrcImg1, int X11, int Y11, class Image_t &SrcImg2, int X21, int Y21);

	/* filename を読み込む。
	成功すれば 0 を、失敗すれば 0 以外を返す */
	public: int Read_ImgSig(char *filename);

	/* filename に書き込む。
	成功すれば 0 を、失敗すれば 0 以外を返す */
	public: int Write_ImgSig(char *filename);

	/* 画像領域を確保しているか確認 (true:確保済み, false:確保していない) */
	public: bool CheckReserve(void);

	public: ImageSig_t &operator=(ImageSig_t &Src);

	/* constractor */
	public: ImageSig_t(void);
	/* destractor */
	public: ~ImageSig_t(void);
};

/* Poisson 方程式を解く */
int SolvePoisson(class Image_t &ColorImg, class ImageSig_t &LapImg, int x1, int y1, class BitMask_t &RegionMask);

/* 単純合成 */
int StraightCompose(class Image_t &DestImg, int destX1, int destY1, class Image_t &SrcImg, int srcX1, int srcY1, class BitMask_t &RegionMask);

/* srcからdestへRGB情報をコピー */
void ImageSigCopy(class ImageSig_t &dest, class ImageSig_t &src);

#endif	/* FILTER_H */
