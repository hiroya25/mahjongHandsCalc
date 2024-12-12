#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "common.h"
#include "mianzi.h"
#include "hule.h"

char *sijiaToString(Sijia sijia){
    switch(sijia){
        case Zimo:     return "自家";
        case Xiagia:   return "下家";
        case DuiMian:  return "対面";
        case Shangjia: return "上家";
    }
}

char *mianziToString(Mianzi *mianzi){
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

int main(){
    // --- 手牌の初期化 --- //
    Mianzi **fulou = (Mianzi **)malloc(1 * sizeof(Mianzi *));
    fulou[0] = NULL;
    /*fulou[0] = allocateMemory(1, sizeof(Mianzi), false);

    fulou[0]->paixing = Kezi;
    fulou[0]->paizhong = Zihpai;
    fulou[0]->value = 6;
    fulou[0]->isFulou = true;
    fulou[0]->isHulu = false;
    fulou[0]->index = 0;
    fulou[0]->source = DuiMian;
    fulou[1] = NULL;*/

    Pai hulepai = { Suozi, 8, Xiagia };
    bool fulouHongpai[1] = {false};

    Shoupai shoupai = {
        {{1, 0, 0, 0, 1, 0, 1, 0, 0, 0},  // 萬子
         {0, 0, 0, 0, 0, 1, 1, 1, 0, 0},  // 筒子
         {0, 0, 0, 2, 1, 1, 1, 1, 1, 1},  // 索子
         {0, 0, 0, 0, 0, 0, 0, 0}},       // 字牌
        fulou,
        fulouHongpai,
        &hulepai
    };

    Hupai hupai = { 1, false, false, false, 0, 0 };
    JuInfo juInfo = {Dong, Xi, &hupai, {35, 0, 0, 0, 0}, {12, 0, 0, 0, 0}, 0, 2};

    Hule *huleInfo = hule(&shoupai, &juInfo);
    // --- 手牌の初期化 --- //

    // 手牌の表現
    for(int i = 0; i < 3; i++){
        bool hav = false;
        for(int j = 0; j < 10; j++){
            if(shoupai.bingpai[i][j] > 0){
                if(!hav) printf("%s", i == 0 ? "m": i == 1 ? "p" : "s");
                for(int k = 0; k < shoupai.bingpai[i][j]; k++) printf("%d", j);
                hav = true;
            }
        }
    }

    for(int i = 0; i < shoupai.bingpai[Zihpai][1]; i++) printf("東");
    for(int i = 0; i < shoupai.bingpai[Zihpai][2]; i++) printf("南");
    for(int i = 0; i < shoupai.bingpai[Zihpai][3]; i++) printf("西");
    for(int i = 0; i < shoupai.bingpai[Zihpai][4]; i++) printf("北");
    for(int i = 0; i < shoupai.bingpai[Zihpai][5]; i++) printf("白");
    for(int i = 0; i < shoupai.bingpai[Zihpai][6]; i++) printf("發");
    for(int i = 0; i < shoupai.bingpai[Zihpai][7]; i++) printf("中");
    
    for(int i = 0; shoupai.fulou[i] != NULL; i++){
        printf(" ");
        printf("%s", mianziToString(shoupai.fulou[i]));
    }

    printf("\n");

    int i = 0;
    while(huleInfo->hupai[i] != NULL){
        printf("%s, %d\n", huleInfo->hupai[i]->name, huleInfo->hupai[i]->fanshu);
        i++;
    }
    printf("\n");

    printf("fu : %u\n", huleInfo->fu);
    printf("fanshu : %u\n", huleInfo->fanshu);
    printf("damanguan : %u\n", huleInfo->damanguan);
    printf("defen : %u\n", huleInfo->defen);
    printf("fenpei[東, 西, 南, 北] : %d, %d, %d, %d\n", huleInfo->fenpei[0], huleInfo->fenpei[1], huleInfo->fenpei[2], huleInfo->fenpei[3]);

    /*
    TeshuDamanguan flag;
    Mianzi ***mianzi = huleMianzi(&flag, &shoupai, &hulepai);
    size_t count = 0;
    while(mianzi[count] != NULL) count++;
    
    Hudi **hudi = allocateMemory(count, sizeof(Hudi *), false);
    Yi ***result = allocateMemory(count, sizeof(Yi **), false);

    for(int i = 0; i < count; i++){
        hudi[i] = getHudi(mianzi[i], 0, 0);
        result[i] = getHupai(flag, mianzi[i], hudi[i], getPreHupai(&hupai));
    }

    // 結果を表示
    if(result != NULL){
        for(int i = 0; i < count; i++){
            uint8_t mianziCount = 0;
            while(mianzi[i][mianziCount] != NULL) mianziCount++;

            printf("面子 : ");
            for(int j = 0; j < mianziCount; j++){
                switch(mianzi[i][j]->paizhong){
                    case Wanzi:
                        printf("m");
                        break; 

                    case Tongzi:
                        printf("p");
                        break; 

                    case Suozi:
                        printf("s");
                        break; 

                    case Zihpai:
                        printf("z");
                        break; 
                }

                switch(mianzi[i][j]->paixing){
                    case Shunzi:
                        printf("%d%d%d", mianzi[i][j]->value, mianzi[i][j]->value + 1, mianzi[i][j]->value + 2);
                        break; 

                    case Kezi:
                        printf("%d%d%d", mianzi[i][j]->value, mianzi[i][j]->value, mianzi[i][j]->value);
                        break; 

                    case Gangzi:
                        printf("%d%d%d%d", mianzi[i][j]->value, mianzi[i][j]->value, mianzi[i][j]->value, mianzi[i][j]->value);
                        break; 

                    case Duizi:
                        printf("%d%d", mianzi[i][j]->value, mianzi[i][j]->value);
                        break; 
                }
                if(mianzi[i][j]->isHulu){
                    printf("(Agari : %d, %d)", mianzi[i][j]->value + mianzi[i][j]->index, mianzi[i][j]->source);
                }
                if(mianzi[i][j]->isFulou){
                    printf("(fulou : %d, %d)", mianzi[i][j]->value + mianzi[i][j]->index, mianzi[i][j]->source);
                }
                if(j != mianziCount - 1) printf(", ");
                else printf("\n");
            }

            uint8_t j = 0;
            while(result[i][j] != NULL){
                printf("%s, %d\n", result[i][j]->name, (int)result[i][j]->fanshu);
                j++;
            }
            printf("\n");
        }

        // メモリ解放
        for (int j = 0; j < count; j++) {
            free(result[j]);
        }
        free(result);
    }else{
        puts("nun");
    }
    */

    return 0;
}