/* mystd.cpp */
#include "mystdio.h"
#include <cstring>
#include <cstdlib>

char *myFgets(char *s, int n, FILE *fp){
	char *rtn;
	if((rtn = fgets(s, n, fp)) == NULL) { return NULL; }
	if(s[(n = ((int)strnlen(s, n) - 1))] == '\n') {
		s[n] = '\0';
	}else{
		fflush(fp);
	}
	return rtn;
}

int inputIntFromFILE(FILE *fp){
	int val;
	char buf[32];
	myFgets(buf, 32, fp);
	val = atoi(buf);
	return val;
}
