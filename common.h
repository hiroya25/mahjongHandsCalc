#ifndef COMMON
#define COMMON
//------------------------------------------------
#include <stdint.h>

//------------------------------------------------
//  型定義(Type definition)
//------------------------------------------------

//--- 列挙体定義 ---//
/**
 * @enum Paizhong
 * @brief 牌の種類を表す列挙体
 */
typedef enum{
    Wanzi,  /*!< 萬子 */
    Tongzi, /*!< 筒子 */
    Suozi,  /*!< 索子 */
    Zihpai  /*!< 字牌 */
} Paizhong;

/**
 * @enum Sijia
 * @brief 自分から見た四家を表す列挙体
 */
typedef enum{
    Zimo,       /*!< 自家 */
    Shangjia,   /*!< 上家 */
    DuiMian,    /*!< 対面 */
    Xiagia      /*!< 下家 */
} Sijia;

/**
 * @enum Paixing
 * @brief 面子の種類を表す列挙体
 */
typedef enum{
    Shunzi,     /*!< 順子 */
    Kezi,       /*!< 刻子 */
    Gangzi,     /*!< 槓子 */
    Duizi       /*!< 対子 */
} Paixing;

//--- 構造体定義 ---//
/**
 * @struct Pai
 * @brief 牌の情報(8bit)
 */
typedef struct{
    Paizhong paizhong : 2;      /*!< 牌の種類 */
    uint8_t value : 4;          /*!< 数字(0~9, 字牌は1~7に対応) */
    Sijia paiSource : 2;        /*!< 牌の出所 */
} Pai;

/**
 * @struct Mianzi
 * @brief 面子を表す構造体
 */
typedef struct{
    Paixing paixing : 2;    /*!< 面子の種類 */
    Paizhong paizhong : 2;  /*!< 牌の種類 */
    uint8_t value : 4;      /*!< 数字(0~9, 順子のときは開始位置、字牌は1~7に対応) */

    bool isFulou;           /*!< 副露かどうか */
    bool isHulu;            /*!< 和了牌かどうか */
    // 副露, 和了牌のとき
    uint8_t index : 2;      /*!< どの牌か、valueからのオフセット */
    Sijia source : 2;       /*!< 誰からの牌か */
} Mianzi;

/**
 * @struct Shoupai
 * @brief 手牌を表す構造体(入力)
 */
typedef struct{
    uint8_t bingpai[4][10]; /*!< 打牌可能な手牌(bingpai[Paizhong][value]で表す, value=0は赤ドラ) */
    Mianzi **fulou;         /*!< 副露した面子 */
    bool *fulouHongpai;     /*!< 副露した面子に赤ドラが含まれるかどうか */
    Pai *drawnPai;          /*!< 直前に手牌に来た牌 */
} Shoupai;


//------------------------------------------------
//  プロトタイプ宣言(Prototype declaration)
//------------------------------------------------

void *allocateMemory(size_t size, size_t typeSize, bool clearFlag);

//------------------------------------------------
#endif