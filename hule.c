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
Shoupai *get0to5Shoupai(const Shoupai *shoupai){
    Shoupai *newShoupai = allocateMemory(1, sizeof(Shoupai), false);
    uint8_t bingpai[4][10];
    memcpy(bingpai, shoupai->bingpai, sizeof(uint8_t) * 4 * 10);
    newShoupai->fulou = shoupai->fulou;
    newShoupai->fulouHongpai = shoupai->fulouHongpai;
    newShoupai->drawnPai = shoupai->drawnPai;

    for(int i = 0; i < 3; i++){
        bingpai[i][5] += bingpai[i][0];
        bingpai[i][0] = 0;
    }

    return newShoupai;
}

/** @brief 幺九牌を含むかどうかを確認する関数
 * @param mianzi (Mianzi) 確認する面子
 * @return (bool) Yes / No
 */
bool isYaojiu(const Mianzi *mianzi){
    // 字牌のとき
    if(mianzi->paizhong == Zihpai) return false;

    // 字牌以外の時
    switch(mianzi->paixing){
    // 順子のとき
    case Shunzi:
        // 1, 2, 3->1 or 7, 8, 9->7のときのみ
        if(mianzi->value == 1 || mianzi->value == 7) return true;
        else return false;

    // 刻子、槓子、対子のとき
    case Kezi:
    case Gangzi:
    case Duizi:
        if(mianzi->value == 1 || mianzi->value == 9) return true;
        else return false;

    default:
        return false;
    }
}

/** @brief 役の情報が同じかを確認する関数
 * @param data1 (Mianzi*) 比較対象の面子1
 * @param data2 (Mianzi*) 比較対象の面子2
 * @return (bool) 同じ面子ならtrue、異なればfalse
 */
bool isSameMianzi(const Mianzi* data1, const Mianzi* data2) {
    return (data1->paixing == data2->paixing) && (data1->paizhong == data2->paizhong) && (data1->value == data2->value);
}

/** @brief 役を新たに作成する関数
 * @param name (char *) 役の名前
 * @param fanshu (int) 翻数
 * @param baojia (SiJia) パオがある場合、四家のうちどこか(0...パオなし)
 * @return (Yi *) 役の情報が入った型
 */
Yi *createYi(const char *name, const uint8_t fanshu, const Sijia baojia){
    Yi *yi = allocateMemory(1, sizeof(Yi), false);

    yi->name = strdup(name);
    yi->fanshu = fanshu;
    yi->baojia = baojia;

    return yi;
}

/** @brief 役のリストをコピーする関数
 * @param yi (Yi *) コピー元の役
 * @return (Yi *) 役の情報が入った型
 */
Yi **copyYiArray(const Yi **data){
    size_t count = 0;
    while(data[count] != NULL) count++;

    Yi **newData = allocateMemory(MAX_HUPAI, sizeof(Yi *), false);
    for(int i = 0; i < count; i++){
        newData[i] = createYi(data[i]->name, data[i]->fanshu, data[i]->baojia);
    }

    return newData;
}

/** @brief 役のリストを結合する関数
 * @param 
 * @param
 */
void concatYiArray(Yi **data1, const Yi **data2){
    size_t data1Count = 0, data2Count = 0;
    while(data1[data1Count] != NULL) data1Count++;
    while(data2[data2Count] != NULL) data2Count++;

    for(int i = 0; i < data2Count; i++){
        data1[data1Count + i] = data2[i];
    }
}

/** @brief 状況役の一覧作成
 * @param hupai (Hupai *) 特殊役の情報
 * @return (Yi **) 役の一覧
 */
