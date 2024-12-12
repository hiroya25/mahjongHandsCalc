#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "common.h"
#include "mianzi.h"
#include "hule.h"

/** @brief 赤ドラ(0)を5に変換する関数
 * @param shoupai (Shoupai *) 打牌可能な牌
 * @return (Shoupai *) 0->5にした結果
 */
Shoupai *convertHongpaiToFive(Shoupai *shoupai){
    Shoupai *newShoupai = allocateMemory(1, sizeof(Shoupai), false);
    memcpy(newShoupai->bingpai, shoupai->bingpai, sizeof(uint8_t) * 4 * 10);
    newShoupai->fulou = shoupai->fulou;
    newShoupai->fulouHongpai = shoupai->fulouHongpai;
    newShoupai->hulepai = shoupai->hulepai;

    for(int i = 0; i < 3; i++){
        newShoupai->bingpai[i][5] += newShoupai->bingpai[i][0];
        newShoupai->bingpai[i][0] = 0;
    }

    return newShoupai;
}

/** @brief 幺九牌を含むかどうかを確認する関数
 * @param mianzi (Mianzi) 確認する面子
 * @return (bool) 幺九牌を含む場合はtrue、含まない場合はfalse
 */
bool isYaojiu(Mianzi *mianzi){
    // 字牌の場合
    if(mianzi->paizhong == Zihpai) return false;

    // 順子の場合
    if(mianzi->paixing == Shunzi) return (mianzi->value == 1 || mianzi->value == 7);

    // 刻子、槓子、対子の場合
    if(mianzi->paixing == Kezi || mianzi->paixing == Gangzi || mianzi->paixing == Duizi) return (mianzi->value == 1 || mianzi->value == 9);

    // その他の場合
    return false;
}

/** @brief 役の情報が同じかを確認する関数
 * @param data1 (Mianzi*) 比較対象の面子1
 * @param data2 (Mianzi*) 比較対象の面子2
 * @return (bool) 同じ面子ならtrue、異なればfalse
 */
bool isSameMianzi(Mianzi* data1, Mianzi* data2) {
    return (data1->paixing == data2->paixing) && (data1->paizhong == data2->paizhong) && (data1->value == data2->value);
}

/** @brief 役を新たに作成する関数
 * @param name (char *) 役の名前
 * @param fanshu (uint8_t) 翻数
 * @param baojia (Sijia) パオがある場合、四家のうちどこか(0...パオなし)
 * @return (Yi *) 役の情報が入った型
 */
Yi *createYi(char *name, uint8_t fanshu, Sijia baojia){
    Yi *yi = allocateMemory(1, sizeof(Yi), false);

    yi->name = strdup(name);
    if(yi->name == NULL){
        fprintf(stderr, "メモリ確保に失敗しました。サイズ: %zu, 型のサイズ: %zu\n", sizeof(name), 1);
        return NULL;
    }
    yi->fanshu = fanshu;
    yi->baojia = baojia;

    return yi;
}

/** @brief 役の配列をディープコピーする関数
 * @param yi (Yi *) コピー元の役
 * @return (Yi *) 役の情報が入った型
 */
Yi **copyYiArray(Yi **data){
    size_t count = 0;
    while(data[count] != NULL) count++;

    Yi **newData = allocateMemory(MAX_HUPAI, sizeof(Yi *), true);
    for(int i = 0; i < count; i++){
        newData[i] = createYi(data[i]->name, data[i]->fanshu, data[i]->baojia);
    }

    return newData;
}

/** @brief 役の配列を結合する関数
 * @param data1 (Yi **) 既存の役配列
 * @param data2 (Yi **) 結合する役配列
 */
void concatYiArray(Yi **data1, Yi **data2){
    if (data1 == NULL || data2 == NULL) {
        fprintf(stderr, "結合する配列がNULLです\n");
        return;
    }

    // data1とdata2の要素数を数える
    size_t data1Count = 0, data2Count = 0;
    while(data1[data1Count] != NULL) data1Count++;
    while(data2[data2Count] != NULL) data2Count++;

    // data2の内容をシャローコピー
    for(int i = 0; i < data2Count; i++){
        data1[data1Count + i] = data2[i];
    }

    // 末尾をNULLに
    data1[data1Count + data2Count] = NULL;
}

/** @brief 役の配列を解放する関数
 * @param data (Yi **) 役配列
 */
void freeYiArray(Yi **data){
    if(data == NULL) return;

    for(int i = 0; data[i] != NULL; i++) free(data[i]);
    free(data);
    data = NULL;
}

/** @brief 状況役の一覧作成
 * @param hupai (Hupai *) 特殊役の情報
 * @return (Yi **) 役の一覧
 */
