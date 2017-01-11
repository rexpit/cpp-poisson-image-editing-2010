/* mystdio.h */
#ifndef _MYSTDIO_H_
#define _MYSTDIO_H_

#include <cstdio>

/* char *myFgets(char *s, int n, FILE *fp)
機能:
	fgets(s, n, fp)→fpのバッファを初期化
戻り値:
	fgetsの戻値
引数:
	char *s:代入先の文字列配列
	int n:最大文字数(sizeof(s)を入れるといい)
	FILE *fp:入力File pointer
*/
char *myFgets(char *s, int n, FILE *fp);

int inputIntFromFILE(FILE *fp);

#endif /* _MYSTD_H_ */