Yi **getPreHupai(const Hupai *hupai){
    Yi** preHupai = allocateMemory(MAX_HUPAI, sizeof(Yi *), true);
    uint8_t count = 0;  // 役の数

    // マクロ
    #define CREATE_YI(name, fanshu) \
        do { \
            preHupai[count] = createYi(name, fanshu, 0); \
            (count)++; \
        } while(0)
    
    // 天和
    if(hupai->tianhu == 1){
        CREATE_YI("天和", -1);
        return preHupai;
    }

    // 地和
    if(hupai->tianhu == 2){
        CREATE_YI("地和", -1);
        return preHupai;
    }

    // 立直
    if(hupai->lizhi == 1) CREATE_YI("立直", 1);

    // ダブル立直
    if(hupai->lizhi == 2) CREATE_YI("ダブル立直", 2);

    // 一発
    if(hupai->yifa) CREATE_YI("一発", 1);

    // 海底摸月
    if(hupai->haidi == 1) CREATE_YI("海底摸月", 1);

    // 河底撈魚
    if(hupai->haidi == 2) CREATE_YI("河底撈魚", 1);

    // 嶺上開花
    if(hupai->lingshang) CREATE_YI("嶺上開花", 1);

    // 槍槓
    if(hupai->qianggang) CREATE_YI("槍槓", 1);

    #undef CREATE_YI
    return preHupai;
}

/** @brief 懸賞役の一覧作成
 * @param hupai (Hupai *) 特殊役の情報
 * @return (Yi **) 役の一覧
 */
Yi **getPostHupai(const Shoupai *shoupai, const uint8_t *baopai, const uint8_t *fubaopai){
    Yi **postHupai = allocateMemory(MAX_HUPAI, sizeof(Yi *), true);
    uint8_t count = 0;  // 役の数

    Shoupai *newShoupai = get0to5Shoupai(shoupai);

    //--- ドラの計算 ---//
    uint8_t nBaopai = 0;    // ドラの翻数
    for(int i = 0; baopai[i] % 10 != 0; i++){
        // 打牌可能な手牌(自摸含む)
        nBaopai += newShoupai->bingpai[baopai[i] / 10][baopai[i] % 10];

        // 副露
        for(int j = 0; newShoupai->fulou[j] != NULL; j++){
            uint8_t fulouVal = newShoupai->fulou[j]->paizhong * 10 + newShoupai->fulou[j]->value;

            if(newShoupai->fulou[j]->paixing == Shunzi){
                if(baopai[i] == fulouVal || baopai[i] == fulouVal + 1 || baopai[i] == fulouVal + 2) nBaopai++;
            }else{
                if(baopai[i] == fulouVal){
                    switch(newShoupai->fulou[j]->paixing){
                        case Kezi:
                            nBaopai += 3;
                            break;

                        case Gangzi:
                            nBaopai += 4;
                            break;
                    }
                }
            }
        }
    }
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
        if(shoupai->fulouHongpai) nHongpai++;
    }

    if(nHongpai > 0){
        postHupai[count] = createYi("赤ドラ", nHongpai, 0);
        count++;
    }
    //--- 赤ドラの計算 ---//

    //--- 裏ドラの計算 ---//
    uint8_t nFubaopai = 0;    // ドラの翻数

    for(int i = 0; fubaopai[i] % 10 != 0; i++){
        // 打牌可能な手牌(自摸含む)
        nFubaopai += newShoupai->bingpai[fubaopai[i] / 10][fubaopai[i] % 10];

        // 副露
        for(int j = 0; newShoupai->fulou[j] != NULL; j++){
            uint8_t fulouVal = newShoupai->fulou[j]->paizhong * 10 + newShoupai->fulou[j]->value;

            if(newShoupai->fulou[j]->paixing == Shunzi){
                if(fubaopai[i] == fulouVal || fubaopai[i] == fulouVal + 1 || fubaopai[i] == fulouVal + 2) nFubaopai++;
            }else{
                if(fubaopai[i] == fulouVal){
                    switch(newShoupai->fulou[j]->paixing){
                        case Kezi:
                            nFubaopai += 3;
                            break;

                        case Gangzi:
                            nFubaopai += 4;
                            break;
                    }
                }
            }
        }
    }
    if(nFubaopai > 0){
        postHupai[count] = createYi("裏ドラ", nFubaopai, 0);
        count++;
    }
    //--- 裏ドラの計算 ---//

    return postHupai;
}

