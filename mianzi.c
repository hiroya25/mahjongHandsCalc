#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "common.h"
#include "mianzi.h"

/** @name Mianzi型を扱う関数 */
///@{

/** @brief Mianzi型を初期化する関数
 * @param paixing (Paixing) 面子の種類
 * @param paizhong (Paizhong) 数牌の種類
 * @param value (uint8_t) 牌の値
 * @param isFulou (bool) 手牌がフーロウかどうか
 * @param fulouIndex (uint8_t) フーロウのインデックス
 * @param isHulu (bool) フーロウでない面子のインデックス
 * @param huluIndex (uint8_t) 葫芦のインデックス
 * @return (Mianzi *) Mianzi型のポインタ
 */
Mianzi *initializeMianzi(Paixing paixing, Paizhong paizhong, uint8_t value, bool isFulou, bool isHulu, uint8_t index, Sijia source){
    Mianzi *mianzi      = allocateMemory(1, sizeof(Mianzi), false);
    mianzi->paixing     = paixing;
    mianzi->paizhong    = paizhong;
    mianzi->value       = value;
    mianzi->isFulou     = isFulou;
    mianzi->isHulu      = isHulu;
    mianzi->index       = index;
    mianzi->source      = source;

    return mianzi;
}

/** @brief Mianziの中身をディープコピーする関数
 * @param data (Mianzi *) コピー元
 * @return (Mianzi *) コピー後
 */
Mianzi *copyMianzi(Mianzi *data){
    Mianzi *newMianzi       = allocateMemory(1, sizeof(Mianzi), false);
    newMianzi->paixing      = data->paixing;
    newMianzi->paizhong     = data->paizhong;
    newMianzi->value        = data->value;
    newMianzi->isFulou      = data->isFulou;
    newMianzi->isHulu       = data->isHulu;
    newMianzi->index        = data->index;
    newMianzi->source       = data->source;

    return newMianzi;
}

/** @brief Mianzi型のポインタの先の中身をディープコピーする関数
 * @param data (Mianzi **) コピー元
 * @return (Mianzi **) コピー後
 */
Mianzi **copyMianzi1DArray(Mianzi **data){
    size_t dataCount = 0;
    while(data[dataCount] != NULL) dataCount++;
    Mianzi **newMianzi1DArray = allocateMemory(dataCount + 1, sizeof(Mianzi *), true);

    for(int i = 0; i < dataCount; i++){
        newMianzi1DArray[i] = copyMianzi(data[i]);
    }
    newMianzi1DArray[dataCount] = NULL;

    return newMianzi1DArray;
}

/** @brief 1次元ポインタ配列の示す先に続くように別の1次元ポインタ配列を追加する
 * @param existingArray (Mianzi **) 既存の1次元ポインタ配列(追加される側)
 * @param newArray (Mianzi **) 追加する1次元ポインタ配列(追加する側) (シャローコピーなのでポインタの先はfree禁止)
 */
void **appendMianziArray(Mianzi **existingArray, Mianzi **newArray){
    size_t existingCount = 0;
    while(existingArray[existingCount] != NULL) existingCount++;

    for(int newArrayIndex = 0; newArray[newArrayIndex] != NULL; newArrayIndex++){
        existingArray[existingCount + newArrayIndex] = newArray[newArrayIndex];
    }
}

/** @brief Mianzi **型の要素を反転させる配列
 * @param mianzi (Mianzi ***) Mianzi型のポインタの2次元配列
 * @param size (size_t) 配列のサイズ
 */
void flipMianziArray(Mianzi ***mianzi, size_t size){
    for(int i = 0; i < size; i++){
        size_t count = 0;
        while(mianzi[i][count] != NULL) count++;

        for(int j = 0; j < count / 2; j++){
            Mianzi *tmp = mianzi[i][j];
            mianzi[i][j] = mianzi[i][count - j - 1];
            mianzi[i][count - j - 1] = tmp;
        }
    }
}

