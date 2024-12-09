#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "mahjongStructs.h"

/**
 * @name 和了形を求めるプログラム
 * 手牌から和了形を求める
 */
///@{
//--- 一般形 ---//
// 前方宣言
Mianzi ***mianziAll(Shoupai *shoupai);
Mianzi ***mianzi(PaiType paiType, int *bingpai, int n, int depth);

/**
 * @brief 一般形(4面子1雀頭)の和了形を求める関数
 * @param shoupai (Shoupai *) 手牌
 * @param hulepai (PaiAction *) 和了牌
 * @return (Mianzi ***) 面子の取り方((mianzi *)[] 型)の配列 (->二次元ポインタ配列[choice][index]、最後はNULL)
 */
Mianzi ***huleMianziYiban(Shoupai *shoupai, Pai *hulepai){
    Mianzi ***huleMianzi = allocateMianzi2DArray(MAX_MIANZI_COMBINATIONS);

    for(int i = 0; i < 4; i++){
        for(int j = 1; j <= 9; j++){
            if(i == 3 && 7 < j) break;                  // 存在しない牌のとき
            if(shoupai->bingpai[i][j] < 2) continue;    // 雀頭候補ではないとき

            // 2枚以上ある牌を雀頭候補として抜き取る
            shoupai->bingpai[i][j] -= 2;

            Mianzi ***mianziCombinations = mianziAll(shoupai);
            if(mianziCombinations == NULL) continue;
            int combinationCount = 0;
            while(mianziCombinations[combinationCount] != NULL) combinationCount++;

            // 一般形の和了形(完成)
            Mianzi ***completeMianzi = allocateMianzi2DArray(MAX_MIANZI_COMBINATIONS);

            for(int k = 0; k < combinationCount; k++){
                // 雀頭
                completeMianzi[k] = allocateMianzi1DArray(6);

                completeMianzi[k][0] = allocateMianzi();
                completeMianzi[k][0]->mianziType = Duizi;
                completeMianzi[k][0]->paiType    = i;
                completeMianzi[k][0]->number     = j;
                completeMianzi[k][0]->isFulou    = false;
                completeMianzi[k][0]->isHulu     = false;   // まだ確定していないが、とりあえず
                completeMianzi[k][0]->paiInfo    = NULL;

                // 面子
                appendMianziArray(completeMianzi[k], mianziCombinations[k]);
                if(completeMianzi[k][4] == NULL) continue;

                // 和了牌のマークをつける
                for(int l = 0; l < 5; l++){
                    if(completeMianzi[k][l]->paiType == hulepai->paiType){
                        if(completeMianzi[k][l]->mianziType == Shunzi && (hulepai->number != completeMianzi[k][l]->number && hulepai->number != completeMianzi[k][l]->number + 1 && hulepai->number != completeMianzi[k][l]->number + 2)) continue;
                        if(completeMianzi[k][l]->mianziType != Shunzi && hulepai->number != completeMianzi[k][l]->number) continue;

                        // 和了牌が見つかったらallMianziからコピーをしてhuleMianziに追加
                        int count = 0;
                        while(huleMianzi[count] != NULL){
                            for(int m = 0; m < 5; m++){
                                if(huleMianzi[count][m]->isHulu){
                                    if(huleMianzi[count][m]->mianziType == completeMianzi[k][l]->mianziType && huleMianzi[count][m]->number == completeMianzi[k][l]->number){
                                        count = -1;
                                        break;
                                    }
                                }
                            }
                            if(count == -1) break;
                            count++;
                        }
                        if(count == -1) continue;

                        huleMianzi[count] = copyMianzi1DArray(completeMianzi[k]);
                        huleMianzi[count][l]->isHulu = true;
                        huleMianzi[count][l]->paiInfo = malloc(sizeof(Pai));
                        if (huleMianzi[count][l]->paiInfo == NULL){
                            perror("メモリ確保に失敗しました");
                            exit(EXIT_FAILURE);
                        }
                        huleMianzi[count][l]->paiInfo->paiType   = hulepai->paiType;
                        huleMianzi[count][l]->paiInfo->number    = hulepai->number;
                        huleMianzi[count][l]->paiInfo->paiSource = hulepai->paiSource;
                    }
                }
            }

            // 解放(mianziCombinations *の指す先はシャローコピー)
            for(int k = 0; k < combinationCount; k++){
                free(mianziCombinations[k]);
            }
            free(mianziCombinations);

            // 解放
            for(int m = 0; completeMianzi[m] != NULL; m++){
                for(int n = 0; completeMianzi[m][n] != NULL; n++){
                    if(completeMianzi[m][n]->paiInfo != NULL) free(completeMianzi[m][n]->paiInfo);
                    free(completeMianzi[m][n]);
                }
                free(completeMianzi[m]);
            }
            free(completeMianzi);

            shoupai->bingpai[i][j] += 2;
        }
    }

    return huleMianzi;
}