Yi **getPreHupai(Hupai *hupai){
    // メモリ確保
    Yi** preHupai = allocateMemory(MAX_HUPAI, sizeof(Yi *), true);
    uint8_t count = 0;  // 役の数

    // 役を追加する関数形式マクロ
    #define CREATE_YI(name, fanshu) \
        do{ \
            preHupai[count] = createYi(name, fanshu, 0); \
            (count)++; \
        }while(0)
    
    // 天和, 地和
    if(hupai->tianhu != 0){
        if(hupai->tianhu == 1)      CREATE_YI("天和", -1);
        else if(hupai->tianhu == 2) CREATE_YI("地和", -1);

        // 他の状況役は複合しないので返す
        preHupai[count] = NULL; // 終端をNULLに
        #undef CREATE_YI
        return preHupai;
    }

    // 立直、ダブル立直
    if(hupai->lizhi == 1)      CREATE_YI("立直", 1);
    else if(hupai->lizhi == 2) CREATE_YI("ダブル立直", 2);

    // 一発
    if(hupai->yifa) CREATE_YI("一発", 1);

    // 海底摸月、河底撈魚
    if(hupai->haidi == 1)      CREATE_YI("海底摸月", 1);
    else if(hupai->haidi == 2) CREATE_YI("河底撈魚", 1);

    // 嶺上開花
    if(hupai->lingshang) CREATE_YI("嶺上開花", 1);

    // 槍槓
    if(hupai->qianggang) CREATE_YI("槍槓", 1);

    preHupai[count] = NULL; // 終端をNULLに
    #undef CREATE_YI
    return preHupai;
}

/** @brief 懸賞役の一覧作成
 * @param shoupai (Shoupai *) 特殊役の情報
 * @param baopai (uint8_t *) ドラの情報
 * @param fubaopai (uint8_t *) 裏ドラの情報
 * @return (Yi **) 役の一覧
 */
Yi **getPostHupai(Shoupai *shoupai, uint8_t *baopai, uint8_t *fubaopai){
    Yi **postHupai = allocateMemory(MAX_HUPAI, sizeof(Yi *), true);
    uint8_t count = 0;  // 役の数

    // 赤ドラを5牌に変換
    Shoupai *newShoupai = convertHongpaiToFive(shoupai);

    //--- ドラの計算 ---//
    uint8_t nBaopai = 0;    // ドラの翻数
    for(int i = 0; baopai[i] % 10 != 0; i++){
        // 表示杯から、実際のドラ牌(種類、番号)に変換
        Paizhong baoPaizhong = baopai[i] / 10;
        uint8_t baoVal = (baoPaizhong == Zihpai) ? ((baopai[i] % 10 >= 1 && baopai[i] % 10 <= 4) ? (baopai[i] % 4 + 1) : ((baopai[i] - 4) % 3 + 5)) : (baopai[i] % 10) % 9 + 1;

        // 打牌可能な手牌(自摸含む)
        nBaopai += newShoupai->bingpai[baoPaizhong][baoVal];

        // 副露
        for(int j = 0; newShoupai->fulou[j] != NULL; j++){
            if(newShoupai->fulou[j]->paizhong != baoPaizhong) continue; // 牌の種類が違うとき

            if(newShoupai->fulou[j]->paixing == Shunzi){
                // 順子のとき、value, value + 1, value + 2のいずれかで、ドラ+1
                if(baoVal == newShoupai->fulou[j]->value || baoVal == newShoupai->fulou[j]->value + 1 || baoVal == newShoupai->fulou[j]->value + 2) nBaopai++;
            }else{
                // 順子以外の時、valueで、ドラ+3 or +4
                if(baoVal == newShoupai->fulou[j]->value){
                    switch(newShoupai->fulou[j]->paixing){
                        // 刻子
                        case Kezi:
                            nBaopai += 3;
                            break;

                        // 槓子
                        case Gangzi:
                            nBaopai += 4;
                            break;
                    }
                }
            }
        }
    }

    // 役の追加
    if(nBaopai > 0){
        postHupai[count] = createYi("ドラ", nBaopai, 0);
        count++;
    }
    //--- ドラの計算 ---//
    
    //--- 赤ドラの計算 ---//
    uint8_t nHongpai = 0;    // ドラの翻数
    
    // 打牌可能な手牌(自摸含む)
    nHongpai += shoupai->bingpai[Wanzi][0] + shoupai->bingpai[Tongzi][0] + shoupai->bingpai[Suozi][0];

    // 副露
    for(int j = 0; shoupai->fulou[j] != NULL; j++){
        if(shoupai->fulouHongpai[j]) nHongpai++;
    }

    // 役の追加
    if(nHongpai > 0){
        postHupai[count] = createYi("赤ドラ", nHongpai, 0);
        count++;
    }
    //--- 赤ドラの計算 ---//

    //--- 裏ドラの計算 ---//
    uint8_t nFubaopai = 0;    // ドラの翻数

    for(int i = 0; fubaopai[i] % 10 != 0; i++){
        // 表示杯から、実際のドラ牌(種類、番号)に変換
        Paizhong baoPaizhong = fubaopai[i] / 10;
        uint8_t baoVal = (baoPaizhong == Zihpai) ? ((fubaopai[i] % 10 >= 1 && fubaopai[i] % 10 <= 4) ? (fubaopai[i] % 4 + 1) : ((fubaopai[i] - 4) % 3 + 5)) : (fubaopai[i] % 10) % 9 + 1;

        // 打牌可能な手牌(自摸含む)
        nFubaopai += newShoupai->bingpai[baoPaizhong][baoVal];

        // 副露
        for(int j = 0; newShoupai->fulou[j] != NULL; j++){
            if(newShoupai->fulou[j]->paizhong != baoPaizhong) continue; // 牌の種類が違うとき

            if(newShoupai->fulou[j]->paixing == Shunzi){
                // 順子のとき、value, value + 1, value + 2のいずれかで、ドラ+1
                if(baoVal == newShoupai->fulou[j]->value || baoVal == newShoupai->fulou[j]->value + 1 || baoVal == newShoupai->fulou[j]->value + 2) nBaopai++;
            }else{
                // 順子以外の時、valueで、ドラ+3 or +4
                if(baoVal == newShoupai->fulou[j]->value){
                    switch(newShoupai->fulou[j]->paixing){
                        // 刻子
                        case Kezi:
                            nBaopai += 3;
                            break;

                        // 槓子
                        case Gangzi:
                            nBaopai += 4;
                            break;
                    }
                }
            }
        }
    }

    // 役の追加
    if(nFubaopai > 0){
        postHupai[count] = createYi("裏ドラ", nFubaopai, 0);
        count++;
    }
    //--- 裏ドラの計算 ---//

    postHupai[count] = NULL;    // 終端をNULLに
    free(newShoupai);           // メモリ解放
    return postHupai;
}