///@}

///--- 特殊形 ---//

/** @brief 七対子形 : 和了形(特殊)を求めるプログラム
 * @param shoupai (Shoupai *) 手牌
 * @param hulepai (Pai *) 和了牌
 * @return (Mianzi **) 面子の情報の配列(七対子ではなかったときNULL)
 */
Mianzi **huleMianziQiduizi(Shoupai *shoupai, Pai *hulepai){
    Mianzi **mianzi = allocateMemory(8, sizeof(Mianzi *), true);
    uint8_t count = 0;

    for(int i = 0; i < 4; i++){
        for(int j = 1; j <= 9; j++){
            // 牌がなければスキップ
            if(shoupai->bingpai[i][j] == 0) continue;

            // 対子の場合
            else if(shoupai->bingpai[i][j] == 2){
                // 和了牌と一致するかどうかを確認
                if(i == hulepai->paizhong && j == hulepai->value) mianzi[count] = initializeMianzi(Duizi, i, j, false, true, hulepai->value - j, hulepai->paiSource);
                else mianzi[count] = initializeMianzi(Duizi, i, j, false, false, 0, 0);
                count++;
            }

            else{
                // 存在しない牌の時
                if(i == 3 && 8 <= j) break;

                free(mianzi);
                return NULL;
            }
        }
    }

    // 七対子かどうかを確認
    if(count != 7){
        free(mianzi);
        mianzi = NULL;
    }

    return mianzi;
}

/** @brief 国士無双形 : 和了形(特殊)を求めるプログラム
 * @param shoupai (Shoupai *) 手牌
 * @param hulepai (Pai *) 和了牌
 * @return (TeshuDamanguan) 国士無双であるかどうか
 */
TeshuDamanguan huleMianziGuoshi(Shoupai *shoupai, Pai *hulepai){
    if(shoupai->fulou != NULL && shoupai->fulou[0] != NULL) return None;

    uint8_t count = 0;
    bool duizi = false; // 牌が2枚あるところが存在するか
    bool danqi = true; // 単騎待ち

    // "1"および"9"の么九牌
    for(int i = 0; i < 3; i++){
        for(int j = 1; j <= 9; j += 8){ // 1と9のみ
            switch(shoupai->bingpai[i][j]){
                case 2:
                    // 和了牌だったとき、13面待ち
                    if(hulepai->paizhong == i && hulepai->value == j) danqi = false;
                    count++;
                    duizi = true;
                    break;

                case 1:
                    count++;
                    break;

                default:
                    return None;
            }
        }
    }

    // 字牌
    for(int i = 1; i <= 7; i++){
        switch(shoupai->bingpai[Zihpai][i]){
            case 2:
                // 和了牌だったとき、13面待ち
                if(hulepai->paizhong == Zihpai && hulepai->value == i) danqi = false;
                count++;
                duizi = true;
                break;

            case 1:
                count++;
                break;

            default:
                return None;
        }
    }

    if(count != 13 || !duizi) return None;
    if(danqi) return Guoshi;
    else return GuoshiShiSanMin;
}

/** @brief 九蓮宝燈形 : 和了形(特殊)を求めるプログラム
 * @param shoupai (Shoupai *) 手牌
 * @param hulepai (Pai *) 和了牌
 * @return (TeshuDamanguan) 九蓮宝燈であるかどうか
 */
TeshuDamanguan huleMianziJiulian(Shoupai *shoupai, Pai *hulepai){
    // 萬子、筒子、索子以外は九蓮宝燈ではない
    Paizhong s = hulepai->paizhong;
    if(s != Wanzi && s != Tongzi && s != Suozi) return None;

    for(int i = 1; i <= 9; i++){
        // 1,9牌が3枚以上ないとき
        if((i == 1 || i == 9) && shoupai->bingpai[s][i] < 3) return None;
        
        // 足りてない牌があるとき
        if(shoupai->bingpai[s][i] == 0) return None;
    }

    // 和了牌と同じ牌が2枚もしくは4枚存在するとき、純正九蓮宝燈
    if(shoupai->bingpai[s][hulepai->value] % 2 == 0) return ChunzhengJiulian;
    else return Jiulian;
}

