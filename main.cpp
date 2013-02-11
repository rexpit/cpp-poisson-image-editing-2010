/*
Poisson Image Editing
20100811~20100817
Coded by Loxyplst (rexpit) (http://rexpit.blog29.fc2.com/)
Specian Thanks: 物理のかぎしっぽ (http://hooktail.sub.jp/)
 Poisson Image Editing (http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.133.6932&rep=rep1&type=pdf)
*/

#pragma warning( disable : 4996 )	/* strn??? の警告抑制 (VC++) */
#include <cstdio>
#include <cmath>	/* sqrt, abs のため */
#include <cstring>	/* file 名設定のため */
#include <string>	/* file 名設定のため */
#include "bitmap.h"
#include "filter.h"
#include "mystdio.h"	/* 文字入力のため */
#include "bitmask.h"
#include "mymath.h"	/* maxNum や minNum を使う */

#define	MODE_EXIT	0
#define	MODE_PIE	1	/* Poisson 画像合成 */
#define	MODE_ST_CM	2	/* 単純合成 */
#define	MODE_BMP2BMP_LAP	3	/* Laplacian */
#define	MODE_BMP2BMP_GS	4	/* Grayscale */


#define	ARGFLAG_SRCIMG	0	/* 元画像があるか */
#define	ARGFLAG_TRGIMG	1	/* 対象画像があるか */
#define	ARGFLAG_MSKIMG	2	/* Mask 画像があるか */
#define	ARGFLAG_OUTIMG	3	/* 出力 file 名があるか */
#define	ARGFLAG_SRCPTN1	4	/* Source の合成領域を囲む四角形の左上端点 (x1,y1) が入力されているか */
#define	ARGFLAG_SRCPTN2	5	/* 同左上端点 (x2,y2) が入力されているか */
#define	ARGFLAG_TRGPTN1	6	/* Target の合成領域を囲む四角形の左上端点 (x1,y1) が入力されているか */
#define	ARGFLAG_DTSP	7	/* Target と Source の差が入力されているか */
#define	ARGFLAG_SC	8	/* 単純合成 */
#define	ARGFLAG_GS	9	/* Grayscale */
#define	ARGFLAG_AMP	10	/* 増幅 */
#define	ARGFLAG_MIX	11	/* 2つの Laplacian を混合 */
#define	ARGFLAG_FLAGNUM	12	/* flag 数 */

#define	FILEARGV_SRC	0	/* 合成元画像 */
#define	FILEARGV_TRG	1	/* 合成対象画像 */
#define	FILEARGV_MSK	2	/* マスク画像 */
#define	FILEARGV_OUT	3	/* 出力画像 */
#define	FILEARGV_FILENUM	4	/* file 数 */

#define	ARGPT_SRC1	0	/* 元画像の左上端点 */
#define	ARGPT_SRC2	1	/* 同左上端点 */
#define	ARGPT_TRG1	2	/* 対象画像の左上端点 */
#define	ARGPT_DTS	3	/* 対象画像と元画像の平行移動 */
#define	ARGPT_PTNUM	4	/* point 数 */

typedef unsigned int uint;
typedef unsigned char byte;

/* Command Line 引数から得られる情報 */
struct ArgInfo_t{
	char *FileArgv[FILEARGV_FILENUM];
	bool Flag[ARGFLAG_FLAGNUM];
	double AmpN;	/* Filter の増幅倍率 */
	int PtX[ARGPT_PTNUM], PtY[ARGPT_PTNUM];	/* 合成境界などの情報 */
};

/* 領域 Mask 設定 */
int SetRegionMask_BeforeCompose(class BitMask_t &RegionMsk, struct ArgInfo_t &ArgInfo, int &SrcX1, int &SrcY1, int &TrgX1, int &TrgY1);

/* Poisson 画像合成用擬似 main 関数 */
int PIE_Main(struct ArgInfo_t &ArgInfo);

/* 単純合成用擬似 main 関数 */
int StCm_Main(struct ArgInfo_t &ArgInfo);

/* BMP を BMP (Laplacian Image) に (擬似 main 関数) */
int bmp2bmp_lap(struct ArgInfo_t &ArgInfo);