/** @brief 符を求める関数
 * @param shoupai (Shoupai *) 手牌
 * @param hulepai (PaiAction *) 和了牌
 * @return (Mianzi **) 面子の情報の配列(七対子ではなかったときNULL)
 */
Hudi *getHudi(Mianzi **mianzi, Fengpai zhuangfeng, Fengpai menfeng){
    //--- 初期設定 ---//
    Hudi *hudi = allocateMemory(1, sizeof(Hudi), false);
    hudi->fu = 20;
    hudi->menqian = true;
    hudi->zimo = true;
    hudi->shunzi = allocateMemory(MAX_MIANZI_COMBINATIONS, sizeof(Mianzi *), true);
    memset(hudi->kezi, 0, sizeof(hudi->kezi));
    hudi->nShunzi = 0;
    hudi->nKezi   = 0;
    hudi->nAnkezi = 0;
    hudi->nGangzi = 0;
    hudi->nZiphai  = 0;
    hudi->nYaojiu = 0;
    hudi->danqi = false;
    hudi->pinghu = false;
    hudi->zhuangfeng = zhuangfeng;
    hudi->menfeng = menfeng;
    //--- 初期設定 ---//
    
    // 面子の数をカウント
    size_t mianziCount = 0;
    while(mianzi[mianziCount] != NULL) mianziCount++;

    // 面子を処理
    for(int i = 0; i < mianziCount; i++){
        Mianzi *p = mianzi[i];  // 処理を行う面子

        if(p->isHulu && p->source != Zimo) hudi->zimo = false;                  // ロン和了の時
        if(p->isFulou) hudi->menqian = false;                                   // 副露している時
        if(isYaojiu(p)) hudi->nYaojiu++;                                        // 幺九牌を含む時
        if(p->paizhong == Zihpai) hudi->nZiphai++;                               // 字牌を含むとき
        if(p->isHulu && p->paixing == Duizi) hudi->danqi++;                     // 単騎待ちのとき

        if(mianziCount != 5) continue;  // 4面子1雀頭形でない場合は以下の処理はスキップ

        switch (p->paixing){
        // 対子(雀頭)
        case Duizi:
            // 字牌の場合
            if(p->paizhong == Zihpai){
                if(p->value == zhuangfeng + 1) hudi->fu += 2;           // 場風のとき
                if(p->value == menfeng + 1) hudi->fu += 2;              // 自風のとき
                if(5 <= p->value && p->value <= 7) hudi->fu += 2;       // 三元牌のとき
            }
            if(hudi->danqi) hudi->fu += 2;  // 単騎待ちの時

            break;
        
        // 刻子、槓子
        case Kezi:
        case Gangzi:
            hudi->nKezi++;
            hudi->kezi[p->paizhong][p->value]++;

            int fu = 2;     //刻子の符を 2 で初期化
            if(isYaojiu(p)) fu *= 2;                                    // 幺九牌のとき
            if(!p->isFulou){ fu *= 2; hudi->nAnkezi++; }                // 暗刻子のとき(槓子含む)
            if(p->paixing == Gangzi){ fu *= 4; hudi->nGangzi++; }       // 槓子のとき

            hudi->fu += fu;
            break;

        // 順子
        case Shunzi:
            hudi->shunzi[hudi->nShunzi] = p;    // シャローコピー
            hudi->nShunzi++;

            // 和了牌を含む面子の時
            if(p->isHulu){
                if(p->value + 1 == p->value + p->index) hudi->fu += 2;                                      // 嵌張待ちのとき
                if(p->value == p->value + p->index || p->value + 2 == p->value + p->index)hudi->fu += 2;    // 辺張待ちのとき
            }
            break;

        default:
            break;
        }
    }

    // 七対子形の場合
    if(mianziCount == 7) hudi->fu = 25;

    // 4面子1雀頭形の場合
    else if(mianziCount == 5){
        hudi->pinghu = (hudi->menqian && hudi->fu == 20);   // 門前で20符なら平和
        // ツモ和了
        if(hudi->zimo){
            if(!hudi->pinghu) hudi->fu += 2;                // 平和でないとき

        // ロン和了
        }else{
            if(hudi->menqian)       hudi->fu += 10;         // 門前のとき
            else if(hudi->fu == 20) hudi->fu = 30;          // 喰い平和のとき
        }
        hudi->fu = ceil(hudi->fu / 10.0) * 10;          // 10点未満は切り上げ
    }

    return hudi;
}

