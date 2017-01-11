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
#include <algorithm>	/* min, max のため */
#include <cstring>	/* file 名設定のため */
#include <string>	/* file 名設定のため */
#include "bitmap.h"
#include "filter.h"
#include "mystdio.h"	/* ファイルからの文字入力のため */
#include "bitmask.h"

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
struct ArgInfo{
	std::string fileArgv[FILEARGV_FILENUM];
	bool flag[ARGFLAG_FLAGNUM];
	double ampN;	/* Filter の増幅倍率 */
	int ptX[ARGPT_PTNUM], ptY[ARGPT_PTNUM];	/* 合成境界などの情報 */
};

/* 領域 Mask 設定 */
int setRegionMask_BeforeCompose(BitMask *regionMsk, const ArgInfo &argInfo, int *srcX1, int *srcY1, int *trgX1, int *trgY1);

/* Poisson 画像合成用擬似 main 関数 */
int pIE_Main(const ArgInfo &argInfo);

/* 単純合成用擬似 main 関数 */
int stCm_Main(const ArgInfo &argInfo);

/* BMP を BMP (Laplacian Image) に (擬似 main 関数) */
int bmp2bmp_lap(const ArgInfo &argInfo);

/* BMP を BMP (Grayscale Image) に (擬似 main 関数) */
int bmp2bmp_gs(const ArgInfo &argInfo);

