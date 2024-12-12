#ifndef MIANZI
#define MIANZI
//------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "common.h"

//------------------------------------------------
//  マクロ定義(Macro definition)
//------------------------------------------------

/** @brief 14枚の牌から4面子と1雀頭を作る組み合わせ数の最大値 */
#define MAX_MIANZI_COMBINATIONS (10)

//------------------------------------------------
//  型定義(Type definition)
//------------------------------------------------

//--- 列挙体定義 ---//
/**
 * @enum 
 * @brief 特殊な形の役満
 */
typedef enum{
    None,               /*!< なし */
    Guoshi,             /*!< 国士無双 */
    GuoshiShiSanMin,    /*!< 国士無双十三面 */
    Jiulian,            /*!< 九蓮宝燈 */
    ChunzhengJiulian    /*!< 純正九蓮宝燈 */
} TeshuDamanguan;


//------------------------------------------------
//  プロトタイプ宣言(Prototype declaration)
//------------------------------------------------

Mianzi ***huleMianzi(TeshuDamanguan *flag, Shoupai *shoupai);
void freeAllMianziArray(Mianzi ***mianzi);

//------------------------------------------------
#endif