/* BMP を BMP (Grayscale Image) に (擬似 main 関数) */
int bmp2bmp_gs(struct ArgInfo_t &ArgInfo);

int main(int argc, char *argv[]){
	int mode = MODE_EXIT, FileArgc = 0;
	struct ArgInfo_t ArgInfo;

	/* 引数 filter (機能選択) */
	if(argc < 2){
		fputs("Poisson Image Editing\nby Loxyplst (rexpit)\n\n Sorry, I'm not good at English. But I wrote in English. Therefore you may be unable to understand my intention. If so, please read Japanese readme.txt.\n\nPoisson Image Editing\nUsage: program [Options]\n or program [Options] <SourceImage> <TargetImage> [MaskImage]\nOption\n (none) Poisson Image Editing (default)\n -s <File name>\n  Source image file name (necessary)\n -t <File name>\n  Target image file name (necessary)\n -m <File name>\n  Mask image file name\n -o <File name>\n  Output image file name\n -sc\n  Straight compose (is not Poisson Image Editing.)\n -gs\n  Make source image into grayscale before making laplacian filter of source image\n -amp <Amplification Rate>\n  Amplify source image's laplacian filter before Poisson Image Editing.\n -mix\n  Select stronger filter and mix.\n -sp1 <Sx1> <Sy1>\n  The left top point of rectangle that encircle source image's region.\n -sp2 <Sx2> <Sy2>\n  The right bottom point of rectangle that encircle source image's region. (This option is used when you do not use mask image.)\n -tp1 <Tx1> <Ty1>\n  The left top point of rectangle that encircle target image's region. (This option is used when you do not use mask image.)\n -dts <Dx> <Dy>\n  The difference (xt - xs, yt - ys) between point (xt, yt) of target image and point (xs, ys) of source image. (This option is used when you use mask image.)\n\nLaplacian Filter\nUsage: program [Options] <SourceImage>\nOption\n (none) Laplacian Filter (default)\n -gs Grayscale\n", stdout);
		return 0;
	}
	for(int i = 0; i < ARGFLAG_FLAGNUM; ++i){	/* 全 flag を off に */
		ArgInfo.Flag[i] = false;
	}
	for(int i = 0; i < FILEARGV_FILENUM; ++i){	/* 全 FileArgv を NULL に */
		ArgInfo.FileArgv[i] = NULL;
	}
	for(int i = 1; i < argc; ++i){
		if(argv[i][0] == '-'){
			if(strcmp(argv[i] + 1, "s") == 0){
				/* 元画像 file 名 */
				++i;
				if(i >= argc){
					fputs("ERROR: If you use the option -s, you must add source image file name in the next.\n", stderr);
					return 1;
				}
				ArgInfo.FileArgv[FILEARGV_SRC] = argv[i];
				ArgInfo.Flag[ARGFLAG_SRCIMG] = true;
			}else if(strcmp(argv[i] + 1, "t") == 0){
				/* 対象画像 file 名 */
				++i;
				if(i >= argc){
					fputs("ERROR: If you use the option -t, you must add target image file name in the next.\n", stderr);
					return 1;
				}
				ArgInfo.FileArgv[FILEARGV_TRG] = argv[i];
				ArgInfo.Flag[ARGFLAG_TRGIMG] = true;
			}else if(strcmp(argv[i] + 1, "m") == 0){
				/* mask 画像 file 名 */
				++i;
				if(i >= argc){
					fputs("ERROR: If you use the option -m, you must add mask image file name in the next.\n", stderr);
					return 1;
				}
				ArgInfo.FileArgv[FILEARGV_MSK] = argv[i];
				ArgInfo.Flag[ARGFLAG_MSKIMG] = true;
			}else if(strcmp(argv[i] + 1, "o") == 0){
				/* 出力画像 file 名 */
				++i;
				if(i >= argc){
					fputs("ERROR: If you use the option -o, you must add output image file name in the next.\n", stderr);
					return 1;
				}
				ArgInfo.FileArgv[FILEARGV_OUT] = argv[i];
				ArgInfo.Flag[ARGFLAG_OUTIMG] = true;
			}else if(strcmp(argv[i] + 1, "sp1") == 0){
				/* 元画像の合成元領域を囲む長方形の左上端点 (x1,y1) の入力 */
				char *ErrMsgInOptSp1 = "ERROR: If you use the option -sp1, you must add source's x1 and y1 in the next.\n", *EndPtr;
				if(i + 2 >= argc){
					fputs(ErrMsgInOptSp1, stderr);
					return 1;
				}
				++i;
				ArgInfo.PtX[ARGPT_SRC1] = strtol(argv[i], &EndPtr, 10);
				if(*EndPtr != '\0'){
					fputs(ErrMsgInOptSp1, stderr);
					return 1;
				}
				++i;
				ArgInfo.PtY[ARGPT_SRC1] = strtol(argv[i], &EndPtr, 10);
				if(*EndPtr != '\0'){
					fputs(ErrMsgInOptSp1, stderr);
					return 1;
				}
				ArgInfo.Flag[ARGFLAG_SRCPTN1] = true;
			}else if(strcmp(argv[i] + 1, "sp2") == 0){
				/* 元画像の合成元領域を囲む長方形の右下端点 (x2,y2) の入力 */
				char *ErrMsgInOptSp2 = "ERROR: If you use the option -sp2, you must add source's x2 and y2 in the next.\n", *EndPtr;
				if(i + 2 >= argc){
					fputs(ErrMsgInOptSp2, stderr);
					return 1;
				}
				++i;
				ArgInfo.PtX[ARGPT_SRC2] = strtol(argv[i], &EndPtr, 10);
				if(*EndPtr != '\0'){
					fputs(ErrMsgInOptSp2, stderr);
					return 1;
				}
				++i;
				ArgInfo.PtY[ARGPT_SRC2] = strtol(argv[i], &EndPtr, 10);
				if(*EndPtr != '\0'){
					fputs(ErrMsgInOptSp2, stderr);
					return 1;
				}
				ArgInfo.Flag[ARGFLAG_SRCPTN2] = true;
			}else if(strcmp(argv[i] + 1, "tp1") == 0){
				/* 対象画像の合成先領域を囲む長方形の左上端点 (x1,y1) の入力 */
				char *ErrMsgInOptTp1 = "ERROR: If you use the option -tp1, you must add target's x1 and y1 in the next.\n", *EndPtr;
				if(i + 2 >= argc){
					fprintf(stderr, ErrMsgInOptTp1, argv[i] + 1);
					return 1;
				}
				++i;
				ArgInfo.PtX[ARGPT_TRG1] = strtol(argv[i], &EndPtr, 10);
				if(*EndPtr != '\0'){
					fprintf(stderr, ErrMsgInOptTp1, argv[i] + 1);
					return 1;
				}
				++i;
				ArgInfo.PtY[ARGPT_TRG1] = strtol(argv[i], &EndPtr, 10);
				if(*EndPtr != '\0'){
					fprintf(stderr, ErrMsgInOptTp1, argv[i] + 1);
					return 1;
				}
				ArgInfo.Flag[ARGFLAG_TRGPTN1] = true;
			}else if(strcmp(argv[i] + 1, "dts") == 0){
				/* 対象画像上の点 (xt,yt) と元画像上の点 (xs,ys) の平行移動 (Δx,Δy) = (xt-xs,yt-ys) の入力 */
				char *ErrMsgInOptDts = "ERROR: If you use the option -%s, you must add target's x1 and y1 in the next.\n", *EndPtr;
				if(i + 2 >= argc){
					fprintf(stderr, ErrMsgInOptDts, argv[i] + 1);
					return 1;
				}
				++i;
				ArgInfo.PtX[ARGPT_DTS] = strtol(argv[i], &EndPtr, 10);
				if(*EndPtr != '\0'){
					fprintf(stderr, ErrMsgInOptDts, argv[i] + 1);
					return 1;
				}
				++i;
				ArgInfo.PtY[ARGPT_DTS] = strtol(argv[i], &EndPtr, 10);
				if(*EndPtr != '\0'){
					fprintf(stderr, ErrMsgInOptDts, argv[i] + 1);
					return 1;
				}
				ArgInfo.Flag[ARGFLAG_DTSP] = true;
			}else if(strcmp(argv[i] + 1, "sc") == 0){
				/* 単純合成 */
				ArgInfo.Flag[ARGFLAG_SC] = true;
			}else if(strcmp(argv[i] + 1, "gs") == 0){
				/* Grayscale */
				ArgInfo.Flag[ARGFLAG_GS] = true;
			}else if(strcmp(argv[i] + 1, "amp") == 0){
				/* Filter の増幅 */
				char *ErrMsgInOptAmp = "ERROR: If you use the option -amp, you must add an amplification rate in the next.\n", *EndPtr;
				++i;
				if(i >= argc){
					fputs(ErrMsgInOptAmp, stderr);
					return 1;
				}
				ArgInfo.AmpN = strtod(argv[i], &EndPtr);
				if(*EndPtr != '\0'){
					fputs(ErrMsgInOptAmp, stderr);
					return 1;
				}
				ArgInfo.Flag[ARGFLAG_AMP] = true;
			}else if(strcmp(argv[i] + 1, "mix") == 0){
				/* Filter の混合 */
				ArgInfo.Flag[ARGFLAG_MIX] = true;
			}else{
				fputs("ERROR: There are undefined options.\n", stderr);
				return 1;
			}
		}else{
			switch(FileArgc){
				case FILEARGV_SRC:
					ArgInfo.Flag[ARGFLAG_SRCIMG] = true;
					break;
				case FILEARGV_TRG:
					ArgInfo.Flag[ARGFLAG_TRGIMG] = true;
					break;
				case FILEARGV_MSK:
					ArgInfo.Flag[ARGFLAG_MSKIMG] = true;
					break;
				default:
					fputs("ERROR: Outside assumption.\n", stderr);
					return 1;
					break;
			}
			ArgInfo.FileArgv[FileArgc] = argv[i];
			++FileArgc;
		}
	}
	if(!ArgInfo.Flag[ARGFLAG_SRCIMG]){	/* 何もできない系 */
		fputs("ERROR: Arguments are illegal.\n", stderr);
		return 1;
	}else if(ArgInfo.Flag[ARGFLAG_TRGIMG] || ArgInfo.Flag[ARGFLAG_AMP]){	/* 複数画像合成系 */
		if(!ArgInfo.Flag[ARGFLAG_TRGIMG]){
			ArgInfo.FileArgv[FILEARGV_TRG] = ArgInfo.FileArgv[FILEARGV_SRC];
			ArgInfo.Flag[ARGFLAG_TRGIMG] = true;
		}
		mode = ArgInfo.Flag[ARGFLAG_SC] ? MODE_ST_CM : MODE_PIE;
	}else if(!ArgInfo.Flag[ARGFLAG_TRGIMG] && !ArgInfo.Flag[ARGFLAG_MSKIMG]){	/* 単画像処理系 */
		if(ArgInfo.Flag[ARGFLAG_SC]){
			fputs("ERROR: Arguments are illegal.\n", stderr);
			return 1;
		}
		mode = ArgInfo.Flag[ARGFLAG_GS] ? MODE_BMP2BMP_GS : MODE_BMP2BMP_LAP;
	}else{
		fputs("ERROR: Arguments are illegal.\n", stderr);
		return 1;
	}

	switch(mode){
		case MODE_PIE:
			return PIE_Main(ArgInfo);
			break;
		case MODE_ST_CM:
			return StCm_Main(ArgInfo);
			break;
		case MODE_BMP2BMP_LAP:
			return bmp2bmp_lap(ArgInfo);
			break;
		case MODE_BMP2BMP_GS:
			return bmp2bmp_gs(ArgInfo);
			break;
		default:
			return 0;
			break;
	}

	return 0;
}