///--- 特殊形 ---//

///--- 一般形 ---//

/** @brief 面子を抜き出す関数
 * @param paizhong (Paizhong) 抜き出す数牌の種類
 * @param bingpai (uint8_t *) 数牌の数の配列へのポインタ(PaiTypeで指定した牌のみ)
 * @param n (uint8_t) 基準にしている牌 - 関数外から呼び出すときは1を入力
 * @param depth (uint8_t) 深度 - 関数外から呼び出すときは1を入力
 * @return (Mianzi ***) 面子の取り方((mianzi *)[] 型)の配列 (->二次元ポインタ配列[choice][index]、最後はNULL)
 */
Mianzi ***mianzi(Paizhong paizhong, uint8_t *bingpai, uint8_t n, uint8_t depth){
    // マクロ
    #define INITMIANZI(paixing) initializeMianzi(paixing, paizhong, n, false, false, 0, 0)
    
    // 基準にしている牌が9より大きいときからの配列を返す
    if(n > 9){
        Mianzi ***mianzi = allocateMemory(2, sizeof(Mianzi **), true);
        return mianzi;
    }

    // 面子を切り取り終わったら、n + 1へ
    if(bingpai[n] == 0) return mianzi(paizhong, bingpai, n + 1, depth);

    // 面子の取り方の総数(それぞれ順子、刻子を抜き取った後)
    size_t shunziSize = 0, keziSize = 0;

    // 順子を抜き取る
    Mianzi ***shunzi = NULL;
    if(n <= 7 && bingpai[n] > 0 && bingpai[n + 1] > 0 && bingpai[n + 2] > 0){
        bingpai[n]--; bingpai[n + 1]--; bingpai[n + 2]--;
        shunzi = mianzi(paizhong, bingpai, n, depth + 1);
        bingpai[n]++; bingpai[n + 1]++; bingpai[n + 2]++;

        if(shunzi != NULL){
            // 初めての時
            if(shunzi[0] == NULL){
                shunzi[0] = allocateMemory(5, sizeof(Mianzi *), true);
            }

            while(shunzi[shunziSize] != NULL) shunziSize++;

            // 取りうる面子すべてに抜き取った順子を追加
            for(int i = 0; i < shunziSize; i++){
                size_t elementSize = 0;
                while(shunzi[i][elementSize] != NULL) elementSize++;
                
                shunzi[i][elementSize] = INITMIANZI(Shunzi);
            }
        }
    }

    // 刻子を抜き取る
    Mianzi ***kezi = NULL;
    if(bingpai[n] >= 3){
        bingpai[n] -= 3;
        kezi = mianzi(paizhong, bingpai, n, depth + 1);
        bingpai[n] += 3;

        if(kezi != NULL){
            // 初めての時
            if(kezi[0] == NULL){
                kezi[0] = allocateMemory(5, sizeof(Mianzi *), true);
            }

            while(kezi[keziSize] != NULL) keziSize++;

            // 取りうる面子すべてに抜き取った順子を追加
            for(int i = 0; i < keziSize; i++){
                size_t elementSize = 0;
                while(kezi[i][elementSize] != NULL) elementSize++;

                kezi[i][elementSize] = INITMIANZI(Kezi);
            }
        }
    }

    //--- 2つのMianzi型の変数を結合する ---//
    // NULLのとき
    if(shunzi == NULL && kezi == NULL) return NULL;
    if(kezi == NULL){
        //呼び出しが最後の時、逆順に反転
        if(depth == 1) flipMianziArray(shunzi, shunziSize);
        return shunzi;
    }
    if(shunzi == NULL){
        //呼び出しが最後の時、逆順に反転
        if(depth == 1) flipMianziArray(kezi, keziSize);
        return kezi;
    }

    // 新しい配列の用意
    size_t newSize = shunziSize + keziSize;
    Mianzi ***result = allocateMemory(newSize + 1, sizeof(Mianzi **), true);

    // シャローコピー
    for(int i = 0; i < shunziSize; i++) result[i] = shunzi[i];
    for(int i = 0; i < keziSize; i++) result[shunziSize + i] = kezi[i];

    // Mianzi ***型のみ解放
    free(shunzi); free(kezi);

    // 呼び出しが最後の時、逆順に反転
    if(depth == 1) flipMianziArray(result, newSize);
    return result;
}

