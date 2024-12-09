#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "common.h"

/**
 * @brief 汎用的にメモリを確保する関数
 * @param size (size_t) 要素数
 * @param typeSize (size_t) 確保する型のサイズ
 * @param clearFlag (bool) 要素を0で初期化するかどうか
 * @return void* 確保されたメモリのポインタ
 */
void *allocateMemory(size_t size, size_t typeSize, bool clearFlag){
    void *ptr;
    if(clearFlag){
        ptr = calloc(size, typeSize);
    }else{
        ptr = malloc(size * typeSize);
    }

    if(ptr == NULL){
        fprintf(stderr, "メモリ確保に失敗しました。サイズ: %zu, 型のサイズ: %zu\n", size, typeSize);
        exit(EXIT_FAILURE);
    }

    return ptr;
}