int SetRegionMask_BeforeCompose(class BitMask_t &RegionMsk, struct ArgInfo_t &ArgInfo, int &SrcX1, int &SrcY1, int &TrgX1, int &TrgY1) {
	int SrcX2, SrcY2;
	char yn[4];
	if(!ArgInfo.Flag[ARGFLAG_SRCIMG] || !ArgInfo.Flag[ARGFLAG_TRGIMG]){
		return 1;
	}
	if(!ArgInfo.Flag[ARGFLAG_MSKIMG]){
		if(ArgInfo.Flag[ARGFLAG_SRCPTN1] && ArgInfo.Flag[ARGFLAG_SRCPTN2] && ArgInfo.Flag[ARGFLAG_TRGPTN1]){	/* Command Line 引数に必要事項が書かれていたとき */
			SrcX1 = ArgInfo.PtX[ARGPT_SRC1];
			SrcY1 = ArgInfo.PtY[ARGPT_SRC1];
			SrcX2 = ArgInfo.PtX[ARGPT_SRC2];
			SrcY2 = ArgInfo.PtY[ARGPT_SRC2];
			if(SrcX1 > SrcX2 || SrcY1 > SrcY2){
				fputs("ERROR: 座標の関係が想定外です。", stderr);
				return 1;
			}
			TrgX1 = ArgInfo.PtX[ARGPT_TRG1];
			TrgY1 = ArgInfo.PtY[ARGPT_TRG1];
		}else{
			while(true){
				fprintf(stderr, "合成元画像 (%s) の選択範囲 (x1,y1)~(x2,y2):\n", ArgInfo.FileArgv[FILEARGV_SRC]);
				fputs("x1 = ", stderr);
				SrcX1 = inputInt(stdin);
				fputs("y1 = ", stderr);
				SrcY1 = inputInt(stdin);
				fputs("x2 = ", stderr);
				SrcX2 = inputInt(stdin);
				fputs("y2 = ", stderr);
				SrcY2 = inputInt(stdin);
				if(SrcX1 > SrcX2 || SrcY1 > SrcY2){
					fputs("ERROR: 座標の関係が想定外です。\n", stderr);
					return 1;
				}
				fprintf(stderr, "\n合成対象画像 (%s) の合成先領域の左上端 (x,y):\n", ArgInfo.FileArgv[FILEARGV_TRG]);
				fputs("x = ", stderr);
				TrgX1 = inputInt(stdin);
				fputs("y = ", stderr);
				TrgY1 = inputInt(stdin);

				fputs("\n設定は以上でよろしいですか? [Y/N]:", stderr);
				myFgets(yn, sizeof(yn) / sizeof(yn[0]), stdin);
				if(yn[0] == 'Y' || yn[0] == 'y'){
					break;
				}else{
					fputs("もう一度設定しますか? (設定しない場合、終了します。)\n[Y:再設定, N:終了]:", stderr);
					myFgets(yn, sizeof(yn) / sizeof(yn[0]), stdin);
					if(yn[0] != 'Y' && yn[0] != 'y'){
						return 1;
					}
				}
			}
		}
		if(RegionMsk.Create_Image(SrcX2 - SrcX1 + 1, SrcY2 - SrcY1 + 1)){ return 1; }
		RegionMsk.Fill_BitMask();
	}else{	/* mask が与えられたとき */
		class Image_t RegionImg;
		int TrgX2, TrgY2;
		if(RegionImg.Read_Bmp(ArgInfo.FileArgv[FILEARGV_MSK])){ return 1; }
		if(RegionMsk.Img2Mask(RegionImg)){ return 1; }

		if(ArgInfo.Flag[ARGFLAG_DTSP]){	/* Command Line 引数に必要事項が書かれていたとき */
			if(ArgInfo.Flag[ARGFLAG_SRCPTN1]){
				SrcX1 = ArgInfo.PtX[ARGPT_SRC1];
				SrcY1 = ArgInfo.PtY[ARGPT_SRC1];
			}else{
				SrcX1 = 0;
				SrcY1 = 0;
			}
			SrcX1 += RegionMsk.x1;
			SrcY1 += RegionMsk.y1;
			TrgX1 = ArgInfo.PtX[ARGPT_DTS] + SrcX1;
			TrgY1 = ArgInfo.PtY[ARGPT_DTS] + SrcY1;
		}else{
			while(true){
				fprintf(stderr, "マスク画像 (%s) の左上端は合成元画像 (%s) 上のどの点ですか? (x, y):\n", ArgInfo.FileArgv[FILEARGV_MSK], ArgInfo.FileArgv[FILEARGV_SRC]);
				fputs("x = ", stderr);
				SrcX1 = inputInt(stdin);
				fputs("y = ", stderr);
				SrcY1 = inputInt(stdin);
				fprintf(stderr, "マスク画像上のマスク領域を囲む長方形は、マスク画像上の点 (%d,%d)~(%d,%d) であると判断しました。\n", RegionMsk.x1, RegionMsk.y1, RegionMsk.x1 + RegionMsk.GetWidth() - 1, RegionMsk.y1 + RegionMsk.GetHeight() - 1);
				SrcX1 += RegionMsk.x1;
				SrcY1 += RegionMsk.y1;
				SrcX2 = SrcX1 + RegionMsk.GetWidth() - 1;
				SrcY2 = SrcY1 + RegionMsk.GetHeight() - 1;
				fprintf(stderr, "よって、合成元画像上での合成領域を囲む長方形は、合成元画像上の点 (%d,%d)~(%d,%d) となります。\n", SrcX1, SrcY1, SrcX2, SrcY2);

				fprintf(stderr, "合成対象画像 (%s) 上の点 (xt, yt) と合成元画像上の点 (xs, ys) のズレ (Δx, Δy) = (xt - xs, yt - ys) :\n", ArgInfo.FileArgv[FILEARGV_TRG]);
				fputs("Δx = ", stderr);
				TrgX1 = inputInt(stdin);
				fputs("Δy = ", stderr);
				TrgY1 = inputInt(stdin);
				TrgX1 += SrcX1; TrgX2 = TrgX1 + (int)RegionMsk.GetWidth() - 1;
				TrgY1 += SrcY1; TrgY2 = TrgY1 + (int)RegionMsk.GetHeight() - 1;
				fprintf(stderr, "よって、対象画像上のマスク領域を囲む長方形は、対象画像上の点 (%d,%d)~(%d,%d) となります。\n", TrgX1, TrgY1, TrgX2, TrgY2);

				fputs("\n設定は以上でよろしいですか? [Y/N]:", stderr);
				myFgets(yn, sizeof(yn) / sizeof(yn[0]), stdin);
				if(yn[0] == 'Y' || yn[0] == 'y'){
					break;
				}else{
					fputs("もう一度設定しますか? (設定しない場合、終了します。)\n[Y:再設定, N:終了]:", stderr);
					myFgets(yn, sizeof(yn) / sizeof(yn[0]), stdin);
					if(yn[0] != 'Y' && yn[0] != 'y'){
						return 1;
					}
				}
			}
		}
	}

	return 0;
}

