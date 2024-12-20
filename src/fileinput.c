#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <setjmp.h>

#include "common.h"
#include "hule.h"
#include "fileinput.h"
#include "stdoutput.h"

jmp_buf jump_buffer;    /* longjmp() から復帰したときに復元するためのバッファ */

/** @brief 読み込みエラーが生じた時の処理
 * @param file (FILE *) ファイルポインタ
 * @param message (char *) エラーメッセージ
 * @details 別のファイルから読み出す時はstdinのみ使用する
 */
void inputErr(FILE *file, char *message){
    if(file == stdin) printf("入力エラー : %s\n", message);
    else              longjmp(jump_buffer, 1);  // stdinいがいのファイル入力の場合はlongjmpでエラーを返す
    return;
}

/** @brief 入力を受け付け、ポインタ先に代入する関数
 * @param ptr (void *) 代入先のポインタ
 * @param type (uint8_t) 入力の種類 0...数字 1...文字列
 */
bool lineinput(FILE *file, void *ptr, uint8_t type){
    char charinput[15]; // 入力文字列

    // 入力を受け付ける
    if(fgets(charinput, sizeof(charinput), file) == NULL){
        inputErr(file, "読み込みに失敗しました");
        return false;
    }

    // 改行を取り除く
    size_t len = strlen(charinput);
    if(len > 0 && charinput[len - 1] == '\n'){
        charinput[len - 1] = '\0';
        len--;
    }

    // キャリッジリターンが存在するなら取り除く(Windows対策)
    if(len > 0 && charinput[len - 1] == '\r'){
        charinput[len - 1] = '\0';
        len--;
    }

    if(type == 0){
        // 入力が数字かを確認
        for(size_t i = 0; i < len; i++){
            if(!isdigit(charinput[i])){ 
                inputErr(file, "数字ではありません");
                return false;
            }
        }

        // 数字をポインタ先に代入
        *((uint8_t *)ptr) = atoi(charinput); 
    }else if(type == 1){
        // 文字列は代入
        strcpy((char *)ptr, charinput);
    }

    return true;
}

/** @brief 局の情報を入力する関数
 * @param juInfo (JuInfo *) 局の情報のポインタ
 */