/** @brief 符を求めるプログラム
 * @param shoupai (Shoupai *) 手牌
 * @param hulepai (PaiAction *) 和了牌
 * @return (Mianzi **) 面子の情報の配列(七対子ではなかったときNULL)
 */
Hudi *getHudi(const Mianzi **mianzi, const Fengpai zhuangfeng, const Fengpai menfeng){
    // 初期設定
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
    hudi->nZipai  = 0;
    hudi->nYaojiu = 0;
    hudi->danqi = false;
    hudi->pinghu = false;
    hudi->zhuangfeng = zhuangfeng;
    hudi->menfeng = menfeng;
    
    size_t mianziCount = 0;
    while(mianzi[mianziCount] != NULL) mianziCount++;

    for(int i = 0; i < mianziCount; i++){
        Mianzi *p = mianzi[i];  // 処理を行う面子

        if(p->isHulu && p->source != Zimo) hudi->zimo = false;                  // ロン和了の時
        if(p->isFulou) hudi->menqian = false;                                   // 副露している時
        if(isYaojiu(p)) hudi->nYaojiu++;                                        // 幺九牌を含む時
        if(p->paizhong == Zihpai) hudi->nZipai++;                               // 字牌を含むとき
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
            if(p->paixing == Gangzi){ fu *= 4; hudi->nGangzi++; }    // 槓子のとき

            hudi->fu += fu;
            break;

        // 順子
        case Shunzi:
            hudi->shunzi[hudi->nShunzi] = p;
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

/** @brief 役を求めるプログラム
 * @param flag (TeshuDamanguan) 国士無双、九蓮宝燈かどうか(1:国士無双, 2:国士無双十三面, 3:九蓮宝燈, 4:純正九蓮宝燈)
 * @param shoupai (Shoupai *) 手牌
 * @param hulepai (PaiAction *) 和了牌
 * @return (Mianzi **) 面子の情報の配列(七対子ではなかったときNULL)
 */
Yi **getHupai(const TeshuDamanguan flag, const Mianzi **mianzi, const Hudi *hudi, const Yi **preHupai){
    // 役満の初期値を設定する。状況役に役満(天和、地和)が含まれている場合はそれを設定、ない場合は初期化。
    Yi **damanguan = (preHupai[0] != NULL && preHupai[0]->fanshu < 0) ? copyYiArray(preHupai) : allocateMemory(MAX_HUPAI, sizeof(Yi *), true);
    size_t damanguanCount = (preHupai[0] != NULL && preHupai[0]->fanshu < 0) ? 1 : 0;

    size_t mianziCount = 0;
    while(mianzi != NULL && mianzi[mianziCount] != NULL) mianziCount++;

    // マクロ
    #define SELECTER(ONE_, TWO_, THREE_, ...) createYi(ONE_, TWO_, THREE_)
    #define CREATE_NEW_YI(yi, yiCount, ...) \
        do { \
            yi[*yiCount] = SELECTER(__VA_ARGS__, 0); \
            (*yiCount)++; \
        } while(0)
    #define CREATE_YI(...) CREATE_NEW_YI(damanguan, &damanguanCount, __VA_ARGS__)

    //--- 役満 ---//
    // 国士無双, 九蓮宝燈
    if(mianziCount == 0 || hudi == NULL){
        switch(flag){
            case Guoshi:
                CREATE_YI("国士無双", -1);
                break;

            case GuoshiShiSanMin:
                CREATE_YI("国士無双十三面", -2);
                break;

            case Jiulian:
                CREATE_YI("九蓮宝燈", -1);
                break;

            case ChunzhengJiulian:
                CREATE_YI("純正九蓮宝燈", -2);
                break;

            default:
                return NULL;
        }

        // 他の役満と複合しないのでreturn
        return damanguan;
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
    if(hudi->nZipai == mianziCount) CREATE_YI("字一色", -1);

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
    if(hudi->nKezi == 4 && hudi->nYaojiu == 5 && hudi->nZipai == 0) CREATE_YI("清老頭", -1);

    //四槓子
    if(hudi->nGangzi == 4) CREATE_YI("四槓子", -1);
    //--- 役満 ---//

    // 役満がある場合は終了
    if(damanguanCount > 0) return damanguan;


    Yi **hupai = copyYiArray(preHupai);
    uint8_t hupaiCount = 0;
    while(hupai[hupaiCount] != NULL) hupaiCount++;

    // マクロ
    #undef CREATE_YI
    #define CREATE_YI(...) CREATE_NEW_YI(hupai, &hupaiCount, __VA_ARGS__)

    //--- 一般役 ---//
    // 門前清自摸和
    if(hudi->menqian && hudi->zimo) CREATE_YI("門前清自摸和", 1);

    // 翻牌
    char *str;
    char *fengHanzi[] = {"東", "南", "西", "北"};
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
    if(hudi->menqian && hudi->nShunzi > 1){
        uint8_t beikou = 0;
        bool *check = allocateMemory(hudi->nShunzi - 1, sizeof(bool), true);

        for(int i = 0; i < hudi->nShunzi; i++){
            if(i != 0 && check[i - 1]) continue;

            for(int j = i + 1; j < hudi->nShunzi; j++){
                if(isSameMianzi(hudi->shunzi[i], hudi->shunzi[j])){
                    beikou++;
                    check[j - 1] = true;
                }
            } 
        }

        free(check);
        if(beikou == 1) CREATE_YI("一盃口", 3);
        if(beikou == 2) CREATE_YI("二盃口", 3);
    }

    // 三色同順
    if(hudi->nShunzi > 2){
        for(int i = 0; i < hudi->nShunzi - 2; i++){
            bool *paizhong = allocateMemory(3, sizeof(bool), true);
            paizhong[hudi->shunzi[i]->paizhong] = true;

            for(int j = i + 1; j < hudi->nShunzi; j++){
                if(hudi->shunzi[i]->value == hudi->shunzi[j]->value){
                    paizhong[hudi->shunzi[i]->paizhong] = true;
                }
            }

            if(paizhong[Wanzi] && paizhong[Tongzi] && paizhong[Suozi]){
                free(paizhong);
                CREATE_YI("三色同順", hudi->menqian ? 2 : 1);
                break;
            }
        }
    }

    // 一気通貫
    if(hudi->nShunzi > 2){
        bool yiq[3][3] = {false};

        for(int i = 0; i < hudi->nShunzi; i++){
            uint8_t value = hudi->shunzi[i]->value;
            if(value == 1 || value == 4 || value == 7){
                yiq[hudi->shunzi[i]->paizhong][(value - 1) / 3] = true;
            }
        }

        for(int i = 0; i < 3; i++){
            if(yiq[i][0] && yiq[i][1] && yiq[i][2]){
                CREATE_YI("一気通貫", hudi->menqian ? 2 : 1);
                break;
            }
        }
    }

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
    if(hudi->kezi[Zihpai][5] + hudi->kezi[Zihpai][6] + hudi->kezi[Zihpai][7] == 2 && mianzi[0]->paizhong == Zihpai && 5 <= mianzi[0]->value && mianzi[0]->value <= 7) CREATE_YI("小三元", 2);

    // 混一色, 清一色
    Paizhong paizhong = 3;
    bool isHunYise = true;
    for(int i = 0; i < mianziCount; i++){
        if(paizhong == 3){
            paizhong = mianzi[i]->paizhong;
        }else{
            if(mianzi[i]->paizhong != Zihpai && mianzi[i]->paizhong != paizhong){
                isHunYise = false;
                break;
            }
        }
    }
    if(isHunYise && hudi->nZipai > 0) CREATE_YI("混一色", hudi->menqian ? 3 : 2);
    if(isHunYise && hudi->nZipai == 0) CREATE_YI("清一色", hudi->menqian ? 6 : 5);

    // 純全帯幺九
    if(hudi->nYaojiu == 5 && hudi->nShunzi > 0 && hudi->nZipai == 0)
        CREATE_YI("純全帯幺九", hudi->menqian ? 3 : 2);

    return hupai;
}

Hule *hule(const Shoupai *shoupai, const JuInfo *juInfo){
    Hule *max = allocateMemory(1, sizeof(Hule), true);

    Yi **preHupai = getPreHupai(juInfo->hupai);
    Yi **postHupai = getPostHupai(shoupai, juInfo->baopai, juInfo->fubaopai);
    TeshuDamanguan damanguanFlag;
    Mianzi ***mianzi = huleMianzi(&damanguanFlag, get0to5Shoupai(shoupai), &shoupai->drawnPai);

    size_t count = 0;
    while(mianzi[count] != NULL){
        Hudi *hudi = getHudi(mianzi[count], juInfo->zhuangfeng, juInfo->menfeng);
        Yi **hupai = getHupai(damanguanFlag, mianzi[count], hudi, preHupai);

        if(hupai == NULL || hupai[0] == NULL) continue;
        size_t hupaiCount = 0;
        while(hupai[hupaiCount] != NULL) hupaiCount++;

        uint8_t fu = hudi->fu;
        uint8_t fanshu = 0, defen = 0, damanguan = 0;
        Sijia baojia2 = 0; uint8_t defen2 = 0;

        if(hupai[0]->fanshu < 0){
            // 役満の時
            for(int i = 0; i < hupaiCount; i++){
                damanguan += abs(hupai[i]->fanshu);
                if(hupai[i]->baojia != 0){
                    baojia2 = hupai[i]->baojia;             // パオ対象
                    defen2 = 8000 * abs(hupai[i]->fanshu);  // パオ対象の基本点
                }
            }

            defen = 8000 * damanguan;   // パオを含む全体の基本点
        }else{
            // 役満以外の場合
            concatYiArray(hupai, postHupai);
            while(hupai[hupaiCount] != NULL) hupaiCount++;

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

        int8_t fenpei[4] = {0};    // 収入と負担額 fenpei[Fengpai]
        
        // パオがあった場合
        if(defen2 > 0){
            if(shoupai->drawnPai->paiSource != Zimo) defen2 /= 2;   // ロン和了の場合は放銃者と折半
            defen  -= defen2;                                       // 基本点からパオ分を減算
            defen2 *= juInfo->menfeng == Dong ? 6 : 4;                 // パオ分の負担額

            fenpei[juInfo->menfeng] =  defen2;                      // 和了者の収入を加算
            fenpei[baojia2]         = -defen2;                      // パオ対象の負担額を減算 
        }

        if(shoupai->drawnPai->paiSource != Zimo || defen == 0){
            // ロン和了もしくはパオ1人払いの場合
            Sijia baojia = defen == 0 ? baojia2 : shoupai->drawnPai->paiSource; // パオ1人払いは放銃者扱い

            //--- 負担額の計算(100点未満切り上げ) ---//
            defen = ceil(defen * (juInfo->menfeng == Dong ? 6 : 4) / 100) * 100;
            fenpei[juInfo->menfeng] +=  defen + juInfo->changbang * 300 + juInfo->lizhibang * 1000; // 和了者の収入を加算(供託含む)
            fenpei[baojia]          += -defen - juInfo->changbang * 300;                            // 放銃者の負担額を減算(供託含む)
        }else{
            // ツモ和了の場合  
            uint8_t zhuangjia = ceil(defen * 2 / 100) * 100;  // 親の負担額
            uint8_t sanjia    = ceil(defen     / 100) * 100;  // 子の負担額

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
        if(defen + defen2 > max->damanguan){
            max->hupai     = hupai;
            max->fu        = fu;
            max->fanshu    = fanshu;
            max->damanguan = damanguan;
            max->defen     = defen;
            memcpy(max->fenpei, fenpei, sizeof(int8_t) * 4);
        }

        count++;
    }

    return max;
}

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

    Shoupai shoupai = {
        {{0, 0, 3, 3, 3, 2, 0, 0, 0, 0},  // 萬子
         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 筒子
         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 索子
         {0, 0, 0, 0, 0, 0, 0, 0}},       // 字牌
        fulou,
        NULL
    };

    Pai hulepai = { Wanzi, 1, DuiMian };

    Hupai hupai = { 0, false, false, false, 0, 0 };

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