int PIE_Main(struct ArgInfo_t &ArgInfo){
	class Image_t SrcImg, TrgImg;
	class ImageSig_t SrcLap;
	class BitMask_t RegionMsk;
	int SrcX1, SrcY1, SrcX2, SrcY2, TrgX1, TrgY1, TrgX2, TrgY2;
	std::string OutputFileName;

	/* 引数判定・合成領域指定 */
	if(SetRegionMask_BeforeCompose(RegionMsk, ArgInfo, SrcX1, SrcY1, TrgX1, TrgY1)){
		return 1;
	}
	SrcX2 = SrcX1 + RegionMsk.GetWidth() - 1; SrcY2 = SrcY1 + RegionMsk.GetHeight() - 1;
	TrgX2 = TrgX1 + RegionMsk.GetWidth() - 1; TrgY2 = TrgY1 + RegionMsk.GetHeight() - 1;

	/* BMP の読込 */
	if(SrcImg.Read_Bmp(ArgInfo.FileArgv[FILEARGV_SRC])){
		fprintf(stderr, "ERROR: Cannot read the file %s.\n", ArgInfo.FileArgv[FILEARGV_SRC]);
		return 1;
	}
	if(ArgInfo.Flag[ARGFLAG_GS]){
		if(SrcImg.MakeIntoGrayscale()){
			return 1;
		}
	}
	if(SrcX1 < 0 || SrcY1 < 0 || SrcX2 >= (int)SrcImg.GetWidth() || SrcY2 >= (int)SrcImg.GetHeight()){
		fputs("ERROR: 指定した領域が画像の範囲外です。\n", stderr);
		return 1;
	}
	if(TrgImg.Read_Bmp(ArgInfo.FileArgv[FILEARGV_TRG])){
		fprintf(stderr, "ERROR: Cannot read the file %s.\n", ArgInfo.FileArgv[FILEARGV_TRG]);
		return 1;
	}
	if(TrgX1 < 0 || TrgY1 < 0 || TrgX2 >= (int)TrgImg.GetWidth() || TrgY2 >= (int)TrgImg.GetHeight()){
		fputs("ERROR: 指定した領域が画像の範囲外です。\n", stderr);
		return 1;
	}

	/* Filter 前処理 */
	if(SrcLap.Create_Image(SrcX2 - SrcX1 + 1, SrcY2 - SrcY1 + 1)){
		return 1;
	}
	if(!ArgInfo.Flag[ARGFLAG_MIX]){
		if(SrcLap.LapFilter(SrcImg, SrcX1, SrcY1) != 0){
			return 1;
		}
	}else{	/* 元画像と対象画像の Filter の混合 */
		if(SrcLap.SelectStrongerGradientAndMix(SrcImg, SrcX1, SrcY1, TrgImg, TrgX1, TrgY1)){
			return 1;
		}
	}
	if(ArgInfo.Flag[ARGFLAG_AMP]){	/* 増幅 */
		if(SrcLap.AmplifyFilter(ArgInfo.AmpN)){
			return 1;
		}
	}
	/* 画像処理 */
	if(SolvePoisson(TrgImg, SrcLap, TrgX1, TrgY1, RegionMsk) != 0){
		return 1;
	}

	/* trgImg を BMP に出力 */
	if(ArgInfo.Flag[ARGFLAG_OUTIMG]){
		OutputFileName = ArgInfo.FileArgv[FILEARGV_OUT];
	}else{
		OutputFileName = ArgInfo.FileArgv[FILEARGV_TRG];
		OutputFileName += "_pie.bmp";
	}
	if(TrgImg.Write_Bmp((char *)OutputFileName.c_str())){
		fprintf(stderr, "ERROR: Cannot write the file %s.\n", OutputFileName.c_str());
		return 1;
	}

	return 0;
}