/** @brief 役を求める関数
 * @param flag (TeshuDamanguan) 国士無双、九蓮宝燈かどうか(1:国士無双, 2:国士無双十三面, 3:九蓮宝燈, 4:純正九蓮宝燈)
 * @param mianzi (Mianzi **) 面子の配列
 * @param hudi (Hudi *) 符の情報(符を求める上で作成された情報)
 * @param preHupai (Yi **) 状況役の一覧
 * @return (Yi **) 役の一覧
 */
Yi **getHupai(TeshuDamanguan flag, Mianzi **mianzi, Hudi *hudi, Yi **preHupai){
    // 役満の初期値 状況役に役満(天和、地和)が含まれている場合はそれを設定, ない場合は初期化。
    Yi **damanguan = (preHupai[0] != NULL && preHupai[0]->fanshu < 0) ? copyYiArray(preHupai) : allocateMemory(MAX_HUPAI, sizeof(Yi *), true);
    size_t damanguanCount = (preHupai[0] != NULL && preHupai[0]->fanshu < 0) ? 1 : 0;

    // 面子の数
    size_t mianziCount = 0;
    while(mianzi != NULL && mianzi[mianziCount] != NULL) mianziCount++;

    //--- 関数形式マクロ ---///
    /** @brief 渡された引数の最初の3つをcreateYiに渡す
     * @param ONE_   createYiの第1引数に渡す値。 char *型。
     * @param TWO_   createYiの第2引数に渡す値。 uint8_t型。
     * @param THREE_ createYiの第3引数に渡す値。 Sijia型(2bit)。
     */
    #define SELECTER(ONE_, TWO_, THREE_, ...) createYi(ONE_, TWO_, THREE_)

    /** @brief yiとyiCountを受け取り、渡された引数(2 or 3つ)をcreateYiに渡して、yi[yiCount]に格納し、yiCountをインクリメントする
     * @param yi 役の配列。 Yi **型
     * @param yiCount 役の数(配列の番号) uint8_t型
     * @param __VA_ARGS__ (残りの引数) createYi に渡す任意の引数。2つか3つで、(char *, uint8_t, (Sijia))
     * @details 渡された引数が2つの場合、createYi(ONE_, TWO_, 0)を呼び出し、渡された引数が3つの場合、createYi(ONE_, TWO_, THREE_)を呼び出す。
     */
    #define CREATE_NEW_YI(yi, yiCount, ...) \
        do{ \
            yi[*yiCount] = SELECTER(__VA_ARGS__, 0); \
            (*yiCount)++; \
        }while(0)

    /** @brief damanguanに役を追加する
     * @param __VA_ARGS__ 役の情報
     */
    #define CREATE_YI(...) CREATE_NEW_YI(damanguan, &damanguanCount, __VA_ARGS__)
    //--- 関数形式マクロ ---///

    //--- 役満 ---//
    // 国士無双, 九蓮宝燈
    if(mianziCount == 0 || hudi == NULL){
        switch(flag){
            case Guoshi:           CREATE_YI("国士無双", -1); break;
            case GuoshiShiSanMin:  CREATE_YI("国士無双十三面", -2); break;
            case Jiulian:          CREATE_YI("九蓮宝燈", -1); break;
            case ChunzhengJiulian: CREATE_YI("純正九蓮宝燈", -2); break;
            default: return NULL;
        }

        // 他の役満と複合しないので返す
        return damanguan;
    }

    // 四暗刻
    if(hudi->nAnkezi == 4){
        // 単騎待ちのとき
        if(hudi->danqi) CREATE_YI("四暗刻単騎", -2);
        else            CREATE_YI("四暗刻", -1);
    }

    // 大三元
    if(hudi->kezi[Zihpai][5] + hudi->kezi[Zihpai][6] + hudi->kezi[Zihpai][7] == 3){
        Sijia baojia = 0;

        // パオの確認
        for(int i = 3; 0 <= i; i--){
            if(!mianzi[i]->isFulou){
                // 副露していない三元牌の面子があるとき
                if(mianzi[i]->paizhong == Zihpai && (5 <= mianzi[i]->value && mianzi[i]->value <= 7)){
                    baojia = 0;
                    break;
                }
                continue;
            } 

            // 最後に副露した三元牌のとき(最後から確認して最初に確認した三元牌のとき)
            if(mianzi[i]->paizhong == Zihpai && (5 <= mianzi[i]->value && mianzi[i]->value <= 7) && baojia == 0){
                baojia = mianzi[i]->source;
            }
        }

        CREATE_YI("大三元", -1, baojia);
    }

    // 四喜和
    if(hudi->kezi[Zihpai][1] + hudi->kezi[Zihpai][2] + hudi->kezi[Zihpai][3] + hudi->kezi[Zihpai][4] == 4){
        Sijia baojia = 0;

        // パオの確認
        for(int i = 3; 0 <= i; i--){
            if(!mianzi[i]->isFulou){
                // 副露していない風牌の面子があるとき
                if(mianzi[i]->paizhong == Zihpai && (1 <= mianzi[i]->value && mianzi[i]->value <= 4)){
                    baojia = 0;
                    break;
                }
                continue;
            } 

            // 最後に副露した風牌のとき(最後から確認して最初に確認した風牌のとき)
            if(mianzi[i]->paizhong == Zihpai && (1 <= mianzi[i]->value && mianzi[i]->value <= 4)){
                baojia = mianzi[i]->source;
            }
        }

        CREATE_YI("大四喜", -2, baojia);
    }else if(hudi->kezi[Zihpai][1] + hudi->kezi[Zihpai][2] + hudi->kezi[Zihpai][3] + hudi->kezi[Zihpai][4] == 3 && mianzi[0]->paizhong == Zihpai && 1 <= mianzi[0]->value && mianzi[0]->value <= 4){
        CREATE_YI("小四喜", -1);
    }

    // 字一色
    if(hudi->nZiphai == mianziCount) CREATE_YI("字一色", -1);

    // 緑一色
    bool isLvyise = true;
    for(int i = 4; 0 <= i; i--){
        // 萬子、筒子が含まれている
        if(mianzi[i]->paizhong == Wanzi || mianzi[i]->paizhong == Tongzi){
            isLvyise = false;
            break;
        }

        // 發以外の字牌が含まれている
        if(mianzi[i]->paizhong == Zihpai && mianzi[i]->value != 6){
            isLvyise = false;
            break;
        }

        // 一五七九索が含まれている
        if(mianzi[i]->paizhong == Suozi && (mianzi[i]->value == 1 || mianzi[i]->value == 5 || mianzi[i]->value == 7 || mianzi[i]->value == 9)){
            isLvyise = false;
            break;
        }
    }
    if(isLvyise) CREATE_YI("緑一色", -1);

    // 清老頭
    if(hudi->nKezi == 4 && hudi->nYaojiu == 5 && hudi->nZiphai == 0) CREATE_YI("清老頭", -1);

    //四槓子
    if(hudi->nGangzi == 4) CREATE_YI("四槓子", -1);
    //--- 役満 ---//

    // 役満がある場合は終了
    if(damanguanCount > 0) return damanguan;


    Yi **hupai = copyYiArray(preHupai);
    uint8_t hupaiCount = 0;
    while(hupai[hupaiCount] != NULL) hupaiCount++;

    //--- 関数形式マクロ ---//
    #undef CREATE_YI // damanguanへのマクロを廃止

    /** @brief hupaiに役を追加する
     * @param __VA_ARGS__ 役の情報
     */
    #define CREATE_YI(...) CREATE_NEW_YI(hupai, &hupaiCount, __VA_ARGS__)
    //--- 関数形式マクロ ---//

    //--- 一般役 ---//
    // 門前清自摸和
    if(hudi->menqian && hudi->zimo) CREATE_YI("門前清自摸和", 1);

    // 翻牌
    char str[32];
    char *fengHanzi[] = {"東", "南", "西", "北"};
    // 場風の役
    if(hudi->kezi[Zihpai][hudi->zhuangfeng + 1] > 0){
        sprintf(str, "場風 : %s", fengHanzi[hudi->zhuangfeng]);
        CREATE_YI(str, 1);
    }
    // 自風の役
    if(hudi->kezi[Zihpai][hudi->menfeng + 1] > 0){
        sprintf(str, "自風 : %s", fengHanzi[hudi->menfeng]);
        CREATE_YI(str, 1);
    }
    // 翻牌 (白、發、中)
    if(hudi->kezi[Zihpai][5] > 0) CREATE_YI("翻牌 白", 1);
    if(hudi->kezi[Zihpai][6] > 0) CREATE_YI("翻牌 發", 1);
    if(hudi->kezi[Zihpai][7] > 0) CREATE_YI("翻牌 中", 1);

    // 平和
    if(hudi->pinghu) CREATE_YI("平和", 1);

    // 断幺九
    if(hudi->nYaojiu == 0 && hudi->nZiphai == 0) CREATE_YI("断幺九", 1);

    // 一盃口、二盃口
    if(hudi->menqian && hudi->nShunzi > 1){
        uint8_t beikou = 0;
        bool *check = allocateMemory(hudi->nShunzi - 1, sizeof(bool), true);

        for(int i = 0; i < hudi->nShunzi; i++){
            if(i != 0 && check[i - 1]) continue;    // 既にチェックされた順子はスキップ

            for(int j = i + 1; j < hudi->nShunzi; j++){
                // 同じ順子を見つけた時
                if(isSameMianzi(hudi->shunzi[i], hudi->shunzi[j])){
                    beikou++;
                    check[j - 1] = true;    // 既にチェックされたものとしてチェック
                }
            } 
        }
        free(check);
        if(beikou == 1) CREATE_YI("一盃口", 3);
        if(beikou == 2) CREATE_YI("二盃口", 3);
    }

    // 三色同順
    if(hudi->nShunzi > 2){
        bool paizhong[3];   // 萬子、筒子、索子の順番で管理

        for(int i = 0; i < hudi->nShunzi - 2; i++){
            memset(paizhong, 0, sizeof(bool) * 3);      // 牌種の情報リセット
            paizhong[hudi->shunzi[i]->paizhong] = true; // i番目の順子の牌種をマーク

            // i番目のより後の順子から同じ番号のものを探し、あったらマーク
            for(int j = i + 1; j < hudi->nShunzi; j++){
                if(hudi->shunzi[i]->value == hudi->shunzi[j]->value){
                    paizhong[hudi->shunzi[j]->paizhong] = true; // j番目の順子の牌種をマーク
                }
            }

            // 3種同じ番号があったら
            if(paizhong[Wanzi] && paizhong[Tongzi] && paizhong[Suozi]){
                CREATE_YI("三色同順", hudi->menqian ? 2 : 1);
                break;
            }
        }
    }

    // 一気通貫
    if(hudi->nShunzi > 2){
        bool yiq[3][3] = {false};   // 3つの牌種、それぞれの1,4,7が揃っているか

        for(int i = 0; i < hudi->nShunzi; i++){
            uint8_t value = hudi->shunzi[i]->value;
            // 1, 4, 7の牌のときマーク
            if(value == 1 || value == 4 || value == 7){
                yiq[hudi->shunzi[i]->paizhong][(value - 1) / 3] = true;
            }
        }

        // 1, 4, 7がすべてそろっている牌種があるとき
        for(int i = 0; i < 3; i++){
            if(yiq[i][0] && yiq[i][1] && yiq[i][2]){
                CREATE_YI("一気通貫", hudi->menqian ? 2 : 1);
                break;
            }
        }
    }

    // 混全帯幺九
    if(hudi->nYaojiu == 5 && hudi->nShunzi > 0 && hudi->nZiphai > 0) CREATE_YI("混全帯幺九", hudi->menqian ? 2 : 1);

    // 七対子
    if(mianziCount == 7) CREATE_YI("七対子", 2);

    // 対々和
    if(hudi->nKezi == 4) CREATE_YI("対々和", 2);

    // 三暗刻
    if(hudi->nKezi == 3) CREATE_YI("三暗刻", 2);

    // 三槓子
    if(hudi->nGangzi == 3) CREATE_YI("三槓子", 2);

    // 混老頭
    if(hudi->nYaojiu == mianziCount && hudi->nShunzi == 0 && hudi->nZiphai > 0) CREATE_YI("混老頭", 2);

    // 小三元
    if(hudi->kezi[Zihpai][5] + hudi->kezi[Zihpai][6] + hudi->kezi[Zihpai][7] == 2 && mianzi[0]->paizhong == Zihpai && 5 <= mianzi[0]->value && mianzi[0]->value <= 7) CREATE_YI("小三元", 2);

    // 混一色, 清一色
    Paizhong paizhong = Zihpai;  // 初期値は字牌(字牌は処理にかかわらない)
    bool isHunYise = true;       // 字牌を除いて1つの牌種のみであるか

    for(int i = 0; i < mianziCount; i++){
        // 保存されている牌種が字牌のとき、面子の牌種を保存
        if(paizhong == Zihpai){
            paizhong = mianzi[i]->paizhong;
        }else{
            // 保存されている牌種が面子の牌種(字牌以外)と違うとき
            if(mianzi[i]->paizhong != Zihpai && mianzi[i]->paizhong != paizhong){
                isHunYise = false;
                break;
            }
        }
    }
    // 結果判定
    if(isHunYise && hudi->nZiphai > 0)  CREATE_YI("混一色", hudi->menqian ? 3 : 2);
    if(isHunYise && hudi->nZiphai == 0) CREATE_YI("清一色", hudi->menqian ? 6 : 5);

    // 純全帯幺九
    if(hudi->nYaojiu == 5 && hudi->nShunzi > 0 && hudi->nZiphai == 0) CREATE_YI("純全帯幺九", hudi->menqian ? 3 : 2);

    return hupai;
}