void inputJuInfo(FILE *file, JuInfo *juInfo){
    memset(juInfo->baopai,   0, sizeof(uint8_t) * 5);
    memset(juInfo->fubaopai, 0, sizeof(uint8_t) * 5);
    juInfo->hupai = allocateMemory(1, sizeof(Hupai), false);
    bool loop = false;

    char charinput[15];
    uint8_t numinput;
    do{
        loop = false;
        if(file == stdin){
            printf("局の情報を入力してください\n");
            printf("場風と自風は次のように答えてください ... 0.東, 1.南, 2.西, 3.北\n\n");
        }

        if(file == stdin) printf("場風 : ");
        if(!lineinput(file, &numinput, 0)) { loop = true; continue; }
        if(0 > numinput || 3 < numinput) { inputErr(file, "無効な入力です 0~3の間で入力してください\n"); loop = true; continue; }
        juInfo->zhuangfeng = numinput;

        if(file == stdin) printf("和了者の自風 : ");
        if(!lineinput(file, &numinput, 0)) { loop = true; continue; }
        if(0 > numinput || 3 < numinput) { inputErr(file, "無効な入力です 0~3の間で入力してください\n"); loop = true; continue; }
        juInfo->menfeng = numinput;
    }while(loop);

    do{
        loop = false;
        if(file == stdin){
            printf("\nドラの表示牌を以下のように入力してください 複数ある場合はスペースを空けて入力してください 裏ドラがない場合は0を入力してください\n");
            printf("アルファベット1文字と数字3文字で表す。(萬子:m, 筒子:p, 索子:s, 字牌:z) 例 一萬->1m, 三筒->3p\n");
            printf("赤ドラは5を入力, 1z : 東, 2z : 南, 3z : 西, 4z : 北, 5z : 白, 6z : 發, 7z : 中\n");
        }

        uint8_t count = 0;
        if(file == stdin) printf("ドラ : ");
        if(!lineinput(file, charinput, 1)) { loop = true; continue; }
        char *ptr = strtok(charinput, " ");

        do{
            if(ptr[1] != 'm' && ptr[1] != 'p' && ptr[1] != 's' && ptr[1] != 'z') { inputErr(file, "無効な入力です。牌の種類に誤りがあります。\n"); loop = true; break; }
            if(strlen(ptr) != 2) { inputErr(file, "無効な入力です 各牌は2文字で入力してください\n"); loop = true; break; }
            if(ptr[0] - '0' < 1 || 9 < ptr[0] - '0') { inputErr(file, "無効な入力です 1~9で入力してください\n"); loop = true; break; }
            if(ptr[1] == 'z' && (ptr[0] - '0' < 1 || 7 < ptr[0] - '0')) { inputErr(file, "無効な入力です 字牌は1~7で入力してください\n"); loop = true; break; }
            if(count >= 6) { inputErr(file, "無効な入力です ドラは5枚までです\n"); loop = true; break; }

            switch(ptr[1]){
                case 'm': juInfo->baopai[count] =      ptr[0] - '0'; break;
                case 'p': juInfo->baopai[count] = 10 + ptr[0] - '0'; break;
                case 's': juInfo->baopai[count] = 20 + ptr[0] - '0'; break;
                case 'z': juInfo->baopai[count] = 30 + ptr[0] - '0'; break;
            }

            count++;
            ptr = strtok(NULL, " ");
        }while(ptr != NULL);
        if(loop) continue;

        count = 0;
        if(file == stdin) printf("裏ドラ : ");
        if(!lineinput(file, charinput, 1)) { loop = true; continue; }
        ptr = strtok(charinput, " ");

        do{
            if(ptr[0] == '0' && strlen(ptr) == 1) break;
            if(ptr[1] != 'm' && ptr[1] != 'p' && ptr[1] != 's' && ptr[1] != 'z') { inputErr(file, "無効な入力です 牌の種類に誤りがあります。\n"); loop = true; break; }
            if(strlen(ptr) != 2) { inputErr(file, "無効な入力です 各牌は2文字で入力してください\n"); loop = true; break; }
            if(ptr[0] - '0' < 1 || 9 < ptr[0] - '0') { inputErr(file, "無効な入力です 1~9で入力してください\n"); loop = true; break; }
            if(ptr[1] == 'z' && (ptr[0] - '0' < 1 || 7 < ptr[0] - '0')) { inputErr(file, "無効な入力です 字牌は1~7で入力してください\n"); loop = true; break; }
            if(count >= 6) { inputErr(file, "無効な入力です ドラは5枚までです\n"); loop = true; break; }

            switch(ptr[1]){
                case 'm': juInfo->fubaopai[count] =      ptr[0] - '0'; break;
                case 'p': juInfo->fubaopai[count] = 10 + ptr[0] - '0'; break;
                case 's': juInfo->fubaopai[count] = 20 + ptr[0] - '0'; break;
                case 'z': juInfo->fubaopai[count] = 30 + ptr[0] - '0'; break;
            }

            count++;
            ptr = strtok(NULL, " ");
        }while(ptr != NULL);
    }while(loop);

    do{
        loop = false;

        if(file == stdin) printf("\n積み棒の数 : ");
        if(!lineinput(file, &numinput, 0)) { loop = true; continue; }
        juInfo->changbang = numinput;

        if(file == stdin) printf("リーチ棒の数 : ");
        if(!lineinput(file, &numinput, 0)) { loop = true; continue; }
        juInfo->lizhibang = numinput;

    }while(loop);

    do{
        loop = false;
        if(file == stdin){
            printf("\n状況役の有無を数字6桁で入力してください\n");
            printf("1桁目 : 0.なし 1.立直 2.ダブル立直\n");
            printf("2桁目 : 0.なし 1.一発\n");
            printf("3桁目 : 0.なし 1.槍槓\n");
            printf("4桁目 : 0.なし 1.嶺上開花\n");
            printf("5桁目 : 0.なし 1.海底摸月 2.河底撈魚\n");
            printf("6桁目 : 0.なし 1.天和 2.地和\n");
        }

        if(file == stdin) printf("入力 : ");
        if(!lineinput(file, charinput, 1)) { loop = true; continue; }
        if(strlen(charinput) != 6) { inputErr(file, "無効な入力です 6桁で入力してください\n"); loop = true; continue; }

        if(charinput[0] - '0' < 0 || 2 < charinput[0] - '0') { inputErr(file, "無効な入力です 1桁目は0~2の間で入力してください\n"); loop = true; continue; }
        if(charinput[1] - '0' < 0 || 1 < charinput[1] - '0') { inputErr(file, "無効な入力です 2桁目は0~1の間で入力してください\n"); loop = true; continue; }
        if(charinput[2] - '0' < 0 || 1 < charinput[2] - '0') { inputErr(file, "無効な入力です 3桁目は0~1の間で入力してください\n"); loop = true; continue; }
        if(charinput[3] - '0' < 0 || 1 < charinput[3] - '0') { inputErr(file, "無効な入力です 4桁目は0~1の間で入力してください\n"); loop = true; continue; }
        if(charinput[4] - '0' < 0 || 2 < charinput[4] - '0') { inputErr(file, "無効な入力です 5桁目は0~2の間で入力してください\n"); loop = true; continue; }
        if(charinput[5] - '0' < 0 || 2 < charinput[5] - '0') { inputErr(file, "無効な入力です 6桁目は0~2の間で入力してください\n"); loop = true; continue; }

        juInfo->hupai->lizhi     = charinput[0] - '0';
        juInfo->hupai->yifa      = charinput[1] - '0';
        juInfo->hupai->qianggang = charinput[2] - '0';
        juInfo->hupai->lingshang = charinput[3] - '0';
        juInfo->hupai->haidi     = charinput[4] - '0';
        juInfo->hupai->tianhu    = charinput[5] - '0';        
    }while(loop);

    return;
}