/**
 * @brief 面子構成を求める関数
 * @param shoupai (Shoupai) 手牌
 * @return (Mianzi ***) 面子の取り方((mianzi *)[] 型)の配列 (->二次元ポインタ配列[choice][index]、最後はNULL)
 */
Mianzi ***mianziAll(Shoupai *shoupai){
    // 面子構成を格納する配列
    Mianzi ***allMianzi = allocateMianzi2DArray(MAX_MIANZI_COMBINATIONS);

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
            int allMianziCount = 0;
            int newMianziCount = 0;
            while(allMianzi[allMianziCount] != NULL) allMianziCount++;
            while(newMianzi[newMianziCount] != NULL) newMianziCount++;

            // allMianziの要素で初期化
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
                    if(newMianzi[i][j]->paiInfo != NULL) free(newMianzi[i][j]->paiInfo);
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

        int allMiazniCount = 0;
        while(allMianzi[allMiazniCount] != NULL) allMiazniCount++;

        // allMianziの全ての要素に文字列を追加
        for(int j = 0; j < allMiazniCount; j++){
            int count = 0;
            while(allMianzi[j][count] != NULL) count++;

            allMianzi[j][count] = allocateMianzi();
            allMianzi[j][count]->mianziType = Kezi;
            allMianzi[j][count]->paiType    = Zihpai;
            allMianzi[j][count]->number     = i;
            allMianzi[j][count]->isFulou    = false;
            allMianzi[j][count]->isHulu     = false;    //確定していないが、とりあえずfalse
            allMianzi[j][count]->paiInfo    = NULL;
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


/**
 * @brief 面子を抜き出す関数
 * @param paiType (PaiType) 抜き出す数牌の種類
 * @param bingpai (int *) 数牌の数の配列へのポインタ(PaiTypeで指定した牌のみ)
 * @param n (int) 基準にしている牌 - 関数外から呼び出すときは1を入力
 * @param depth (int) 深度 - 関数外から呼び出すときは1を入力
 * @return (Mianzi ***) 面子の取り方((mianzi *)[] 型)の配列 (->二次元ポインタ配列[choice][index]、最後はNULL)
 */
Mianzi ***mianzi(PaiType paiType, int *bingpai, int n, int depth){
    // 基準にしている牌が9より大きいとき空の配列を返す
    if(n > 9){
        Mianzi ***mianzi = allocateMianzi2DArray(2);
        return mianzi;
    }

    // 面子を抜き取り終わったら、n + 1へ
    if(bingpai[n] == 0) return mianzi(paiType, bingpai, n + 1, depth);

    // 面子の取り方の総数(それぞれ順子, 刻子を抜き取った後)
    int shunziSize = 0, keziSize = 0;

    // 順子を抜き取る
    Mianzi ***shunzi = NULL;
    if(n <= 7 && bingpai[n] > 0 && bingpai[n + 1] > 0 && bingpai[n + 2] > 0){
        bingpai[n]--; bingpai[n + 1]--; bingpai[n + 2]--;
        shunzi = mianzi(paiType, bingpai, n, depth + 1);
        bingpai[n]++; bingpai[n + 1]++; bingpai[n + 2]++;

        if(shunzi != NULL){
            // 初めての時
            if(shunzi[0] == NULL){
                shunzi[0] = allocateMianzi1DArray(5);
            }

            while(shunzi[shunziSize] != NULL) shunziSize++;

            // 取りうる面子すべてに抜き取った順子を追加
            for(int i = 0; i < shunziSize; i++){
                int elementSize = 0;
                while(shunzi[i][elementSize] != NULL) elementSize++;
                
                shunzi[i][elementSize] = allocateMianzi();
                shunzi[i][elementSize]->mianziType = Shunzi;
                shunzi[i][elementSize]->paiType    = paiType;
                shunzi[i][elementSize]->number     = n;
                shunzi[i][elementSize]->isFulou    = false;
                shunzi[i][elementSize]->isHulu     = false; //確定していないが、とりあえずfalse
                shunzi[i][elementSize]->paiInfo    = NULL;
            }
        }
    }

    // 刻子を抜き取る
    Mianzi ***kezi = NULL;
    if(bingpai[n] >= 3){
        bingpai[n] -= 3;
        kezi = mianzi(paiType, bingpai, n, depth + 1);
        bingpai[n] += 3;

        if(kezi != NULL){
            // 初めての時
            if(kezi[0] == NULL){
                kezi[0] = allocateMianzi1DArray(5);
            }

            while(kezi[keziSize] != NULL) keziSize++;

            // 取りうる面子すべてに抜き取った順子を追加
            for(int i = 0; i < keziSize; i++){
                int elementSize = 0;
                while(kezi[i][elementSize] != NULL) elementSize++;
                
                kezi[i][elementSize] = allocateMianzi();
                kezi[i][elementSize]->mianziType = Kezi;
                kezi[i][elementSize]->paiType    = paiType;
                kezi[i][elementSize]->number     = n;
                kezi[i][elementSize]->isFulou    = false;
                kezi[i][elementSize]->isHulu     = false;   //確定していないが、とりあえずfalse
                kezi[i][elementSize]->paiInfo    = NULL;
            }
        }
    }

    //--- 2つのMianzi型の変数を結合する ---//
    // NULLのとき
    if(shunzi == NULL && kezi == NULL) return NULL;
    if(kezi == NULL){
        //呼び出しが最後の時、逆順に反転
        if(depth == 1){
            for(int i = 0; i < shunziSize; i++){
                int count = 0;
                while(shunzi[i][count] != NULL) count++;

                for(int j = 0; j < count / 2; j++){
                    Mianzi *tmp = shunzi[i][j];
                    shunzi[i][j] = shunzi[i][count - j - 1];
                    shunzi[i][count - j - 1] = tmp;
                }
            }
        }
        return shunzi;
    }
    if(shunzi == NULL){
        //呼び出しが最後の時、逆順に反転
        if(depth == 1){
            for(int i = 0; i < keziSize; i++){
                int count = 0;
                while(kezi[i][count] != NULL) count++;

                for(int j = 0; j < count / 2; j++){
                    Mianzi *tmp = kezi[i][j];
                    kezi[i][j] = kezi[i][count - j - 1];
                    kezi[i][count - j - 1] = tmp;
                }
            }
        }
        return kezi;
    }

    // 新しい配列の用意
    int newSize = shunziSize + keziSize;
    Mianzi ***result = allocateMianzi2DArray(newSize + 1);

    // シャローコピー(Mianzi **型)
    for(int i = 0; i < shunziSize; i++){
        result[i] = shunzi[i];
    }
    for(int i = 0; i < keziSize; i++){
        result[shunziSize + i] = kezi[i];
    }

    result[newSize] = NULL;

    // Mianzi ***型のみ解放
    free(shunzi); free(kezi);

    //呼び出しが最後の時、逆順に反転
    if(depth == 1){
        for(int i = 0; i < newSize; i++){
            int count = 0;
            while(result[i][count] != NULL) count++;

            for(int j = 0; j < count / 2; j++){
                Mianzi *tmp = result[i][j];
                result[i][j] = result[i][count - j - 1];
                result[i][count - j - 1] = tmp;
            }
        }
    }
    return result;
}

//--- 特殊形 ---//
/**
 * @brief 七対子形 : 和了形(特殊)を求めるプログラム
 * @param shoupai (Shoupai *) 手牌
 * @param hulepai (PaiAction *) 和了牌
 * @return (Mianzi *) 面子の情報の配列(七対子ではなかったときNULL)
 */
Mianzi **huleMianziQiduizi(Shoupai *shoupai, Pai *hulepai){
    // 7組の面子を格納
    Mianzi **mianzi = allocateMianzi1DArray(8);
    int count = 0;

    for(int i = 0; i < 4; i++){
        for(int j = 1; j <= 9; j++){
            // 牌がなければスキップ
            if(shoupai->bingpai[i][j] == 0) continue;

            // 対子の場合
            else if(shoupai->bingpai[i][j] == 2){
                mianzi[count] = allocateMianzi();
                mianzi[count]->mianziType = Duizi;
                mianzi[count]->paiType    = i;
                mianzi[count]->number     = j;
                mianzi[count]->isFulou    = false;

                // 和了牌がどうかを確認
                if(i == hulepai->paiType && j == hulepai->number){
                    mianzi[count]->isHulu  = true;
                    mianzi[count]->paiInfo = hulepai;
                }else{
                    mianzi[count]->isHulu  = false;
                    mianzi[count]->paiInfo = NULL;
                }
                count++;
            }else{
                // 存在しない牌のとき
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
    }else{
        mianzi[7] = NULL;
    }
    return mianzi;
}

///@}

/**
 * @brief 符を求めるプログラム
 * @param shoupai (Shoupai *) 手牌
 * @param hulepai (PaiAction *) 和了牌
 * @return (Mianzi **) 面子の情報の配列(七対子ではなかったときNULL)
 */
Hudi *getHudi(Mianzi **mianzi, Fengpai zhuangfeng, Fengpai menfeng){
    // 初期設定
    Hudi *hudi = malloc(sizeof(Hudi));
    hudi->fu = 20;
    hudi->menqian = true;
    hudi->zimo = true;
    hudi->nShunzi = 0;
    hudi->nKezi   = 0;
    hudi->nAnkezi = 0;
    hudi->nGangzi = 0;
    hudi->nZipai  = 0;
    hudi->nYaojiu = 0;
    hudi->danqi = false;
    hudi->pinghu = false;
    hudi->zhuangfeng = zhuangfeng;
    hudi->menfeng = menfeng;
    
    unsigned mianziCount = 0;
    while(mianzi[mianziCount] != NULL) mianziCount++;

    for(int i = 0; i < mianziCount; i++){
        Mianzi *p = mianzi[i];  // 処理を行う面子

        if(p->isHulu && p->paiInfo->paiSource != Zimo) hudi->zimo = false;      // ロン和了の時
        if(p->isFulou) hudi->menqian = false;                                   // 副露している時
        if(isYaojiu(p)) hudi->nYaojiu++;                                        // 幺九牌を含む時
        if(p->paiType == Zihpai) hudi->nZipai++;                                // 字牌を含むとき
        if(p->isHulu && p->mianziType == Duizi) hudi->danqi++;                  // 単騎待ちのとき

        if(mianziCount != 5) continue;  // 4面子1雀頭形でない場合は以下の処理はスキップ

        switch (p->mianziType){
        // 対子(雀頭)
        case Duizi:
            // 字牌の場合
            if(p->paiType == Zihpai){
                if(p->number == zhuangfeng + 1) hudi->fu += 2;        // 場風のとき
                if(p->number == menfeng + 1) hudi->fu += 2;           // 自風のとき
                if(5 <= p->number && p->number <= 7) hudi->fu += 2;   // 三元牌のとき
            }
            if(hudi->danqi) hudi->fu += 2;  // 単騎待ちの時

            break;
        
        // 刻子、槓子
        case Kezi:
        case Gangzi:
            hudi->nKezi++;
            int fu = 2;     //刻子の符を 2 で初期化
            if(isYaojiu(p)) fu *= 2;                                    // 幺九牌のとき
            if(!p->isFulou){ fu *= 2; hudi->nAnkezi++; }                // 暗刻子のとき(槓子含む)
            if(p->mianziType == Gangzi){ fu *= 4; hudi->nGangzi++; }    // 槓子のとき

            hudi->fu += fu;
            break;

        // 順子
        case Shunzi:
            hudi->nShunzi++;
            // 和了牌を含む面子の時
            if(p->isHulu){
                if(p->number + 1 == p->paiInfo->number) hudi->fu += 2;                                      // 嵌張待ちのとき
                if(p->number == p->paiInfo->number || p->number + 2 == p->paiInfo->number)hudi->fu += 2;    // 辺張待ちのとき
            }
            break;

        default:
            break;
        }
    }

    if(mianziCount == 7) hudi->fu = 25;  // 七対子形の場合
    else if(mianziCount == 5){ // 4面子1雀頭形の場合
        hudi->pinghu = (hudi->menqian && hudi->fu == 20);   // 門前で20符なら平和
        if(hudi->zimo){                                 // ツモ和了のとき
            if(!hudi->pinghu) hudi->fu += 2;                // 平和でないとき
        }else{                                          // ロン和了の場合
            if(hudi->menqian)       hudi->fu += 10;         // 門前のとき
            else if(hudi->fu == 20) hudi->fu = 30;          // 喰い平和のとき
        }
        hudi->fu = ceil(hudi->fu / 10.0) * 10;          // 10点未満は切り上げ
    }

    return hudi;
}

/**
 * @brief 役を求めるプログラム
 * @param shoupai (Shoupai *) 手牌
 * @param hulepai (PaiAction *) 和了牌
 * @return (Mianzi **) 面子の情報の配列(七対子ではなかったときNULL)
 */
Yi **getHupai(Mianzi **mianzi, Hudi *hudi, Yi **preHupai){
    // 役満の初期値を設定する。状況役に役満(天和、地和)が含まれている場合はそれを設定、ない場合は初期化。
    Yi** damanguan = (preHupai[0] != NULL && preHupai[0]->fanshu < 0) ? preHupai : (Yi **)malloc(MAX_HUPAI * sizeof(Yi *));
    unsigned damanguanCount = (preHupai[0] != NULL && preHupai[0]->fanshu == 0) ? 1 : 0;

    unsigned mianziCount = 0;
    while(mianzi[mianziCount] != NULL) mianziCount++;

    // マクロに変数を渡すマクロ
    #define CREATE_YI(...) CREATE_NEW_YI(damanguan, &damanguanCount, __VA_ARGS__)

    //--- 役満 ---//
    if(mianziCount == 1){
        switch(mianzi[0]->damanguan){
            // 国士無双
            case 1:
                if(hudi->danqi){
                    ("国士無双十三面", -2);
                }else{
                    CREATE_YI("国士無双", -1);
                }
                break;


            // 九蓮宝燈
            case 2:
                break;

            default:
                return NULL;
        }

        damanguanCount++;
    }

    // 四暗刻
    if(hudi->nAnkezi == 4){
        if(hudi->danqi){
            CREATE_YI("四暗刻単騎", -2);
        }else{
            CREATE_YI("四暗刻", -1);
        }
    }

    // 大三元
    if(hudi->kezi[Zihpai][5] + hudi->kezi[Zihpai][6] + hudi->kezi[Zihpai][7] == 3){
        SiJia baojia = 0;

        for(int i = 3; 0 <= i; i--){
            if(!mianzi[i]->isFulou) continue; 

            if(mianzi[i]->paiType == Zihpai && (5 <= mianzi[i]->number && mianzi[i]->number <= 7)){
                baojia = mianzi[i]->paiInfo->paiSource;
                break;
            }
        }

        CREATE_YI("大三元", -1, baojia);
    }

    // 四喜和
    if(hudi->kezi[Zihpai][1] + hudi->kezi[Zihpai][2] + hudi->kezi[Zihpai][3] + hudi->kezi[Zihpai][4] == 4){
        SiJia baojia = 0;

        for(int i = 3; 0 <= i; i--){
            if(!mianzi[i]->isFulou) continue;

            if(mianzi[i]->paiType == Zihpai && (1 <= mianzi[i]->number && mianzi[i]->number <= 4)){
                baojia = mianzi[i]->paiInfo->paiSource;
                break;
            }
        }

        CREATE_YI("大四喜", -2, baojia);
    }else if(hudi->kezi[Zihpai][1] + hudi->kezi[Zihpai][2] + hudi->kezi[Zihpai][3] + hudi->kezi[Zihpai][4] == 3 && mianzi[0]->paiType == Zihpai && 1 <= mianzi[0]->number && mianzi[0]->number <= 4){
        CREATE_YI("小四喜", -1);
    }

    // 字一色
    if(hudi->nZipai == mianziCount) CREATE_YI("字一色", -1);

    // 緑一色
    bool isLvyise = true;
    for(int i = 4; 0 <= i; i--){
        // 萬子、筒子が含まれている
        if(mianzi[i]->paiType == Wanzi || mianzi[i]->paiType == Tongzi){
            isLvyise = false;
            break;
        }

        // 發以外の字牌が含まれている
        if(mianzi[i]->paiType == Zihpai && mianzi[i]->number != 6){
            isLvyise = false;
            break;
        }

        // 一五七九索が含まれている
        if(mianzi[i]->paiType == Suozi && (mianzi[i]->number == 1 || mianzi[i]->number == 5 || mianzi[i]->number == 7 || mianzi[i]->number == 9)){
            isLvyise = false;
            break;
        }
    }
    if(isLvyise) CREATE_YI("緑一色", -1);

    // 清老頭
    if(hudi->nKezi == 4 && hudi->nYaojiu == 5 && hudi->nZipai == 0) CREATE_YI("清老頭", -1);

    //四槓子
    if(hudi->nGangzi == 4) CREATE_YI("四槓子", -1);


    // 役満がある場合終了
    if(damanguanCount > 0) return damanguan;

    Yi **hupai = preHupai;
    int hupaiCount = 0;
    while(hupai[hupaiCount] != NULL) hupaiCount++;

    // マクロに変数を渡すマクロ
    #undef CREATE_YI
    #define CREATE_YI(...) CREATE_NEW_YI(hupai, &hupaiCount, __VA_ARGS__)


    //--- 一般役 ---//
    // 門前清自摸和
    if(hudi->menqian && hudi->zimo) CREATE_YI("門前清自摸和", 1);

    // 翻牌
    char *str;
    char **fengHanzi = {"東", "南", "西", "北"};
    if(hudi->kezi[Zihpai][hudi->zhuangfeng + 1] > 0){
        sprintf(str, "場風 : %s", fengHanzi[hudi->zhuangfeng]);
        CREATE_YI(str, 1);
    }
    if(hudi->kezi[Zihpai][hudi->menfeng + 1] > 0){
        sprintf(str, "自風 : %s", fengHanzi[hudi->zhuangfeng]);
        CREATE_YI(str, 1);
    }
    if(hudi->kezi[Zihpai][5] > 0) CREATE_YI("翻牌 白", 1);
    if(hudi->kezi[Zihpai][6] > 0) CREATE_YI("翻牌 發", 1);
    if(hudi->kezi[Zihpai][7] > 0) CREATE_YI("翻牌 中", 1);

    // 平和
    if(hudi->pinghu) CREATE_YI("平和", 1);

    // 断幺九
    if(hudi->nYaojiu == 0) CREATE_YI("断幺九", 1);

    // 一盃口、二盃口
    if(hudi->menqian){
        int beikou = 0;

    }

    // 三色同順

    // 一気通貫

    // 混全帯幺九
    if(hudi->nYaojiu == 5 && hudi->nShunzi > 0 && hudi->nZipai > 0) CREATE_YI("混全帯幺九", hudi->menqian ? 2 : 1);

    // 七対子
    if(mianziCount == 7) CREATE_YI("七対子", 2);

    // 対々和
    if(hudi->nKezi == 4) CREATE_YI("対々和", 2);

    // 三暗刻
    if(hudi->nKezi == 3) CREATE_YI("三暗刻", 2);

    // 三槓子
    if(hudi->nGangzi == 3) CREATE_YI("三槓子", 2);

    // 三色同順
    for(int i = 1; i <= 9; i++){
        if(hudi->kezi[Wanzi][i] + hudi->kezi[Tongzi][i] + hudi->kezi[Suozi][i] == 3) CREATE_YI("三色同順", 2);
    }

    // 混老頭
    if(hudi->nYaojiu == mianziCount && hudi->nShunzi == 0 && hudi->nZipai > 0) CREATE_YI("混老頭", 2);

    // 小三元
    if(hudi->kezi[Zihpai][5] + hudi->kezi[Zihpai][6] + hudi->kezi[Zihpai][7] == 2 && mianzi[0]->paiType == Zihpai && 5 <= mianzi[0]->number && mianzi[0]->number <= 7) CREATE_YI("小三元", 2);

    // 混一色

    // 純全帯幺九

    // 清一色

    return hupai;
}

int main(){
    // --- 手牌の初期化 --- //
    Mianzi **fulou = (Mianzi **)malloc(2 * sizeof(Mianzi *));
    Pai *paiInfo = malloc(sizeof(Pai));
    fulou[0] = allocateMianzi();
    fulou[0]->mianziType = Shunzi;
    fulou[0]->paiType = Tongzi;
    fulou[0]->number = 5;
    fulou[0]->isFulou = true;
    fulou[0]->isHulu = false;
    fulou[0]->paiInfo = paiInfo;
    paiInfo->number = 6;
    paiInfo->paiType = Tongzi;
    paiInfo->paiSource = Zimo;
    fulou[1] = NULL;

    Shoupai shoupai = {
        {{0, 3, 3, 3, 0, 0, 0, 0, 0, 0},  // 萬子
         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 筒子
         {0, 0, 0, 0, 0, 0, 0, 0, 2, 0},  // 索子
         {0, 0, 0, 0, 0, 0, 0, 0}},       // 字牌
        fulou,
        NULL
    };
    

    Pai hulepai = { Wanzi, 1, Zimo };

    Mianzi ***result = huleMianziYiban(&shoupai, &hulepai);

    // 結果を表示
    if(result != NULL){
        int i = 0;
        while(result[i] != NULL){
            int count = 0;
            while(result[i][count] != NULL) count++;

            printf("面子 : ");
            for(int j = 0; j < count; j++){
                switch(result[i][j]->paiType){
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

                switch(result[i][j]->mianziType){
                    case Shunzi:
                        printf("%d%d%d", result[i][j]->number, result[i][j]->number + 1, result[i][j]->number + 2);
                        break; 

                    case Kezi:
                        printf("%d%d%d", result[i][j]->number, result[i][j]->number, result[i][j]->number);
                        break; 

                    case Gangzi:
                        printf("%d%d%d%d", result[i][j]->number, result[i][j]->number, result[i][j]->number, result[i][j]->number);
                        break; 

                    case Duizi:
                        printf("%d%d", result[i][j]->number, result[i][j]->number);
                        break; 
                }
                if(result[i][j]->isHulu){
                    printf("(Agari : %d, %d)", result[i][j]->paiInfo->number, result[i][j]->paiInfo->paiSource);
                }
                if(result[i][j]->isFulou){
                    printf("(fulou : %d, %d)", result[i][j]->paiInfo->number, result[i][j]->paiInfo->paiSource);
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