/** @brief 役、面子を求め、点数を計算する関数
 * @param shoupai (Shoupai *) 手牌の情報
 * @param juInfo (JuInfo *) 局の情報
 * @return (Hule *) 点数や役、符などの情報
 */
Hule *hule(Shoupai *shoupai, JuInfo *juInfo){
    Hule *max = allocateMemory(1, sizeof(Hule), true);

    Shoupai *subShoupai = convertHongpaiToFive(shoupai);                        // 赤ドラを5に変換した手牌
    Yi **preHupai = getPreHupai(juInfo->hupai);                                 // 状況役の一覧
    Yi **postHupai = getPostHupai(shoupai, juInfo->baopai, juInfo->fubaopai);   // 懸賞役の一覧
    TeshuDamanguan damanguanFlag;                                               // 特殊形の面子による役満のフラグ
    Mianzi ***mianzi = huleMianzi(&damanguanFlag, subShoupai);                  // 面子の組み合わせの配列
    free(subShoupai);   // 解放

    size_t count = 0;
    while(mianzi[count] != NULL){
        Hudi *hudi = getHudi(mianzi[count], juInfo->zhuangfeng, juInfo->menfeng);   // 符計算
        Yi **hupai = getHupai(damanguanFlag, mianzi[count], hudi, preHupai);        // 役計算

        if(hupai == NULL || hupai[0] == NULL) { count++; continue; } // hupaiがないとき、スキップ

        // hupaiの数
        size_t hupaiCount = 0;
        while(hupai[hupaiCount] != NULL) hupaiCount++;

        // 初期化
        uint8_t fu = hudi->fu;                                  // 符
        uint8_t fanshu = 0, damanguan = 0; uint16_t defen = 0;  // 点数に関する変数
        Fengpai baojia2 = 0; uint16_t defen2 = 0;               // パオに関する変数

        // 役満の時
        if(hupai[0]->fanshu < 0){
            for(int i = 0; i < hupaiCount; i++){
                damanguan += abs(hupai[i]->fanshu);
                
                // パオがあるとき
                if(hupai[i]->baojia != 0){
                    baojia2 = (juInfo->menfeng + hupai[i]->baojia) % 4;         // パオ対象
                    defen2 = 8000 * abs(hupai[i]->fanshu);                      // パオ対象の基本点
                }
            }

            defen = 8000 * damanguan;   // パオを含む全体の基本点
        }else{
            // 役満以外の場合
            concatYiArray(hupai, postHupai);    // 懸賞役をシャローコピー
            while(hupai[hupaiCount] != NULL) hupaiCount++; // hupaiの数更新

            for(int i = 0; i < hupaiCount; i++) fanshu += hupai[i]->fanshu; // 翻数

            // 基本点計算
            if     (fanshu >= 13) defen = 8000; // 役満
            else if(fanshu >= 11) defen = 6000; // 三倍満
            else if(fanshu >=  8) defen = 4000; // 倍満
            else if(fanshu >=  6) defen = 3000; // 跳満
            else{
                defen = fu * 4;                                 // 符を4倍
                for(int i = 0; i < fanshu; i++) defen *= 2;     // 翻数分2倍
                if(defen >= 2000) defen = 2000;                 // Max2000点(満貫)
            }
        }

        int32_t fenpei[4] = {0};    // 収入と負担額 fenpei[Fengpai]
        
        // パオがあった場合
        if(defen2 > 0){
            if(shoupai->hulepai->paiSource != Zimo) defen2 /= 2;   // ロン和了の場合は放銃者と折半
            defen  -= defen2;                                       // 基本点からパオ分を減算
            defen2 *= juInfo->menfeng == Dong ? 6 : 4;              // パオ分の負担額

            fenpei[juInfo->menfeng] =  defen2;                      // 和了者の収入を加算
            fenpei[baojia2]         = -defen2;                      // パオ対象の負担額を減算 
        }

        if(shoupai->hulepai->paiSource != Zimo || defen == 0){
            // ロン和了もしくはパオ1人払いの場合
            Fengpai baojia = defen == 0 ? baojia2 : (juInfo->menfeng + shoupai->hulepai->paiSource) % 4; // パオ1人払いは放銃者扱い

            //--- 負担額の計算(100点未満切り上げ) ---//
            defen = ceil(defen * (juInfo->menfeng == Dong ? 6 : 4) / 100.0) * 100;
            fenpei[juInfo->menfeng] +=  defen + juInfo->changbang * 300 + juInfo->lizhibang * 1000; // 和了者の収入を加算(供託含む)
            fenpei[baojia]          += -defen - juInfo->changbang * 300;                            // 放銃者の負担額を減算(供託含む)
        }else{
            // ツモ和了の場合  
            uint16_t zhuangjia = ceil(defen * 2 / 100.0) * 100;  // 親の負担額
            uint16_t sanjia    = ceil(defen     / 100.0) * 100;  // 子の負担額

            if(juInfo->menfeng == Dong){
                // 親和了のとき
                defen = zhuangjia * 3;  // 和了打点は 親の負担額 x 3
                for(int j = 0; j < 4; j++){
                    if(j == juInfo->menfeng) fenpei[j] = defen + juInfo->changbang * 300 + juInfo->lizhibang * 1000;    // 和了者の収入を加算(供託含む)
                    else                     fenpei[j] = -zhuangjia - juInfo->changbang * 100;                          // 負担者の負担額を減算(供託含む)
                }
            }else{
                // 子和了のとき
                defen = zhuangjia + sanjia * 2;  // 和了打点は 親の負担額 + 子の負担額 x 2
                for(int j = 0; j < 4; j++){
                    if(j == juInfo->menfeng) fenpei[j] = defen + juInfo->changbang * 300 + juInfo->lizhibang * 1000;    // 和了者の収入を加算(供託含む)
                    else if(j == Dong)       fenpei[j] = -zhuangjia - juInfo->changbang * 100;                          // 親の負担額を減算(供託含む)
                    else                     fenpei[j] = -sanjia    - juInfo->changbang * 100;                          // 子の負担額を減算(供託含む)
                }
            }
        }

        // 得られた和了点の最大値を解とする
        if(defen + defen2 > max->defen ? true : fanshu > max->fanshu){
            // 元々のhupaiを解放
            freeYiArray(max->hupai);

            // 値を更新
            max->hupai     = hupai;
            max->fu        = fu;
            max->fanshu    = fanshu;
            max->damanguan = damanguan;
            max->defen     = defen + defen2;
            memcpy(max->fenpei, fenpei, sizeof(int32_t) * 4);

            // hupaiをNULLに
            hupai = NULL;
        }

        //--- 解放 ---//
        // 順子の面子構成はシャローコピーなので変数のみ解放
        free(hudi);

        // NULLでなければ解放
        if(hupai != NULL) freeYiArray(hupai);

        count++;
    }

    //--- 解放 ---//
    // すべてディープコピーしたのですべて解放
    freeYiArray(preHupai);

    // 役満のときは、すべてfree, 通常の時は、シャローコピーなので変数のみ解放
    if(max->damanguan > 0) freeYiArray(postHupai);
    else free(postHupai);

    // 副露面子はディープコピーしたので全て解放
    freeAllMianziArray(mianzi);

    return max;
}