int StCm_Main(struct ArgInfo_t &ArgInfo){
	class Image_t SrcImg, TrgImg;
	class BitMask_t RegionMsk;
	int SrcX1, SrcY1, SrcX2, SrcY2, TrgX1, TrgY1, TrgX2, TrgY2;
	std::string OutputFileName;

	/* 引数判定・合成領域指定 */
	if((SetRegionMask_BeforeCompose(RegionMsk, ArgInfo, SrcX1, SrcY1, TrgX1, TrgY1))){
		return 1;
	}
	SrcX2 = SrcX1 + RegionMsk.GetWidth() - 1; SrcY2 = SrcY1 + RegionMsk.GetHeight() - 1;
	TrgX2 = TrgX1 + RegionMsk.GetWidth() - 1; TrgY2 = TrgY1 + RegionMsk.GetHeight() - 1;

	/* BMP の読込 */
	if(SrcImg.Read_Bmp(ArgInfo.FileArgv[FILEARGV_SRC])){
		fprintf(stderr, "ERROR: Cannot read the file %s.\n", ArgInfo.FileArgv[FILEARGV_SRC]);
		return 1;
	}
	if(SrcX1 < 0 || SrcY1 < 0 || SrcX2 >= (int)SrcImg.GetWidth() || SrcY2 >= (int)SrcImg.GetHeight()){
		fputs("ERROR: 指定した領域が画像の範囲外です。\n", stderr);
		return 1;
	}
	if(TrgImg.Read_Bmp(ArgInfo.FileArgv[FILEARGV_TRG])){
		fprintf(stderr, "ERROR: Cannot read the file %s.\n", ArgInfo.FileArgv[FILEARGV_TRG]);
		return 1;
	}
	if(TrgX1 < 0 || TrgY1 < 0 || TrgX2 >= (int)TrgImg.GetWidth() || TrgY2 >= (int)TrgImg.GetHeight()){
		fputs("ERROR: 指定した領域が画像の範囲外です。\n", stderr);
		return 1;
	}

	/* 画像処理 */
	if(StraightCompose(TrgImg, TrgX1, TrgY1, SrcImg, SrcX1, SrcY1, RegionMsk) != 0){
		return 1;
	}

	/* trgImg を BMP に出力 */
	if(ArgInfo.Flag[ARGFLAG_OUTIMG]){
		OutputFileName = ArgInfo.FileArgv[FILEARGV_OUT];
	}else{
		OutputFileName = ArgInfo.FileArgv[FILEARGV_TRG];
		OutputFileName += "_stcm.bmp";
	}
	if(TrgImg.Write_Bmp((char *)OutputFileName.c_str())){
		fprintf(stderr, "ERROR: Cannot write the file %s.\n", OutputFileName.c_str());
		return 1;
	}

	return 0;
}