int main(const int argc, const char *argv[]){
	int mode = MODE_EXIT, fileArgc = 0;
	ArgInfo argInfo;

	/* 引数 filter (機能選択) */
	if(argc < 2){
		fputs("Poisson Image Editing\nby Loxyplst (rexpit)\n\n Sorry, I'm not good at English. But I wrote in English. Therefore you may be unable to understand my intention. If so, please read Japanese readme.txt.\n\nPoisson Image Editing\nUsage: program [Options]\n or program [Options] <SourceImage> <TargetImage> [MaskImage]\nOption\n (none) Poisson Image Editing (default)\n -s <File name>\n  Source image file name (necessary)\n -t <File name>\n  Target image file name (necessary)\n -m <File name>\n  Mask image file name\n -o <File name>\n  Output image file name\n -sc\n  Straight compose (is not Poisson Image Editing.)\n -gs\n  Make source image into grayscale before making laplacian filter of source image\n -amp <Amplification Rate>\n  Amplify source image's laplacian filter before Poisson Image Editing.\n -mix\n  Select stronger filter and mix.\n -sp1 <Sx1> <Sy1>\n  The left top point of rectangle that encircle source image's region.\n -sp2 <Sx2> <Sy2>\n  The right bottom point of rectangle that encircle source image's region. (This option is used when you do not use mask image.)\n -tp1 <Tx1> <Ty1>\n  The left top point of rectangle that encircle target image's region. (This option is used when you do not use mask image.)\n -dts <Dx> <Dy>\n  The difference (xt - xs, yt - ys) between point (xt, yt) of target image and point (xs, ys) of source image. (This option is used when you use mask image.)\n\nLaplacian Filter\nUsage: program [Options] <SourceImage>\nOption\n (none) Laplacian Filter (default)\n -gs Grayscale\n", stdout);
		return 0;
	}
	for(int i = 0; i < ARGFLAG_FLAGNUM; ++i){	/* 全 flag を off に */
		argInfo.flag[i] = false;
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
				argInfo.fileArgv[FILEARGV_SRC] = argv[i];
				argInfo.flag[ARGFLAG_SRCIMG] = true;
			}else if(strcmp(argv[i] + 1, "t") == 0){
				/* 対象画像 file 名 */
				++i;
				if(i >= argc){
					fputs("ERROR: If you use the option -t, you must add target image file name in the next.\n", stderr);
					return 1;
				}
				argInfo.fileArgv[FILEARGV_TRG] = argv[i];
				argInfo.flag[ARGFLAG_TRGIMG] = true;
			}else if(strcmp(argv[i] + 1, "m") == 0){
				/* mask 画像 file 名 */
				++i;
				if(i >= argc){
					fputs("ERROR: If you use the option -m, you must add mask image file name in the next.\n", stderr);
					return 1;
				}
				argInfo.fileArgv[FILEARGV_MSK] = argv[i];
				argInfo.flag[ARGFLAG_MSKIMG] = true;
			}else if(strcmp(argv[i] + 1, "o") == 0){
				/* 出力画像 file 名 */
				++i;
				if(i >= argc){
					fputs("ERROR: If you use the option -o, you must add output image file name in the next.\n", stderr);
					return 1;
				}
				argInfo.fileArgv[FILEARGV_OUT] = argv[i];
				argInfo.flag[ARGFLAG_OUTIMG] = true;
			}else if(strcmp(argv[i] + 1, "sp1") == 0){
				/* 元画像の合成元領域を囲む長方形の左上端点 (x1,y1) の入力 */
				char *errMsgInOptSp1 = "ERROR: If you use the option -sp1, you must add source's x1 and y1 in the next.\n", *endPtr;
				if(i + 2 >= argc){
					fputs(errMsgInOptSp1, stderr);
					return 1;
				}
				++i;
				argInfo.ptX[ARGPT_SRC1] = strtol(argv[i], &endPtr, 10);
				if(*endPtr != '\0'){
					fputs(errMsgInOptSp1, stderr);
					return 1;
				}
				++i;
				argInfo.ptY[ARGPT_SRC1] = strtol(argv[i], &endPtr, 10);
				if(*endPtr != '\0'){
					fputs(errMsgInOptSp1, stderr);
					return 1;
				}
				argInfo.flag[ARGFLAG_SRCPTN1] = true;
			}else if(strcmp(argv[i] + 1, "sp2") == 0){
				/* 元画像の合成元領域を囲む長方形の右下端点 (x2,y2) の入力 */
				char *errMsgInOptSp2 = "ERROR: If you use the option -sp2, you must add source's x2 and y2 in the next.\n", *endPtr;
				if(i + 2 >= argc){
					fputs(errMsgInOptSp2, stderr);
					return 1;
				}
				++i;
				argInfo.ptX[ARGPT_SRC2] = strtol(argv[i], &endPtr, 10);
				if(*endPtr != '\0'){
					fputs(errMsgInOptSp2, stderr);
					return 1;
				}
				++i;
				argInfo.ptY[ARGPT_SRC2] = strtol(argv[i], &endPtr, 10);
				if(*endPtr != '\0'){
					fputs(errMsgInOptSp2, stderr);
					return 1;
				}
				argInfo.flag[ARGFLAG_SRCPTN2] = true;
			}else if(strcmp(argv[i] + 1, "tp1") == 0){
				/* 対象画像の合成先領域を囲む長方形の左上端点 (x1,y1) の入力 */
				char *errMsgInOptTp1 = "ERROR: If you use the option -tp1, you must add target's x1 and y1 in the next.\n", *endPtr;
				if(i + 2 >= argc){
					fprintf(stderr, errMsgInOptTp1, argv[i] + 1);
					return 1;
				}
				++i;
				argInfo.ptX[ARGPT_TRG1] = strtol(argv[i], &endPtr, 10);
				if(*endPtr != '\0'){
					fprintf(stderr, errMsgInOptTp1, argv[i] + 1);
					return 1;
				}
				++i;
				argInfo.ptY[ARGPT_TRG1] = strtol(argv[i], &endPtr, 10);
				if(*endPtr != '\0'){
					fprintf(stderr, errMsgInOptTp1, argv[i] + 1);
					return 1;
				}
				argInfo.flag[ARGFLAG_TRGPTN1] = true;
			}else if(strcmp(argv[i] + 1, "dts") == 0){
				/* 対象画像上の点 (xt,yt) と元画像上の点 (xs,ys) の平行移動 (Δx,Δy) = (xt-xs,yt-ys) の入力 */
				char *errMsgInOptDts = "ERROR: If you use the option -%s, you must add target's x1 and y1 in the next.\n", *endPtr;
				if(i + 2 >= argc){
					fprintf(stderr, errMsgInOptDts, argv[i] + 1);
					return 1;
				}
				++i;
				argInfo.ptX[ARGPT_DTS] = strtol(argv[i], &endPtr, 10);
				if(*endPtr != '\0'){
					fprintf(stderr, errMsgInOptDts, argv[i] + 1);
					return 1;
				}
				++i;
				argInfo.ptY[ARGPT_DTS] = strtol(argv[i], &endPtr, 10);
				if(*endPtr != '\0'){
					fprintf(stderr, errMsgInOptDts, argv[i] + 1);
					return 1;
				}
				argInfo.flag[ARGFLAG_DTSP] = true;
			}else if(strcmp(argv[i] + 1, "sc") == 0){
				/* 単純合成 */
				argInfo.flag[ARGFLAG_SC] = true;
			}else if(strcmp(argv[i] + 1, "gs") == 0){
				/* Grayscale */
				argInfo.flag[ARGFLAG_GS] = true;
			}else if(strcmp(argv[i] + 1, "amp") == 0){
				/* Filter の増幅 */
				char *errMsgInOptAmp = "ERROR: If you use the option -amp, you must add an amplification rate in the next.\n", *EndPtr;
				++i;
				if(i >= argc){
					fputs(errMsgInOptAmp, stderr);
					return 1;
				}
				argInfo.ampN = strtod(argv[i], &EndPtr);
				if(*EndPtr != '\0'){
					fputs(errMsgInOptAmp, stderr);
					return 1;
				}
				argInfo.flag[ARGFLAG_AMP] = true;
			}else if(strcmp(argv[i] + 1, "mix") == 0){
				/* Filter の混合 */
				argInfo.flag[ARGFLAG_MIX] = true;
			}else{
				fputs("ERROR: There are undefined options.\n", stderr);
				return 1;
			}
		}else{
			switch(fileArgc){
				case FILEARGV_SRC:
					argInfo.flag[ARGFLAG_SRCIMG] = true;
					break;
				case FILEARGV_TRG:
					argInfo.flag[ARGFLAG_TRGIMG] = true;
					break;
				case FILEARGV_MSK:
					argInfo.flag[ARGFLAG_MSKIMG] = true;
					break;
				default:
					fputs("ERROR: Outside assumption.\n", stderr);
					return 1;
					break;
			}
			argInfo.fileArgv[fileArgc] = argv[i];
			++fileArgc;
		}
	}
	if(!argInfo.flag[ARGFLAG_SRCIMG]){	/* 何もできない系 */
		fputs("ERROR: Arguments are illegal.\n", stderr);
		return 1;
	}else if(argInfo.flag[ARGFLAG_TRGIMG] || argInfo.flag[ARGFLAG_AMP]){	/* 複数画像合成系 */
		if(!argInfo.flag[ARGFLAG_TRGIMG]){
			argInfo.fileArgv[FILEARGV_TRG] = argInfo.fileArgv[FILEARGV_SRC];
			argInfo.flag[ARGFLAG_TRGIMG] = true;
		}
		mode = argInfo.flag[ARGFLAG_SC] ? MODE_ST_CM : MODE_PIE;
	}else if(!argInfo.flag[ARGFLAG_TRGIMG] && !argInfo.flag[ARGFLAG_MSKIMG]){	/* 単画像処理系 */
		if(argInfo.flag[ARGFLAG_SC]){
			fputs("ERROR: Arguments are illegal.\n", stderr);
			return 1;
		}
		mode = argInfo.flag[ARGFLAG_GS] ? MODE_BMP2BMP_GS : MODE_BMP2BMP_LAP;
	}else{
		fputs("ERROR: Arguments are illegal.\n", stderr);
		return 1;
	}

	switch(mode){
		case MODE_PIE:
			return pIE_Main(argInfo);
		case MODE_ST_CM:
			return stCm_Main(argInfo);
		case MODE_BMP2BMP_LAP:
			return bmp2bmp_lap(argInfo);
		case MODE_BMP2BMP_GS:
			return bmp2bmp_gs(argInfo);
		default:
			return 0;
	}
}

