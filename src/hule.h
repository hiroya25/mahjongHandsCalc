#ifndef HULE
#define HULE
//------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "common.h"
#include "mianzi.h"

//------------------------------------------------
//  マクロ定義(Macro definition)
//------------------------------------------------

/** @brief 役の数の最大数 */
#define MAX_HUPAI (10)

//------------------------------------------------
//  型定義(Type definition)
//------------------------------------------------

//--- 列挙体定義 ---//

/**
 * @enum Fengpai
 * @brief 風(場風・自風)を表す列挙体
 */
typedef enum{
    Dong,  /*!< 東 */
    Nan,   /*!< 南 */
    Xi,    /*!< 西 */
    Bei    /*!< 北 */
} Fengpai;

//--- 構造体定義 ---//

/**
 * @struct Yi
 * @brief 役と翻数を表す構造体
 */
typedef struct{
    char *name;         /*!< 役の名称 */
    int8_t fanshu;      /*!< 翻数(役満の時-1, ダブル役満の時-2) */
    Sijia baojia;       /*!< パオ(0(=Zimo(自分)のとき、パオなし)) */
} Yi;

/**
 * @struct Hupai
 * @brief 特殊役の情報
 */
typedef struct{
    uint8_t lizhi : 2;  /*!< 1: 立直, 2: ダブル立直 */
    bool yifa;          /*!< 一発かどうか */
    bool qianggang;     /*!< 槍槓かどうか */
    bool lingshang;     /*!< 嶺上開花かどうか */
    uint8_t haidi : 2;  /*!< 1: 海底摸月, 2: 河底撈魚 */
    uint8_t tianhu : 2; /*!< 1: 天和, 2: 地和 */
} Hupai;

/**
 * @struct JuInfo
 * @brief 局の情報
 */
typedef struct{
    Fengpai zhuangfeng;     /*!< 場風 */
    Fengpai menfeng;        /*!< 和了者の自風 */
    Hupai *hupai;           /*!< 特殊役情報 */
    uint8_t baopai[5];      /*!< ドラ表示牌 (牌の種類 : val/10, 牌の番号 : val%10) */
    uint8_t fubaopai[5];    /*!< 裏ドラ表示牌 (牌の種類 : val/10, 牌の番号 : val%10) */
    uint8_t changbang;      /*!< 積み棒の数 */
    uint8_t lizhibang;      /*!< 供託立直棒の数 */
} JuInfo;

/**
 * @struct Hudi
 * @brief 符の情報を表す構造体
 */
typedef struct{
    uint8_t fu;             /*!< 合計符数 */
    bool menqian;           /*!< 門前 */
    bool zimo;              /*!< ツモ和了 */
    Mianzi **shunzi;        /*!< 順子の面子構成 */
    uint8_t kezi[4][10];    /*!< 刻子の面子構成 */
    uint8_t nShunzi;        /*!< 順子の数 */
    uint8_t nKezi;          /*!< 刻子の数(槓子含む) */
    uint8_t nAnkezi;        /*!< 暗刻子の数(暗槓子を含む) */
    uint8_t nGangzi;        /*!< 槓子の数 */
    uint8_t nZiphai;         /*!< 字牌面子の数(雀頭含む) */
    uint8_t nYaojiu;        /*!< 幺九牌入り面子の数(雀頭を含む) */
    bool danqi;             /*!< 単騎待ち */
    bool pinghu;            /*!< 平和形 */
    Fengpai zhuangfeng;     /*!< 場風 */
    Fengpai menfeng;        /*!< 自風 */
} Hudi;

/**
 * @struct Hule
 * @brief 面子や役、符などの計算結果をまとめた構造体(出力)
 */
typedef struct{
    Yi **hupai;         /*!< 役一覧 */
    uint8_t fu;         /*!< 符 */
    uint8_t fanshu;     /*!< 翻数 */
    uint8_t damanguan;  /*!< 役満複合数 */
    uint32_t defen;     /*!< (供託を含めない)和了点 */
    int32_t fenpei[4];  /*!< (供託を含めた)点数の移動 */
} Hule;

//------------------------------------------------
//  プロトタイプ宣言(Prototype declaration)
//------------------------------------------------

Hule *hule(bool *huleHand, Shoupai *shoupai, JuInfo *juInfo);

//------------------------------------------------
#endif