int bmp2bmp_lap(struct ArgInfo_t &ArgInfo){
	/* 変数宣言 */
	class Image_t ColorImg;	/* 画像 */
	class ImageSig_t LapImg;	/* Laplacian */
	std::string OutputFileName;	/* 出力 file 名 */
	fputs("BMP2BMP (Laplacian Image)\n", stderr);

	/* 画像 file を ColorImg に入力 */
	if(ColorImg.Read_Bmp(ArgInfo.FileArgv[FILEARGV_SRC])){
		fprintf(stderr, "ERROR: Cannot read the file %s.\n", ArgInfo.FileArgv[FILEARGV_SRC]);
		return 1;
	}
	/* BMP と同じ size の符号付画像を作成 */
	if(LapImg.Create_Image(ColorImg.GetWidth(), ColorImg.GetHeight())){
		fputs("ERROR\n", stderr);
		return 1;
	}

	/* 画像処理 */
	if((LapImg.LapFilter(ColorImg, 0, 0)) != 0){
		return 1;
	}
	for(int y = 0; y < (int)ColorImg.GetHeight(); ++y){
		for(int x = 0; x < (int)ColorImg.GetWidth(); ++x){
			for(int i = 0; i < 3; ++i){
				ColorImg.SetData_NoSecure(x, y, i, minNum((int)abs(LapImg.GetData_NoSecure(x, y, i)), 255));
			}
		}
	}

	/* ColorImg を画像 file に出力 */
	if(ArgInfo.Flag[ARGFLAG_OUTIMG]){
		OutputFileName = ArgInfo.FileArgv[FILEARGV_OUT];
	}else{
		OutputFileName = ArgInfo.FileArgv[FILEARGV_SRC];
		OutputFileName += "_lap.bmp";
	}
	if(ColorImg.Write_Bmp((char *)OutputFileName.c_str())){
		fprintf(stderr, "ERROR: Cannot write the file %s.\n", OutputFileName.c_str());
		return 1;
	}

	return 0;
}