/** @brief 面子構成を求める関数
 * @param shoupai (Shoupai) 手牌
 * @return (Mianzi ***) 面子の取り方((mianzi *)[] 型)の配列 (->二次元ポインタ配列[choice][index]、最後はNULL)
 */
Mianzi ***mianziAll(Shoupai *shoupai){
    // 面子構成を格納する配列
    Mianzi ***allMianzi = allocateMemory(MAX_MIANZI_COMBINATIONS, sizeof(Mianzi **), true);

    // 萬子、筒子、索子の副露していない牌から面子を探す
    for(int i = 0; i < 3; i++){
        Mianzi ***newMianzi = mianzi(i, shoupai->bingpai[i], 1, 1);
        if(newMianzi == NULL) continue;

        if(allMianzi[0] == NULL){
            // allMianziに格納するのが初めての時
            // 配列の成分を全てシャローコピー
            int j = 0;
            while(newMianzi[j] != NULL){
                allMianzi[j] = newMianzi[j];
                j++;
            }
            free(newMianzi);
        }else{
            // allMianziとnewMianziを結合
            size_t allMianziCount = 0, newMianziCount = 0;
            while(allMianzi[allMianziCount] != NULL) allMianziCount++;
            while(newMianzi[newMianziCount] != NULL) newMianziCount++;
            if(newMianziCount == 0){ free(newMianzi); break; }

            // allMianziの要素を初期化
            for(int i = 0; i < newMianziCount - 1; i++){
                for(int j = 0; j < allMianziCount; j++){
                    // allMianzi[i + j] = 空行 に allMianzi[j]のコピーを追加
                    appendMianziArray(allMianzi[i + j], copyMianzi1DArray(allMianzi[j]));
                }
            }

            // newMianziの文字列を追加
            for(int i = 0; i < newMianziCount; i++){
                for(int j = 0; j < allMianziCount; j++){
                    appendMianziArray(allMianzi[j + i * allMianziCount], copyMianzi1DArray(newMianzi[i]));
                }
            }

            // 解放
            for(int i = 0; i < newMianziCount; i++){
                for(int j = 0; newMianzi[i][j] != NULL; j++){
                    free(newMianzi[i][j]);
                }
                free(newMianzi[i]);
            }
            free(newMianzi);

        }
    }

    // 字牌(刻子)の面子を処理
        for(int i = 1; i <= 7; i++){
        if(shoupai->bingpai[Zihpai][i] == 0) continue;
        if(shoupai->bingpai[Zihpai][i] != 3) return NULL;

        size_t allMiazniCount = 0;
        while(allMianzi[allMiazniCount] != NULL) allMiazniCount++;

        // allMianziの全ての要素に文字列を追加
        for(int j = 0; j < allMiazniCount; j++){
            int count = 0;
            while(allMianzi[j][count] != NULL) count++;

            allMianzi[j][count] = initializeMianzi(Kezi, Zihpai, i, false, false, 0, 0);
        }
    }

    // 副露済みの面子を後方に追加
    if(shoupai->fulou != NULL){
        int i = 0;
        while(allMianzi[i] != NULL){
            appendMianziArray(allMianzi[i], copyMianzi1DArray(shoupai->fulou));
            i++;
        }
    }

    return allMianzi;
}

