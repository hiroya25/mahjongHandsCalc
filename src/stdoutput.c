#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "common.h"
#include "hule.h"
#include "stdoutput.h"

/** @brief Sijia型を文字列に変換する関数
 * @param sijia (Sijia) 四家
 * @return (char *) 四家の文字列
 */
char *sijiaToString(Sijia sijia){
    switch(sijia){
        case Zimo:     return "自家";
        case Xiagia:   return "下家";
        case DuiMian:  return "対面";
        case Shangjia: return "上家";
    }
}

/** @brief Mianzi型を文字列に変換する関数
 * @param mianzi (Mianzi *) 面子
 * @return (char *) 面子の文字列
 */
char *mianziToString(Mianzi *mianzi){
    // 初期化
    size_t bufferSize = 256;
    char *str = allocateMemory(bufferSize, 1, false);
    str[0] = '\0';

    bool isZihpai = false;
    char *zihpaiStr[] = {"", "東", "南", "西", "北", "白", "發", "中"};

    switch(mianzi->paizhong){
    case Wanzi:  strcat(str, "m"); break;
    case Tongzi: strcat(str, "p"); break;
    case Suozi:  strcat(str, "s"); break;
    case Zihpai: isZihpai = true;  break;
    }
    
    switch(mianzi->paixing){
        case Shunzi:
            snprintf(str + strlen(str), bufferSize - strlen(str), "%u%u%u", mianzi->value, mianzi->value + 1, mianzi->value + 2);
            break; 

        case Duizi:
        case Kezi:
        case Gangzi:
            uint8_t counter = (mianzi->paixing == Duizi) ? 2 : (mianzi->paixing == Kezi) ? 3 : 4;
            for(int i = 0; i < counter; i++){
                if(isZihpai) strncat(str, zihpaiStr[mianzi->value], bufferSize - strlen(str) - 1);
                else         snprintf(str + strlen(str), bufferSize - strlen(str), "%u", mianzi->value);
            }
            break;
    }
    if(mianzi->isHulu){
        if(isZihpai){
            if(mianzi->source == Zimo) snprintf(str + strlen(str), bufferSize - strlen(str), "(ツモ:%s)"    , zihpaiStr[mianzi->value + mianzi->index]);
            else                       snprintf(str + strlen(str), bufferSize - strlen(str), "(ロン:%s, %s)", zihpaiStr[mianzi->value + mianzi->index], sijiaToString(mianzi->source));
        }else{
            if(mianzi->source == Zimo) snprintf(str + strlen(str), bufferSize - strlen(str), "(ツモ:%u)"    , mianzi->value + mianzi->index);
            else                       snprintf(str + strlen(str), bufferSize - strlen(str), "(ロン:%u, %s)", mianzi->value + mianzi->index, sijiaToString(mianzi->source));
        }

    }else if(mianzi->isFulou){
        char *v[] = {"チー", "ポン", "カン"};
        if(isZihpai){
            snprintf(str + strlen(str), bufferSize - strlen(str), "(%s:%s, %s)", v[mianzi->paixing], zihpaiStr[mianzi->value + mianzi->index], sijiaToString(mianzi->source));
        }else{
            snprintf(str + strlen(str), bufferSize - strlen(str), "(%s:%u, %s)", v[mianzi->paixing], mianzi->value + mianzi->index, sijiaToString(mianzi->source));
        }
    }
    
    return str;
}

/** @brief Shoupai型を文字列に変換する関数
 * @param shoupai (Shoupai *) 手牌
 * @return (char *) 手牌の文字列
 */
