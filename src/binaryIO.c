#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "common.h"
#include "hule.h"
#include "binaryIO.h"


/** @brief バイナリファイルに書き込む関数
 * @param file (FILE *) ファイルポインタ
 * @param juInfo (JuInfo *) 局情報
 * @param shoupai (Shoupai *) 手牌
 * @return (bool) 書き込みに成功したかどうか
 */
bool writeBinary(FILE *file, JuInfo *juInfo, Shoupai *shoupai){
    // --- 局情報の書き込み(ポインタ込み) --- //
    if(fwrite(juInfo, sizeof(JuInfo), 1, file) != 1){
        fprintf(stderr, "書き込みに失敗しました\n");
        return false;
    }
    if(fwrite(juInfo->hupai, sizeof(Hupai), 1, file) != 1){
        fprintf(stderr, "書き込みに失敗しました\n");
        return false;
    }

    // --- 手牌の書き込み(ポインタ込み) --- //
    if(fwrite(shoupai, sizeof(Shoupai), 1, file) != 1){
        fprintf(stderr, "書き込みに失敗しました\n");
        return false;
    }

    // --- 副露の書き込み --- //
    for(int i = 0; i < 5; i++){
        bool isNull = (shoupai->fulou[i] == NULL);
        if(fwrite(&isNull, sizeof(bool), 1, file) != 1){
            fprintf(stderr, "書き込みに失敗しました\n");
            return false;
        }
        if(!isNull){
            if(fwrite(shoupai->fulou[i], sizeof(Mianzi), 1, file) != 1){
                fprintf(stderr, "書き込みに失敗しました\n");
                return false;
            }
        }else{
            break;
        }
    }
    if(fwrite(shoupai->fulouHongpai, sizeof(bool), 5, file) != 5){
        fprintf(stderr, "書き込みに失敗しました\n");
        return false;
    }
    if(fwrite(shoupai->hulepai, sizeof(Pai), 1, file) != 1){
        fprintf(stderr, "書き込みに失敗しました\n");
        return false;
    }
    return true;
}

/** @brief バイナリファイルを読み込む関数
 * @param file (FILE *) ファイルポインタ
 * @param juInfo (JuInfo *) 局情報
 * @param shoupai (Shoupai *) 手牌
 * @return (bool) 読み込みに成功したかどうか
 */
bool readBinary(FILE *file, JuInfo *juInfo, Shoupai *shoupai){
    // --- 局情報の読み込み(ポインタ込み) --- //
    if(fread(juInfo, sizeof(JuInfo), 1, file) != 1){
        fprintf(stderr, "読み込みに失敗しました\n");
        return false;
    }
    juInfo->hupai = allocateMemory(1, sizeof(Hupai), false);
    if(fread(juInfo->hupai, sizeof(Hupai), 1, file) != 1){
        fprintf(stderr, "読み込みに失敗しました\n");
        return false;
    }

    // --- 手牌の読み込み(ポインタ込み) --- //
    if(fread(shoupai, sizeof(Shoupai), 1, file) != 1){
        fprintf(stderr, "読み込みに失敗しました\n");
        return false;
    }
    shoupai->fulou = allocateMemory(5, sizeof(Mianzi *), true);
    for(int i = 0; i < 5; i++){
        bool isNull;
        if(fread(&isNull, sizeof(bool), 1, file) != 1){
            fprintf(stderr, "読み込みに失敗しました\n");
            return false;
        }
        if(!isNull){
            shoupai->fulou[i] = allocateMemory(1, sizeof(Mianzi), false);
            if(fread(shoupai->fulou[i], sizeof(Mianzi), 1, file) != 1){
                fprintf(stderr, "読み込みに失敗しました\n");
                return false;
            }
        }else{
            break;
        }
    }
    
    shoupai->fulouHongpai = allocateMemory(5, sizeof(bool), true);
    if(fread(shoupai->fulouHongpai, sizeof(bool), 5, file) != 5){
        fprintf(stderr, "読み込みに失敗しました\n");
        return false;
    }
    shoupai->hulepai = allocateMemory(1, sizeof(Pai), false);
    if(fread(shoupai->hulepai, sizeof(Pai), 1, file) != 1){
        fprintf(stderr, "読み込みに失敗しました\n");
        return false;
    }
    return true;
}