/** @brief 一般形(4面子1雀頭)の和了形を求める関数
 * @param shoupai (Shoupai *) 手牌
 * @param hulepai (PaiAction *) 和了牌
 * @return (Mianzi ***) 面子の取り方((mianzi *)[] 型)の配列 (->二次元ポインタ配列[choice][index]、最後はNULL)
 */
Mianzi ***huleMianziYiban(Shoupai *shoupai, Pai *hulepai){
    Mianzi ***huleMianzi = allocateMemory(MAX_MIANZI_COMBINATIONS, sizeof(Mianzi **), true);

    for(int i = 0; i < 4; i++){
        for(int j = 1; j <= 9; j++){
            if(i == Zihpai && 7 < i) break;             // 存在しない牌のとき
            if(shoupai->bingpai[i][j] < 2) continue;    // 雀頭候補ではないとき

            // 2枚以上ある牌を雀頭候補として抜き取る
            shoupai->bingpai[i][j] -= 2;

            // 雀頭候補以外のshoupaiから面子検索
            Mianzi ***mianziCombinations = mianziAll(shoupai);
            if(mianziCombinations == NULL) continue;
            size_t combinationCount = 0;
            while(mianziCombinations[combinationCount] != NULL) combinationCount++;

            // 4面子
            Mianzi ***compMianzi = allocateMemory(MAX_MIANZI_COMBINATIONS, sizeof(Mianzi **), true);
            for(int k = 0; k < combinationCount; k++){
                compMianzi[k] = allocateMemory(6, sizeof(Mianzi *), true);

                //雀頭
                compMianzi[k][0] = initializeMianzi(Duizi, i, j, false, false, 0, 0);

                // 面子
                appendMianziArray(compMianzi[k], mianziCombinations[k]);
                if(compMianzi[k][4] == NULL) continue;

                size_t count = 0, offset = 0;
                while(huleMianzi[count] != NULL) count++;

                // 和了牌のマークをつける
                for(int l = 0; l < 5; l++){
                    if(compMianzi[k][l]->paizhong == hulepai->paizhong){
                        if(compMianzi[k][l]->paixing == Shunzi && (hulepai->value != compMianzi[k][l]->value && hulepai->value != compMianzi[k][l]->value + 1 && hulepai->value != compMianzi[k][l]->value + 2)) continue;
                        if(compMianzi[k][l]->paixing != Shunzi && hulepai->value != compMianzi[k][l]->value) continue;

                        // 和了牌が見つかったらcompMianziからコピーをしてhuleMianziに追加
                        bool flag = false;
                        for(int m = 0; m < offset; m++){
                            for(int n = 0; n < 5; n++){
                                if(huleMianzi[count + m][n]->isHulu){
                                    if(huleMianzi[count + m][n]->paixing == compMianzi[k][l]->paixing && huleMianzi[count + m][n]->value == compMianzi[k][l]->value){
                                        flag = true;
                                        break;
                                    }
                                }
                            }
                        }

                        if(flag) break;

                        huleMianzi[count + offset] = copyMianzi1DArray(compMianzi[k]);
                        huleMianzi[count + offset][l]->isHulu = true;
                        huleMianzi[count + offset][l]->index  = hulepai->value - huleMianzi[count][l]->value;
                        huleMianzi[count + offset][l]->source = hulepai->paiSource;
                        offset++;
                    }
                }
            }
            
            // 解放(mianziCombinations *の指す先はシャローコピー)
            for(int k = 0; k < combinationCount; k++){
                free(mianziCombinations[k]);
            }
            free(mianziCombinations);

            // 解放
            for(int m = 0; compMianzi[m] != NULL; m++){
                for(int n = 0; compMianzi[m][n] != NULL; n++){
                    free(compMianzi[m][n]);
                }
                free(compMianzi[m]);
            }
            free(compMianzi);

            shoupai->bingpai[i][j] += 2;
        }
    }
    return huleMianzi;
}

