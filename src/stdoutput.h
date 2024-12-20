#ifndef STDOUTPUT
#define STDOUTPUT
//------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "common.h"

//------------------------------------------------
//  プロトタイプ宣言(Prototype declaration)
//------------------------------------------------

char *sijiaToString(Sijia sijia);
char *mianziToString(Mianzi *mianzi);
char *shoupaiToString(Shoupai *shoupai);
char *huleToString(Hule *hule);

//------------------------------------------------
#endif