/** @brief 副露を入力する関数
 * @param count (uint8_t *) 手牌の枚数
 * @param shoupai (Shoupai *) 手牌の構造体
 * @param menfeng (Fengpai) 自風
 */
void inputfulou(FILE *file, uint8_t *count, Shoupai *shoupai, const Fengpai menfeng){
    if(file == stdin) printf("\n[副露の入力]\n");

    uint8_t fulouCount = 0;
    while(shoupai->fulou[fulouCount] != NULL) fulouCount++;
    bool flag = true;

    do{
        if(flag && file == stdin){
            printf("以下のように1牌ずつ入力してください。副露面子の入力を終了する時はexitと入力してください\n");
            printf("アルファベット1文字と数字3文字で表す。(萬子:m, 筒子:p, 索子:s, 字牌:z) 例 一萬のポン->m111, 三筒->p333\n");
            printf("赤ドラは無視し5を入力, 1z : 東, 2z : 南, 3z : 西, 4z : 北, 5z : 白, 6z : 發, 7z : 中\n");
        }
        if(file == stdin) printf("現在の手牌 : %s\n\n", shoupaiToString(shoupai));
        flag = true;

        char charinput[15];
        if(file == stdin) printf("入力 : ");
        if(!lineinput(file, charinput, 1)){ continue; }

        if(strcmp(charinput, "exit") == 0) break; // 入力終了判定

        // 入力確認
        if(charinput[0] != 'm' && charinput[0] != 'p' && charinput[0] != 's' && charinput[0] != 'z'){ inputErr(file, "無効な入力です 牌の種類に誤りがあります。\n"); continue; }
        if(strlen(charinput) < 4){ inputErr(file, "無効な入力です 数字は3枚以上必要です\n"); continue; }
        if(charinput[1] != charinput[2] && charinput[5] != '\0') { inputErr(file, "無効な入力です 順子は3枚のみです\n"); continue; }

        if(charinput[0] == 'z'){
            // 2~4(5)文字目が1~7ではないとき
            if(charinput[1] - '0' < 0 || 9 < charinput[1] - '0') { inputErr(file, "無効な入力です 字牌は1~7のみです\n"); continue; }
            if(charinput[2] - '0' < 0 || 9 < charinput[2] - '0') { inputErr(file, "無効な入力です 字牌は1~7のみです\n"); continue; }
            if(charinput[3] - '0' < 0 || 9 < charinput[3] - '0') { inputErr(file, "無効な入力です 字牌は1~7のみです\n"); continue; }
            if(strlen(charinput) == 5 && (charinput[4] - '0' < 0 || 9 < charinput[4] - '0')) { inputErr(file, "無効な入力です 字牌は1~7のみです\n"); continue; }

            // 字牌のときで、順子のとき
            if(charinput[1] != charinput[2] || charinput[1] != charinput[3]) { inputErr(file, "無効な入力です 字牌は順子になりません\n"); continue; }
        }else{
            // 2~4(5)文字目が数字ではないとき
            if(charinput[1] - '0' < 0 || 9 < charinput[1] - '0') { inputErr(file, "無効な入力です 2文字目は数字である必要があります\n"); continue; }
            if(charinput[2] - '0' < 0 || 9 < charinput[2] - '0') { inputErr(file, "無効な入力です 3文字目は数字である必要があります\n"); continue; }
            if(charinput[3] - '0' < 0 || 9 < charinput[3] - '0') { inputErr(file, "無効な入力です 4文字目は数字である必要があります\n"); continue; }
            if(strlen(charinput) == 5 && (charinput[4] - '0' < 0 || 9 < charinput[4] - '0')) { inputErr(file, "無効な入力です 5文字目は数字である必要があります\n"); continue; }

            if(charinput[1] != charinput[2] && !(charinput[1] - '0' + 1 == charinput[2] - '0' && charinput[2] - '0' + 1 == charinput[3] - '0')) { inputErr(file, "無効な入力です 萬子、筒子、索子は刻子または順子である必要があります。\n"); continue; }
        }

        // 更新
        Paizhong paizhong;
        Paixing paixing;

        switch(charinput[0]){
            case 'm': paizhong = Wanzi;  break;
            case 'p': paizhong = Tongzi; break;
            case 's': paizhong = Suozi;  break;
            case 'z': paizhong = Zihpai; break;
        }

        if(charinput[1] == charinput[2] && charinput[2] == charinput[3]){
            paixing = (strlen(charinput) == 4) ? Kezi : Gangzi;  // 刻子または槓子
        }else{
            paixing = Shunzi;  // 順子
        }

        uint8_t value = charinput[1] - '0';

        if(file == stdin){
            printf("副露した牌がどの牌かを入力してください (例 : m2)\n");
            printf("入力 : ");
        }
        if(!lineinput(file, charinput, 1)) { continue; }
        if(charinput[1] - '0' < 1 || 9 < charinput[1] - '0') { inputErr(file, "無効な入力です\n"); continue; }
        uint8_t index = (charinput[1] - '0') - value;

        if(file == stdin){
            printf("副露した牌がどこから来たのかを入力してください\n");
            printf("1.下家, 2.対面, 3.上家 もしくは 4.東家, 5.南家, 6.西家, 7.北家\n");
            if(paixing == Gangzi) printf("暗槓のときは0を入力してください\n");
        }

        uint8_t numinput;
        Sijia source;

        if(file == stdin) printf("入力 : ");
        if(!lineinput(file, &numinput, 0)) { continue; }

        if(numinput == 0 && paixing == Gangzi){
            source = 0;  // 暗槓
        }else if(1 <= numinput && numinput <= 3){
            source = numinput;  // 下家・対面・上家
        }else if(4 <= numinput && numinput <= 7){
            source = (numinput + menfeng - 4) % 4;  // 東家・南家・西家・北家
        }else{
            inputErr(file, "無効な入力です。\n");
            continue;
        }

        if(file == stdin){
            printf("副露牌の中に赤ドラは含まれていますか？ (0.いいえ, 1.はい)");
            printf("入力 : ");
        }
        if(!lineinput(file, &numinput, 0)) { continue; }

        if(numinput == 0 || numinput == 1){
            shoupai->fulouHongpai[fulouCount] = numinput;
        }else{
            inputErr(file, "無効な入力です。\n");
            continue;
        }

        shoupai->fulou[fulouCount] = initializeMianzi(paixing, paizhong, value, true, false, index, source);
        fulouCount++;
        *count += (paixing == Gangzi ? 4 : 3);  // 槓子は4枚、副露は3枚追加
        flag = false;
    }while(*count < 14);

    if(file == stdin) printf("副露の入力を終了します\n");
}

