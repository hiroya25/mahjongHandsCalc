#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "common.h"
#include "mianzi.h"
#include "hule.h"

/** @brief 幺九牌を含むかどうかを確認する関数
 * @param mianzi (Mianzi) 確認する面子
 * @return (bool) Yes / No
 */
bool isYaojiu(Mianzi *mianzi){
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

/** @brief 役のリストをコピーする関数
 * @param yi (Yi *) コピー元の役
 * @return (Yi *) 役の情報が入った型
 */
bool isSameMianzi(Mianzi *data1, Mianzi *data2){
    if(data1->paixing != data2->paixing) return false;
    if(data1->paizhong != data2->paizhong) return false;
    if(data1->value != data2->value) return false;

    return true;
}

/** @brief 役を新たに作成する関数
 * @param name (char *) 役の名前
 * @param fanshu (int) 翻数
 * @param baojia (SiJia) パオがある場合、四家のうちどこか(0...パオなし)
 * @return (Yi *) 役の情報が入った型
 */
Yi *createYi(const char *name, int fanshu, Sijia baojia){
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
Yi **copyYiArray(Yi **data){
    size_t count = 0;
    while(data[count] != NULL) count++;

    Yi **newData = allocateMemory(MAX_HUPAI, sizeof(Yi *), false);
    for(int i = 0; i < count; i++){
        newData[i] = createYi(data[i]->name, data[i]->fanshu, data[i]->baojia);
    }

    return newData;
}

/** @brief 符を求めるプログラム
 * @param shoupai (Shoupai *) 手牌
 * @param hulepai (PaiAction *) 和了牌
 * @return (Mianzi **) 面子の情報の配列(七対子ではなかったときNULL)
 */
Hudi *getHudi(Mianzi **mianzi, Fengpai zhuangfeng, Fengpai menfeng){
    // 初期設定
    Hudi *hudi = allocateMemory(1, sizeof(Hudi), false);
    hudi->fu = 20;
    hudi->menqian = true;
    hudi->zimo = true;

    hudi->kezi;
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
            int fu = 2;     //刻子の符を 2 で初期化
            if(isYaojiu(p)) fu *= 2;                                    // 幺九牌のとき
            if(!p->isFulou){ fu *= 2; hudi->nAnkezi++; }                // 暗刻子のとき(槓子含む)
            if(p->paixing == Gangzi){ fu *= 4; hudi->nGangzi++; }    // 槓子のとき

            hudi->fu += fu;
            break;

        // 順子
        case Shunzi:
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
Yi **getHupai(TeshuDamanguan flag, Mianzi **mianzi, Hudi *hudi, Yi **preHupai){
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
