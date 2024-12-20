#ifndef FILEINPUT
#define FILEINPUT
//------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "common.h"
#include "hule.h"

//------------------------------------------------
//  プロトタイプ宣言(Prototype declaration)
//------------------------------------------------

bool fileInput(FILE *file, JuInfo *juInfo, Shoupai *shoupai);
bool lineinput(FILE *file, void *ptr, uint8_t type);

//------------------------------------------------
#endif