int setRegionMask_BeforeCompose(BitMask *regionMsk, const ArgInfo &argInfo, int *srcX1, int *srcY1, int *trgX1, int *trgY1) {
	int srcX2, srcY2;
	char yn[4];
	if(!argInfo.flag[ARGFLAG_SRCIMG] || !argInfo.flag[ARGFLAG_TRGIMG]){
		return 1;
	}
	if(!argInfo.flag[ARGFLAG_MSKIMG]){
		if(argInfo.flag[ARGFLAG_SRCPTN1] && argInfo.flag[ARGFLAG_SRCPTN2] && argInfo.flag[ARGFLAG_TRGPTN1]){	/* Command Line 引数に必要事項が書かれていたとき */
			*srcX1 = argInfo.ptX[ARGPT_SRC1];
			*srcY1 = argInfo.ptY[ARGPT_SRC1];
			srcX2 = argInfo.ptX[ARGPT_SRC2];
			srcY2 = argInfo.ptY[ARGPT_SRC2];
			if(*srcX1 > srcX2 || *srcY1 > srcY2){
				fputs("ERROR: 座標の関係が想定外です。", stderr);
				return 1;
			}
			*trgX1 = argInfo.ptX[ARGPT_TRG1];
			*trgY1 = argInfo.ptY[ARGPT_TRG1];
		}else{
			while(true){
				fprintf(stderr, "合成元画像 (%s) の選択範囲 (x1,y1)~(x2,y2):\n", argInfo.fileArgv[FILEARGV_SRC]);
				fputs("x1 = ", stderr);
				*srcX1 = inputIntFromFILE(stdin);
				fputs("y1 = ", stderr);
				*srcY1 = inputIntFromFILE(stdin);
				fputs("x2 = ", stderr);
				srcX2 = inputIntFromFILE(stdin);
				fputs("y2 = ", stderr);
				srcY2 = inputIntFromFILE(stdin);
				if(*srcX1 > srcX2 || *srcY1 > srcY2){
					fputs("ERROR: 座標の関係が想定外です。\n", stderr);
					return 1;
				}
				fprintf(stderr, "\n合成対象画像 (%s) の合成先領域の左上端 (x,y):\n", argInfo.fileArgv[FILEARGV_TRG]);
				fputs("x = ", stderr);
				*trgX1 = inputIntFromFILE(stdin);
				fputs("y = ", stderr);
				*trgY1 = inputIntFromFILE(stdin);

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
		if(regionMsk->create_Image(srcX2 - *srcX1 + 1, srcY2 - *srcY1 + 1)){ return 1; }
		regionMsk->fill_BitMask();
	}else{	/* mask が与えられたとき */
		Image regionImg;
		int trgX2, trgY2;
		if(regionImg.read_Bmp(argInfo.fileArgv[FILEARGV_MSK].c_str())){ return 1; }
		if(regionMsk->img2Mask(&regionImg)){ return 1; }

		if(argInfo.flag[ARGFLAG_DTSP]){	/* Command Line 引数に必要事項が書かれていたとき */
			if(argInfo.flag[ARGFLAG_SRCPTN1]){
				*srcX1 = argInfo.ptX[ARGPT_SRC1];
				*srcY1 = argInfo.ptY[ARGPT_SRC1];
			}else{
				*srcX1 = 0;
				*srcY1 = 0;
			}
			*srcX1 += regionMsk->x1;
			*srcY1 += regionMsk->y1;
			*trgX1 = argInfo.ptX[ARGPT_DTS] + *srcX1;
			*trgY1 = argInfo.ptY[ARGPT_DTS] + *srcY1;
		}else{
			while(true){
				fprintf(stderr, "マスク画像 (%s) の左上端は合成元画像 (%s) 上のどの点ですか? (x, y):\n", argInfo.fileArgv[FILEARGV_MSK], argInfo.fileArgv[FILEARGV_SRC]);
				fputs("x = ", stderr);
				*srcX1 = inputIntFromFILE(stdin);
				fputs("y = ", stderr);
				*srcY1 = inputIntFromFILE(stdin);
				fprintf(stderr, "マスク画像上のマスク領域を囲む長方形は、マスク画像上の点 (%d,%d)~(%d,%d) であると判断しました。\n", regionMsk->x1, regionMsk->y1, regionMsk->x1 + regionMsk->getWidth() - 1, regionMsk->y1 + regionMsk->getHeight() - 1);
				*srcX1 += regionMsk->x1;
				*srcY1 += regionMsk->y1;
				srcX2 = *srcX1 + regionMsk->getWidth() - 1;
				srcY2 = *srcY1 + regionMsk->getHeight() - 1;
				fprintf(stderr, "よって、合成元画像上での合成領域を囲む長方形は、合成元画像上の点 (%d,%d)~(%d,%d) となります。\n", *srcX1, *srcY1, srcX2, srcY2);

				fprintf(stderr, "合成対象画像 (%s) 上の点 (xt, yt) と合成元画像上の点 (xs, ys) のズレ (Δx, Δy) = (xt - xs, yt - ys) :\n", argInfo.fileArgv[FILEARGV_TRG]);
				fputs("Δx = ", stderr);
				*trgX1 = inputIntFromFILE(stdin);
				fputs("Δy = ", stderr);
				*trgY1 = inputIntFromFILE(stdin);
				*trgX1 += *srcX1; trgX2 = *trgX1 + static_cast<int>(regionMsk->getWidth()) - 1;
				*trgY1 += *srcY1; trgY2 = *trgY1 + static_cast<int>(regionMsk->getHeight()) - 1;
				fprintf(stderr, "よって、対象画像上のマスク領域を囲む長方形は、対象画像上の点 (%d,%d)~(%d,%d) となります。\n", *trgX1, *trgY1, trgX2, trgY2);

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

int pIE_Main(const ArgInfo &argInfo){
	Image srcImg, trgImg;
	ImageSig srcLap;
	BitMask regionMsk;
	int srcX1, srcY1, trgX1, trgY1;
	std::string outputFileName;

	/* 引数判定・合成領域指定 */
	if(setRegionMask_BeforeCompose(&regionMsk, argInfo, &srcX1, &srcY1, &trgX1, &trgY1)){
		return 1;
	}
	const int srcX2 = srcX1 + regionMsk.getWidth() - 1, srcY2 = srcY1 + regionMsk.getHeight() - 1,
		trgX2 = trgX1 + regionMsk.getWidth() - 1, tgY2 = trgY1 + regionMsk.getHeight() - 1;

	/* BMP の読込 */
	if(srcImg.read_Bmp(argInfo.fileArgv[FILEARGV_SRC].c_str())){
		fprintf(stderr, "ERROR: Cannot read the file %s.\n", argInfo.fileArgv[FILEARGV_SRC]);
		return 1;
	}
	if(argInfo.flag[ARGFLAG_GS]){
		if(srcImg.makeIntoGrayscale()){
			return 1;
		}
	}
	if(srcX1 < 0 || srcY1 < 0 || srcX2 >= static_cast<int>(srcImg.getWidth()) || srcY2 >= static_cast<int>(srcImg.getHeight())){
		fputs("ERROR: 指定した領域が画像の範囲外です。\n", stderr);
		return 1;
	}
	if(trgImg.read_Bmp(argInfo.fileArgv[FILEARGV_TRG].c_str())){
		fprintf(stderr, "ERROR: Cannot read the file %s.\n", argInfo.fileArgv[FILEARGV_TRG]);
		return 1;
	}
	if(trgX1 < 0 || trgY1 < 0 || trgX2 >= static_cast<int>(trgImg.getWidth()) || tgY2 >= static_cast<int>(trgImg.getHeight())){
		fputs("ERROR: 指定した領域が画像の範囲外です。\n", stderr);
		return 1;
	}

	/* Filter 前処理 */
	if(srcLap.create_Image(srcX2 - srcX1 + 1, srcY2 - srcY1 + 1)){
		return 1;
	}
	if(!argInfo.flag[ARGFLAG_MIX]){
		if(srcLap.lapFilter(&srcImg, srcX1, srcY1) != 0){
			return 1;
		}
	}else{	/* 元画像と対象画像の Filter の混合 */
		if(srcLap.selectStrongerGradientAndMix(&srcImg, srcX1, srcY1, &trgImg, trgX1, trgY1)){
			return 1;
		}
	}
	if(argInfo.flag[ARGFLAG_AMP]){	/* 増幅 */
		if(srcLap.amplifyFilter(argInfo.ampN)){
			return 1;
		}
	}
	/* 画像処理 */
	if(solvePoisson(&trgImg, &srcLap, trgX1, trgY1, &regionMsk) != 0){
		return 1;
	}

	/* trgImg を BMP に出力 */
	if(argInfo.flag[ARGFLAG_OUTIMG]){
		outputFileName = argInfo.fileArgv[FILEARGV_OUT];
	}else{
		outputFileName = argInfo.fileArgv[FILEARGV_TRG];
		outputFileName += "_pie.bmp";
	}
	if(trgImg.write_Bmp(outputFileName.c_str())){
		fprintf(stderr, "ERROR: Cannot write the file %s.\n", outputFileName.c_str());
		return 1;
	}

	return 0;
}

int stCm_Main(const ArgInfo &argInfo){
	Image srcImg, trgImg;
	BitMask regionMsk;
	int srcX1, srcY1, trgX1, trgY1;
	std::string outputFileName;

	/* 引数判定・合成領域指定 */
	if((setRegionMask_BeforeCompose(&regionMsk, argInfo, &srcX1, &srcY1, &trgX1, &trgY1))){
		return 1;
	}
	const int srcX2 = srcX1 + regionMsk.getWidth() - 1, srcY2 = srcY1 + regionMsk.getHeight() - 1,
		trgX2 = trgX1 + regionMsk.getWidth() - 1, trgY2 = trgY1 + regionMsk.getHeight() - 1;

	/* BMP の読込 */
	if(srcImg.read_Bmp(argInfo.fileArgv[FILEARGV_SRC].c_str())){
		fprintf(stderr, "ERROR: Cannot read the file %s.\n", argInfo.fileArgv[FILEARGV_SRC]);
		return 1;
	}
	if(srcX1 < 0 || srcY1 < 0 || srcX2 >= static_cast<int>(srcImg.getWidth()) || srcY2 >= static_cast<int>(srcImg.getHeight())){
		fputs("ERROR: 指定した領域が画像の範囲外です。\n", stderr);
		return 1;
	}
	if(trgImg.read_Bmp(argInfo.fileArgv[FILEARGV_TRG].c_str())){
		fprintf(stderr, "ERROR: Cannot read the file %s.\n", argInfo.fileArgv[FILEARGV_TRG]);
		return 1;
	}
	if(trgX1 < 0 || trgY1 < 0 || trgX2 >= static_cast<int>(trgImg.getWidth()) || trgY2 >= static_cast<int>(trgImg.getHeight())){
		fputs("ERROR: 指定した領域が画像の範囲外です。\n", stderr);
		return 1;
	}

	/* 画像処理 */
	if(straightCompose(&trgImg, trgX1, trgY1, &srcImg, srcX1, srcY1, &regionMsk) != 0){
		return 1;
	}

	/* trgImg を BMP に出力 */
	if(argInfo.flag[ARGFLAG_OUTIMG]){
		outputFileName = argInfo.fileArgv[FILEARGV_OUT];
	}else{
		outputFileName = argInfo.fileArgv[FILEARGV_TRG];
		outputFileName += "_stcm.bmp";
	}
	if(trgImg.write_Bmp(outputFileName.c_str())){
		fprintf(stderr, "ERROR: Cannot write the file %s.\n", outputFileName.c_str());
		return 1;
	}

	return 0;
}

int bmp2bmp_lap(const ArgInfo &argInfo){
	/* 変数宣言 */
	Image colorImg;	/* 画像 */
	ImageSig lapImg;	/* Laplacian */
	std::string outputFileName;	/* 出力 file 名 */
	fputs("BMP2BMP (Laplacian Image)\n", stderr);

	/* 画像 file を ColorImg に入力 */
	if(colorImg.read_Bmp(argInfo.fileArgv[FILEARGV_SRC].c_str())){
		fprintf(stderr, "ERROR: Cannot read the file %s.\n", argInfo.fileArgv[FILEARGV_SRC]);
		return 1;
	}
	/* BMP と同じ size の符号付画像を作成 */
	if(lapImg.create_Image(colorImg.getWidth(), colorImg.getHeight())){
		fputs("ERROR\n", stderr);
		return 1;
	}

	/* 画像処理 */
	if((lapImg.lapFilter(&colorImg, 0, 0)) != 0){
		return 1;
	}
	for(int y = 0; y < static_cast<int>(colorImg.getHeight()); ++y){
		for(int x = 0; x < static_cast<int>(colorImg.getWidth()); ++x){
			for(int i = 0; i < 3; ++i){
				colorImg.setData_NoSecure(x, y, i, std::min(static_cast<int>(std::abs(lapImg.getData_NoSecure(x, y, i))), 255));
			}
		}
	}

	/* ColorImg を画像 file に出力 */
	if(argInfo.flag[ARGFLAG_OUTIMG]){
		outputFileName = argInfo.fileArgv[FILEARGV_OUT];
	}else{
		outputFileName = argInfo.fileArgv[FILEARGV_SRC];
		outputFileName += "_lap.bmp";
	}
	if(colorImg.write_Bmp(outputFileName.c_str())){
		fprintf(stderr, "ERROR: Cannot write the file %s.\n", outputFileName.c_str());
		return 1;
	}

	return 0;
}

int bmp2bmp_gs(const ArgInfo &argInfo){
	/* 変数宣言 */
	Image colorImg;	/* 画像 */
	std::string outputFileName;	/* 出力 file 名 */
	fputs("BMP2BMP (Grayscale Image)\n", stderr);

	/* 画像 file を ColorImg に入力 */
	if(colorImg.read_Bmp(argInfo.fileArgv[FILEARGV_SRC].c_str())){
		fprintf(stderr, "ERROR: Cannot read the file %s.\n", argInfo.fileArgv[FILEARGV_SRC].c_str());
		return 1;
	}

	/* 画像処理 */
	if(colorImg.makeIntoGrayscale()){
		return 1;
	}

	/* ColorImg を画像 file に出力 */
	if(argInfo.flag[ARGFLAG_OUTIMG]){
		outputFileName = argInfo.fileArgv[FILEARGV_OUT];
	}else{
		outputFileName = argInfo.fileArgv[FILEARGV_SRC];
		outputFileName += "_gs.bmp";
	}
	if(colorImg.write_Bmp(outputFileName.c_str())){
		fprintf(stderr, "ERROR: Cannot write the file %s.\n", outputFileName.c_str());
		return 1;
	}

	return 0;
}