///--- 一般形 ---//
/** @brief 和了形を求める関数
 * @param flag (uint8_t *) 特殊形の和了形であるか(0...x, 1...国士無双, 2...九蓮宝燈)
 * @param shoupai (Shoupai *) 手牌
 * @param hulepai (PaiAction *) 和了牌
 * @return (Mianzi ***) 面子の取り方((mianzi *)[] 型)の配列 (->二次元ポインタ配列[choice][index]、最後はNULL)
 */
Mianzi ***huleMianzi(TeshuDamanguan *flag, Shoupai *shoupai, Pai *hulepai){
    // 国士無双, 九蓮宝燈の確認
    *flag = huleMianziGuoshi(shoupai, hulepai);
    if(*flag != None) return NULL;

    *flag = huleMianziJiulian(shoupai, hulepai);
    if(*flag != None) return NULL;

    Mianzi ***mianzi = huleMianziYiban(shoupai, hulepai);
    Mianzi **mianziQiduizi = huleMianziQiduizi(shoupai, hulepai);
    if(mianziQiduizi != NULL){
        size_t count = 0;
        while(mianzi[count] != NULL) count++;
        mianzi[count] = mianziQiduizi;
    }

    return mianzi;
}



/*
int main(){
    // --- 手牌の初期化 --- //
    Mianzi **fulou = (Mianzi **)malloc(2 * sizeof(Mianzi *));
    fulou[0] = allocateMemory(2, sizeof(Mianzi), false);
    fulou[0]->paixing = Shunzi;
    fulou[0]->paizhong = Tongzi;
    fulou[0]->value = 5;
    fulou[0]->isFulou = true;
    fulou[0]->isHulu = false;
    fulou[0]->index = 0;
    fulou[0]->source = 0;
    fulou[1] = NULL;

    Shoupai shoupai = {
        {{0, 2, 2, 2, 2, 2, 2, 2, 0, 0},  // 萬子
         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 筒子
         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 索子
         {0, 0, 0, 0, 0, 0, 0, 0}},       // 字牌
        NULL,
        NULL
    };

    Pai hulepai = { Wanzi, 1, Zimo };

    uint8_t flag;
    Mianzi ***result = huleMianzi(&flag, &shoupai, &hulepai);

    switch(flag){
        case 1:
            puts("国士無双");
            break;

        case 2:
            puts("九蓮宝燈");
            break;

        default:
    }

    // 結果を表示
    if(result != NULL){
        int i = 0;
        while(result[i] != NULL){
            int count = 0;
            while(result[i][count] != NULL) count++;

            printf("面子 : ");
            for(int j = 0; j < count; j++){
                switch(result[i][j]->paizhong){
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

                switch(result[i][j]->paixing){
                    case Shunzi:
                        printf("%d%d%d", result[i][j]->value, result[i][j]->value + 1, result[i][j]->value + 2);
                        break; 

                    case Kezi:
                        printf("%d%d%d", result[i][j]->value, result[i][j]->value, result[i][j]->value);
                        break; 

                    case Gangzi:
                        printf("%d%d%d%d", result[i][j]->value, result[i][j]->value, result[i][j]->value, result[i][j]->value);
                        break; 

                    case Duizi:
                        printf("%d%d", result[i][j]->value, result[i][j]->value);
                        break; 
                }
                if(result[i][j]->isHulu){
                    printf("(Agari : %d, %d)", result[i][j]->value + result[i][j]->index, result[i][j]->source);
                }
                if(result[i][j]->isFulou){
                    printf("(fulou : %d, %d)", result[i][j]->value + result[i][j]->index, result[i][j]->source);
                }
                if(j != count - 1) printf(", ");
            }
            printf("\n");
            i++;
        }
        // メモリ解放
        for (int j = 0; j < i; j++) {
            free(result[j]);
        }
        free(result);
    }else{
        puts("nun");
    }

    return 0;
}
*/