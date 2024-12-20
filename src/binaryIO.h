#ifndef BINARYIO
#define BINARYIO
//------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "common.h"

//------------------------------------------------
//  プロトタイプ宣言(Prototype declaration)
//------------------------------------------------

bool writeBinary(FILE *file, JuInfo *juInfo, Shoupai *shoupai);
bool readBinary(FILE *file, JuInfo *juInfo, Shoupai *shoupai);

//------------------------------------------------
#endif