char *shoupaiToString(Shoupai *shoupai){
    // 初期化
    size_t bufferSize = 256;
    char *str = allocateMemory(bufferSize, 1, false);
    str[0] = '\0';

    // 和了牌が存在するとき
    if(shoupai->hulepai != NULL){
        // 和了牌を打牌可能な牌から1減らす
        shoupai->bingpai[shoupai->hulepai->paizhong][shoupai->hulepai->value]--;
    }

    // 打牌可能な牌
    // 数牌
    for(int i = 0; i < 3; i++){
        bool display = false;

        for(int j = 1; j <= 9; j++){
            if(shoupai->bingpai[i][j] == 0) continue;

            if(!display){
                switch(i){
                    case 0: strcat(str, "m"); break;
                    case 1: strcat(str, "p"); break;
                    case 2: strcat(str, "s"); break;
                }
                display = true;
            }

            for(int k = 0; k < shoupai->bingpai[i][j]; k++){
                snprintf(str + strlen(str), bufferSize - strlen(str), "%u", j);
            }
        }

        if(i != 2 && display) strcat(str, " ");
    }

    // 字牌
    for(int i = 1; i <= 7; i++){
        for(int j = 0; j < shoupai->bingpai[3][i]; j++){
            switch(i){
                case 1: strcat(str, "東"); break;
                case 2: strcat(str, "南"); break;
                case 3: strcat(str, "西"); break;
                case 4: strcat(str, "北"); break;
                case 5: strcat(str, "白"); break;
                case 6: strcat(str, "發"); break;
                case 7: strcat(str, "中"); break;
            }
        }
    }

    // 副露
    for(int i = 0; shoupai->fulou[i] != NULL; i++){
        // 副露面子を文字列に追加
        strcat(str, " ");
        strcat(str, mianziToString(shoupai->fulou[i]));
    }

    // 和了牌が存在するとき最後に出力し戻す
    if(shoupai->hulepai != NULL){
        strcat(str, " ");

        // 和了牌を出力
        switch(shoupai->hulepai->paizhong){
            case Wanzi:  strcat(str, "m"); break;
            case Tongzi: strcat(str, "p"); break;
            case Suozi:  strcat(str, "s"); break;
            case Zihpai: strcat(str, "z"); break;
        }
        snprintf(str + strlen(str), bufferSize - strlen(str), "%u", shoupai->hulepai->value);
        if(shoupai->hulepai->source == Zimo) strcat(str, "(ツモ)");
        else                                 snprintf(str + strlen(str), bufferSize - strlen(str), "(ロン:%s)", sijiaToString(shoupai->hulepai->source));

        // 和了牌を打牌可能な牌に戻す
        shoupai->bingpai[shoupai->hulepai->paizhong][shoupai->hulepai->value]++;
    }

    return str;
}

/** @brief Hule型を文字列に変換する関数
 * @param hule (Hule *) 和了情報
 * @return (char *) 和了情報の文字列
 */
char *huleToString(Hule *hule){
    // 初期化
    size_t bufferSize = 256;
    char *str = allocateMemory(bufferSize, 1, false);
    str[0] = '\0';

    // 役の追加
    int i = 0;
    while(hule->hupai[i] != NULL){
        if(hule->damanguan == 0) snprintf(str + strlen(str), bufferSize - strlen(str), "%s, %d\n", hule->hupai[i]->name, hule->hupai[i]->fanshu);
        else                     snprintf(str + strlen(str), bufferSize - strlen(str), "%s\n", hule->hupai[i]->name);
        i++;
    }

    // 改行
    strcat(str, "\n");

    // 情報の追加
    if(hule->damanguan != 0){
        // 数字を漢数字に変換
        char *numStr[] = {"", "二", "三", "四"};
        snprintf(str + strlen(str), bufferSize - strlen(str), "%s役満\n", numStr[hule->damanguan - 1]);
    }else{
        snprintf(str + strlen(str), bufferSize - strlen(str), "%u翻(%u符)\n\n", hule->fanshu, hule->fu);

        if     (hule->fanshu >= 13) strcat(str, "数え役満 "); // 数え役満
        else if(hule->fanshu >= 11) strcat(str, "三倍満 ");   // 三倍満
        else if(hule->fanshu >= 8)  strcat(str, "倍満 ");     // 倍満
        else if(hule->fanshu >= 6)  strcat(str, "跳満 ");     // 跳満
        else if(hule->defen >= 2000)  strcat(str, "満貫 ");   // 満貫

        snprintf(str + strlen(str), bufferSize - strlen(str), "%u点\n", hule->defen);
    }

    snprintf(str + strlen(str), bufferSize - strlen(str), "点数変動[東, 南, 西, 北] : %d, %d, %d, %d\n", hule->fenpei[0], hule->fenpei[1], hule->fenpei[2], hule->fenpei[3]);

    return str;
}
