//---------------------------------------------------------------------------
//! @file   InputKey.cpp
//! @brief  キー入力管理
//---------------------------------------------------------------------------
#include "InputKey.h"
#include <System/Scene.h>

namespace Input {

namespace {
constexpr int MAX_KEY_NUM = 256;

enum face
{
    Primary,
    Secondary,

    Num,
};

std::array<std::array<char, MAX_KEY_NUM>, face::Num> keys;

int primary = face::Primary;

int key_now()
{
    return primary;
}
int key_old()
{
    return primary == face::Primary ? face::Secondary : face::Primary;
}

std::array<unsigned char, MAX_KEY_NUM> key_count;

//-----------------------------------------------------------------------
//! keys_配列検証用
//! @param  [in]    keyID   キー種別
//! @return キー種別が範囲外であればtrueが返ります。
//-----------------------------------------------------------------------
bool IsOverKeyNum(u32 keyID)
{
    return (keyID >= MAX_KEY_NUM);
}
};    // namespace

//---------------------------------------------------------------------------
// 初期化
//---------------------------------------------------------------------------
void InputKeyInit()
{
    std::fill(key_count.begin(), key_count.end(), 0);

    // 以下同じ意味です
    //std::fill_n( key_count, MAX_KEY_NUM, 0 );

    /*
		for( int i = 0; i < MAX_KEY_NUM; ++i )
		{
			keys[ i ] = 0;
		}
		*/
}

//---------------------------------------------------------------------------
// 更新
//---------------------------------------------------------------------------
int InputKeyUpdate()
{
    primary++;
    primary %= face::Num;

    GetHitKeyStateAll(keys[key_now()].data());

    // ポーズの時はF1,F2しか効かないようにしておく
    if(Scene::IsPause()) {
        for(int i : {KEY_INPUT_F1, KEY_INPUT_F2}) {
            if(keys[key_now()][i] != 0) {
                key_count[i]++;
                if(key_count[i] >= CHAR_MAX)
                    key_count[i] = CHAR_MAX;
                continue;
            }
            key_count[i] = 0;
        }
        //で、解除見込みの場合の時は、すべてのキーを読み取る
        if(IsKeyDown(KEY_INPUT_F1) || IsKeyDown(KEY_INPUT_F2)) {
            key_count[KEY_INPUT_F1] = 0;
            key_count[KEY_INPUT_F2] = 0;

            for(int i = 0; i < MAX_KEY_NUM; ++i) {
                if(keys[key_now()][i] != 0) {
                    key_count[i]++;
                    if(key_count[i] >= CHAR_MAX)
                        key_count[i] = CHAR_MAX;
                    continue;
                }

                key_count[i] = 0;
            }
        }
    }
    else {
        for(int i = 0; i < MAX_KEY_NUM; ++i) {
            if(keys[key_now()][i] != 0) {
                key_count[i]++;
                if(key_count[i] >= CHAR_MAX)
                    key_count[i] = CHAR_MAX;
                continue;
            }

            key_count[i] = 0;
        }
    }
    return 0;
}
//---------------------------------------------------------------------------
// 終了
//---------------------------------------------------------------------------
void InputKeyExit()
{
}

//---------------------------------------------------------------------------
// 指定キーの1回だけ押下検証
//---------------------------------------------------------------------------
bool IsKeyOn(int keyID)
{
    if(ImGui::IsAnyItemActive())
        return false;

    if(IsOverKeyNum(keyID))
        return false;
    return (key_count[keyID] == 1);
}

//---------------------------------------------------------------------------
// 指定キーが押されていない検証
//---------------------------------------------------------------------------
bool IsKeyRelease(int keyID)
{
    if(ImGui::IsAnyItemActive())
        return false;

    if(IsOverKeyNum(keyID))
        return false;
    return (key_count[keyID] == 0);
}

//---------------------------------------------------------------------------
// 指定キーの長押し検証
//---------------------------------------------------------------------------
bool IsKeyRepeat(int keyID, u32 frame)
{
    if(ImGui::IsAnyItemActive())
        return false;

    if(IsOverKeyNum(keyID))
        return false;
    return (key_count[keyID] >= frame);
}

//---------------------------------------------------------------------------
// 指定キーの検証
//---------------------------------------------------------------------------
bool IsKey(int keyID)
{
    if(ImGui::IsAnyItemActive())
        return false;

    return keys[key_now()][keyID];
}

//---------------------------------------------------------------------------
// Windowsポンプに合わせた命名
//---------------------------------------------------------------------------
bool IsKeyDown(int keyID)
{
    if(ImGui::IsAnyItemActive())
        return false;

    return keys[key_now()][keyID] && !keys[key_old()][keyID];
}
bool IsKeyUp(int keyID)
{
    if(ImGui::IsAnyItemActive())
        return false;

    return !keys[key_now()][keyID] && keys[key_old()][keyID];
}

}    // namespace Input