/*
int main(){
    // --- 手牌の初期化 --- //
    Mianzi **fulou = (Mianzi **)malloc(2 * sizeof(Mianzi *));
    fulou[0] = allocateMemory(2, sizeof(Mianzi), false);
    fulou[0]->paixing = Kezi;
    fulou[0]->paizhong = Wanzi;
    fulou[0]->value = 1;
    fulou[0]->isFulou = true;
    fulou[0]->isHulu = false;
    fulou[0]->index = 0;
    fulou[0]->source = DuiMian;
    fulou[1] = NULL;

    Pai hulepai = { Wanzi, 1, DuiMian };
    bool fulouHongpai[2] = {false, false};

    Shoupai shoupai = {
        {{0, 0, 3, 3, 3, 2, 0, 0, 0, 0},  // 萬子
         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 筒子
         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 索子
         {0, 0, 0, 0, 0, 0, 0, 0}},       // 字牌
        fulou,
        fulouHongpai,
        &hulepai
    };

    Hupai hupai = { 0, false, false, false, 0, 0 };
    JuInfo juInfo = {Dong, Dong, &hupai, {1, 0, 0, 0, 0}, {21, 0, 0, 0, 0}, 1, 1};

    Hule *huleInfo = hule(&shoupai, &juInfo);

    int i = 0;
    while(huleInfo->hupai[i] != NULL){
        printf("%s, %d\n", huleInfo->hupai[i]->name, (int)huleInfo->hupai[i]->fanshu);
        i++;
    }
    printf("\n");

    printf("fu : %d\n", (int)huleInfo->fu);
    printf("fanshu : %d\n", (int)huleInfo->fanshu);
    printf("damanguan : %d\n", (int)huleInfo->damanguan);
    printf("defen : %d\n", (int)huleInfo->defen);
    printf("fenpei : %d, %d, %d, %d\n", (int)huleInfo->fenpei[0], (int)huleInfo->fenpei[1], (int)huleInfo->fenpei[2], (int)huleInfo->fenpei[3]);

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

    return 0;
}
*/