int bmp2bmp_gs(struct ArgInfo_t &ArgInfo){
	/* 変数宣言 */
	class Image_t ColorImg;	/* 画像 */
	std::string OutputFileName;	/* 出力 file 名 */
	fputs("BMP2BMP (Grayscale Image)\n", stderr);

	/* 画像 file を ColorImg に入力 */
	if(ColorImg.Read_Bmp(ArgInfo.FileArgv[FILEARGV_SRC])){
		fprintf(stderr, "ERROR: Cannot read the file %s.\n", ArgInfo.FileArgv[FILEARGV_SRC]);
		return 1;
	}

	/* 画像処理 */
	if(ColorImg.MakeIntoGrayscale()){
		return 1;
	}

	/* ColorImg を画像 file に出力 */
	if(ArgInfo.Flag[ARGFLAG_OUTIMG]){
		OutputFileName = ArgInfo.FileArgv[FILEARGV_OUT];
	}else{
		OutputFileName = ArgInfo.FileArgv[FILEARGV_SRC];
		OutputFileName += "_gs.bmp";
	}
	if(ColorImg.Write_Bmp((char *)OutputFileName.c_str())){
		fprintf(stderr, "ERROR: Cannot write the file %s.\n", OutputFileName.c_str());
		return 1;
	}

	return 0;
}
