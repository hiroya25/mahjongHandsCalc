#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "common.h"
#include "mianzi.h"
#include "hule.h"
#include "fileinput.h"
#include "stdoutput.h"
#include "binaryIO.h"

int main(void){
    printf("麻雀点数計算プログラム\n");

    bool loop = false;
    do{
        // 変数の宣言
        JuInfo *juInfo = allocateMemory(1, sizeof(JuInfo), false);
        Shoupai *shoupai = allocateMemory(1, sizeof(Shoupai), false);

        loop = false;

        uint8_t type = 0;
        printf("入力方法を選択してください 0...手入力 1...テキストファイル入力 2...バイナリファイル入力\n");
        if(!lineinput(stdin, &type, 0)) { loop = true; continue; }
        if(type == 0){
            // 手入力で手牌入力
            if(!fileInput(stdin, juInfo, shoupai)){
                printf("入力エラー\n");
                free(juInfo);
                free(shoupai);
                continue;
            }
        }else if(type == 1){
            // ファイルによる入力
            printf("ファイル名 : ");
            char filename[256];
            if(!lineinput(stdin, filename, 1)){
                printf("入力エラー\n");
                loop = true;
                free(juInfo);
                free(shoupai);
                continue;
            }

            FILE *file = fopen(filename, "r");
            if(file == NULL){
                printf("ファイルが開けません\n");
                loop = true;
                free(juInfo);
                free(shoupai);
                continue;
            }

            if(!fileInput(file, juInfo, shoupai)){
                printf("入力エラー\n");
                loop = true;
                free(juInfo);
                free(shoupai);
                continue;
            }
        }else if(type == 2){
            // バイナリファイルによる入力
            printf("ファイル名 : ");
            char filename[256];
            if(!lineinput(stdin, filename, 1)){
                printf("入力エラー\n");
                loop = true;
                free(juInfo);
                free(shoupai);
                continue;
            }

            FILE *file = fopen(filename, "rb");
            if(file == NULL){
                printf("ファイルが開けません\n");
                loop = true;
                free(juInfo);
                free(shoupai);
                continue;
            }

            if(!readBinary(file, juInfo, shoupai)){
                printf("入力エラー\n");
                loop = true;
                free(juInfo);
                free(shoupai);
                continue;
            }
        }else{
            printf("無効な入力です\n");
            loop = true;
            continue;
        }

        // 和了判定
        bool huleFlag = false;
        Hule *huleInfo = hule(&huleFlag, shoupai, juInfo);

        if(huleInfo == NULL || huleInfo->hupai == NULL){
            if(huleFlag) printf("\n役が存在しません\n");
            else         printf("\n上がりの形をしていません\n");

            free(juInfo);
            free(shoupai);
            return 0;
        }
        
        // 結果の表示
        printf("\n----------------------------------------\n");
        printf("手牌 : %s\n", shoupaiToString(shoupai));
        printf("\n");
        printf("%s\n\n", huleToString(huleInfo));


        uint8_t numinput;
        bool inputLoop;
        do{
            if(type != 2)printf("\n操作を入力してください 0...終了, 1...続行, 2...保存\n");
            else printf("操作を入力してください 0...終了, 1...続行\n");

            inputLoop = false;
            if(!lineinput(stdin, &numinput, 0)) { inputLoop = true; continue; }

            switch(numinput){
                case 0:
                    // 終了
                    loop = false;
                    break;

                case 1:
                    // 続行
                    loop = true;
                    break;

                case 2:
                    // 保存
                    if(type != 2){
                        // 保存
                        printf("ファイル名 : ");
                        char filename[256];
                        if(!lineinput(stdin, filename, 1)){
                            printf("入力エラー\n");
                            inputLoop = true;
                            continue;
                        }

                        FILE *file = fopen(filename, "wb");
                        if(file == NULL){
                            printf("ファイルが開けません\n");
                            inputLoop = true;
                            continue;
                        }

                        if(!writeBinary(file, juInfo, shoupai)){
                            inputLoop = true;
                            continue;
                        }else{
                            printf("保存しました\n");
                            type = 3;
                        }
                        inputLoop = true;
                    }else{
                        // 無効
                        printf("無効な入力です\n");
                        inputLoop = true;
                    }
                    break;

                default:
                    printf("無効な入力です\n");
                    inputLoop = true;
                    break;
            }
        }while(inputLoop);

        // メモリの解放
        for(int i = 0; huleInfo->hupai[i] != NULL; i++){
            free(huleInfo->hupai[i]);
        }  
        free(huleInfo->hupai);
        free(huleInfo);
        free(juInfo->hupai);
        free(juInfo);
        for(int i = 0; shoupai->fulou[i] != NULL; i++){
            free(shoupai->fulou[i]);
        }
        free(shoupai);
    }while(loop);

    printf("終了します\n");

    return 0;
}