/** @brief 手牌を入力する関数
 * @param shoupai (Shoupai *) 手牌の構造体
 * @param menfeng (Fengpai) 自風
 */
void inputShoupai(FILE *file, Shoupai *shoupai, const Fengpai menfeng){
    memset(shoupai->bingpai, 0, sizeof(uint8_t) * 4 * 10);
    shoupai->fulou = allocateMemory(5, sizeof(Mianzi *), true);
    shoupai->fulouHongpai = allocateMemory(5, sizeof(bool), true);
    shoupai->hulepai = NULL;

    if(file == stdin) printf("\n[手牌の入力]\n");

    uint8_t count = 0; // 現在の牌数
    char charinput[15];
    bool flag = true;
    do{
        if(flag && file == stdin){
            printf("以下のように1牌ずつ入力してください。副露面子を入力するときはfと入力してください\n");
            printf("数字1文字とアルファベット1文字で表す。(萬子:m, 筒子:p, 索子:s, 字牌:z) 例 一萬->1m, 三筒->3p\n");
            printf("赤ドラの場合は、数字を0で指定(0z, 0p, 0s)\n");
            printf("1z : 東, 2z : 南, 3z : 西, 4z : 北, 5z : 白, 6z : 發, 7z : 中\n");
        }
        if(file == stdin) printf("現在の手牌 : %s\n\n", shoupaiToString(shoupai));
        flag = true;

        if(file == stdin) printf("入力 : ");
        if(!lineinput(file, charinput, 1)){ inputErr(file, "無効な入力です\n"); continue; }

        if(charinput[0] == 'f'){
            inputfulou(file, &count, shoupai, menfeng);
            if(file == stdin) printf("[手牌の入力]\n");
            continue;
        }

        // 入力確認
        if(charinput[1] != 'm' && charinput[1] != 'p' && charinput[1] != 's' && charinput[1] != 'z'){
            inputErr(file, "無効な入力です。牌の種類に誤りがあります。\n");
            continue;  // 牌の種類が異なるとき
        }
        if(strlen(charinput) != 2){
            inputErr(file, "無効な入力です 2文字で入力してください\n");
            continue;  // 入力された牌が2枚以下のとき
        }
        if(charinput[0] - '0' < 0 || 9 < charinput[0] - '0'){
            inputErr(file, "無効な入力です 0~9で入力してください\n");
            continue;
        }
        if(charinput[1] == 'z' && (charinput[0] - '0' < 1 || 7 < charinput[0] - '0')){
            inputErr(file, "無効な入力です 字牌の時は1~7で入力してください\n");
            continue;
        }

        // 更新
        Paizhong paizhong;
        switch(charinput[1]){
            case 'm': paizhong = Wanzi; break;
            case 'p': paizhong = Tongzi; break;
            case 's': paizhong = Suozi; break;
            case 'z': paizhong = Zihpai; break;
        }
        shoupai->bingpai[paizhong][charinput[0] - '0']++;

        count++;
        flag = false;
    }while(count < 14);

    shoupai->hulepai = allocateMemory(1, sizeof(Pai), false);
    bool loop = false;
    do{
        loop = false;
        if(file == stdin){
            printf("\n和了牌になりえる牌を入力してください(最後に手牌にきた牌、最後に副露しようとした牌)\n");
            printf("数字1文字とアルファベット1文字で表す。(萬子:m, 筒子:p, 索子:s, 字牌:z) 例 一萬->1m, 三筒->3p\n");
            printf("赤ドラの場合は、数字を0で指定(0z, 0p, 0s)\n");
            printf("z1 : 東, z2 : 南, z3 : 西, z4 : 北, z5 : 白, z6 : 發, z7 : 中\n\n");
        }

        if(file == stdin) printf("入力 : ");
        if(!lineinput(file, charinput, 1)){ loop = true; continue; }

        // 入力確認
        if(charinput[1] != 'm' && charinput[1] != 'p' && charinput[1] != 's' && charinput[1] != 'z'){
            inputErr(file, "無効な入力です。牌の種類に誤りがあります。\n");
            loop = true;
            continue;  // 牌の種類が異なるとき
        }
        if(strlen(charinput) != 2){
            inputErr(file, "無効な入力です 2文字で入力してください\n");
            loop = true;
            continue;  // 入力された牌が2枚以下のとき
        }
        if(charinput[0] - '0' < 0 || 9 < charinput[0] - '0'){
            inputErr(file, "無効な入力です 0~9で入力してください\n");
            loop = true;
            continue;
        }
        if(charinput[1] == 'z' && (charinput[0] - '0' < 1 || 7 < charinput[0] - '0')){
            inputErr(file, "無効な入力です 字牌の時は1~7で入力してください\n");
            loop = true;
            continue;
        }

        switch(charinput[1]){
            case 'm': shoupai->hulepai->paizhong = Wanzi; break;
            case 'p': shoupai->hulepai->paizhong = Tongzi; break;
            case 's': shoupai->hulepai->paizhong = Suozi; break;
            case 'z': shoupai->hulepai->paizhong = Zihpai; break;
        }
        shoupai->hulepai->value = charinput[0] - '0';

        if(loop) continue;

        if(file == stdin){
            printf("\nその牌がどこから来たのかを入力してください\n");
            printf("0.自家 1.下家, 2.対面, 3.上家 もしくは 4.東家, 5.南家, 6.西家, 7.北家\n\n");
        }

        uint8_t numinput;
        Sijia source;
        if(file == stdin) printf("入力 : ");
        if (!lineinput(file, &numinput, 0)) { loop = true; continue; }

        // 範囲チェック
        if (0 <= numinput && numinput <= 3) {
            source = numinput;  // 自家・下家・対面・上家
        } else if (4 <= numinput && numinput <= 7) {
            source = (numinput + menfeng - 4) % 4;  // 東家・南家・西家・北家
        } else {
            inputErr(file, "無効な入力です。\n");
            loop = true;
            continue;
        }

        shoupai->hulepai->source = source;  // 正常な入力の場合
    } while (loop);
    return;
}

/** @brief ユーザーが入力する関数
 * @param juInfo (JuInfo *) 局の情報のポインタ
 * @param shoupai (Shoupai *) 手牌の構造体
 * @return (bool) ユーザーが入力したかどうか
 */
bool fileInput(FILE *file, JuInfo *juInfo, Shoupai *shoupai){
    // 変数がNULLのとき
    if(juInfo == NULL || shoupai == NULL) return false;

    if(setjmp(jump_buffer) == 0){
        // 最初に setjmp() を呼び出したときは、こちらに分岐する

        // 各情報を入力
        inputJuInfo(file, juInfo);
        inputShoupai(file, shoupai, juInfo->menfeng);

        return true;
    }else{
        // longjmp() から戻ってきたときは、こちらに分岐する
        // ファイル入力に問題が発生したとき
        return false;
    }
}