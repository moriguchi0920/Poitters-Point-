//---------------------------------------------------------------------------
//!	@file	InputPad.cpp
//! @brief	パッド入力管理
//---------------------------------------------------------------------------
#include "InputPad.h"

namespace {
constexpr unsigned int MAX_PAD_NUM            = static_cast<unsigned int>(PAD_NO::PAD_NO_MAX);    //<! PAD接続最大数
constexpr unsigned int MAX_PAD_TYPE           = static_cast<unsigned int>(PAD_ID::PAD_ID_MAX);    //<! 上下左右/A B C X Y Z L R START Mボタン
constexpr unsigned int MIN_PAD_TYPE           = static_cast<unsigned int>(PAD_ID::PAD_A);         //<! 上下左右/A B C X Y Z L R START Mボタン
constexpr unsigned int MAX_PAD_DIRECT_NUM     = 4;                                                //<! 方向キーの方向最大値
constexpr float        MAX_ANALOG_INPUT_VALUE = 1000.0f;                                          //<! アナログスティックの入力値の最大値
constexpr unsigned int PAD_BUTTON_NUM         = static_cast<unsigned int>(PAD_ID::PAD_R_PUSH) - static_cast<unsigned int>(PAD_ID::PAD_A) + 1;

std::vector<DINPUT_JOYSTATE>                                          pad_input_states;        //<! 詳細なパッドの状態の取得
std::array<bool, MAX_PAD_NUM>                                         use_pads;                //<! 使用しているパッドの状態を管理
std::array<std::array<unsigned int, PAD_BUTTON_NUM>, MAX_PAD_NUM>     pad_buttons;             //<! 使用しているパッドのボタンの押下状態を管理
std::array<std::array<unsigned int, MAX_PAD_DIRECT_NUM>, MAX_PAD_NUM> pad_direct_keys;         //<! 使用しているパッドの方向キーの押下状態を管理
bool                                                                  is_all_on_direct_key;    //<! 方向キーのいずれかが押されているかどうか

// パッド番号の範囲外チェック関数
bool isOverPadNum(s32 pad_num)
{
    return (pad_num < 0 || pad_num >= MAX_PAD_NUM);
}
// そのパッド番号が使用中かどうか
bool isUsePadNum(s32 pad_num)
{
    if(isOverPadNum(pad_num))
        return false;
    return use_pads[pad_num];
}
// そのパッドIDの範囲内チェック
bool isOverPadID(u32 pad_id)
{
    return (pad_id < MIN_PAD_TYPE || pad_id >= MAX_PAD_TYPE);
}
};    // namespace

//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
void InputPadInit()
{
    int connect_pad_num = GetJoypadNum();
    pad_input_states.resize(connect_pad_num);
    std::fill(use_pads.begin(), use_pads.end(), false);
    // 使用中のパッドをすべて登録
    std::fill(use_pads.begin(), use_pads.begin() + connect_pad_num, true);

    is_all_on_direct_key = false;
}

//---------------------------------------------------------------------------
//! 更新
//---------------------------------------------------------------------------
void InputPadUpdate()
{
    // パッドの入力状態を詳細に取得
    for(int i = 0; i < MAX_PAD_NUM; ++i) {
        if(isUsePadNum(i) == false)
            continue;
        GetJoypadDirectInputState(DX_INPUT_PAD1 + i, &pad_input_states[i]);
    }

    for(int i = 0; i < MAX_PAD_NUM; ++i) {
        if(isUsePadNum(i) == false)
            continue;

        // 各ボタンが押されているフレーム数を計算
        DINPUT_JOYSTATE input_state = pad_input_states[i];
        for(int j = 0; j < PAD_BUTTON_NUM; ++j) {
            if(input_state.Buttons[j] == 128) {
                ++pad_buttons[i][j];
                continue;
            }
            pad_buttons[i][j] = 0;
        }
        // 方向キーの各方向が押されているフレーム数を計算
        int pov_0_index = ((input_state.POV[0] + 4500) % 36000) / 9000;
        if(input_state.POV[0] != -1) {
            pad_direct_keys[i][pov_0_index]++;
            continue;
        }
        else {
            pov_0_index = -1;
        }
        // 方向キーの各方向の押されていないフレーム数をリセット
        for(int j = 0; j < MAX_PAD_DIRECT_NUM; ++j) {
            if(j == pov_0_index)
                continue;
            pad_direct_keys[i][j] = 0;
        }
    }
}

//---------------------------------------------------------------------------
// 描画（デバッグ用のため、現在は不要）
//---------------------------------------------------------------------------
//void InputPadRender()
//{
//    //for (int i = 0; i < MAX_PAD_NUM; ++i)
//    //{
//    //    DINPUT_JOYSTATE input_state = pad_input_states[i];
//    //    int pov_0_index = ((input_state.POV[0] + 4500) % 36000) / 9000;
//    //    printfDx("%d\n", pad_direct_keys[0][pov_0_index]);
//    //}
//
//    //for (int i = 0; i < MAX_PAD_NUM; ++i)
//    //{
//    //	if (isUsePadNum(i) == false)
//    //		continue;
//    //	DINPUT_JOYSTATE input_state = pad_input_states[i];
//    //	printfDx("POV[0]: %d\n", input_state.POV[0]);
//    //	for (int j = 0; j < MAX_PAD_DIRECT_NUM; ++j)
//    //	{
//    //		printfDx("[%d]: %d\n", j, pad_direct_keys[i][j]);
//    //	}
//    //}
//    //for (int i = 0; i < MAX_PAD_NUM; ++i)
//    //{
//    //	if (isUsePadNum(i) == false)
//    //		continue;
//    //	DINPUT_JOYSTATE input_state = pad_input_states[i];
//    //	int pov_0 = input_state.POV[0];
//    //	//int deg = min(((pov_0 >= 31500) ? 35000 : 36000) / (pov_0 + 4500), 4) % 4;
//    //	//printfDx("POV0: %d(%d)\n", pov_0, deg);
//    //	//printfDx("POV0: %d(%d)\n", pov_0, 36000 / (pov_0 + 4500));
//    //	//int ans1 = pov_0 + 4500;
//    //	//int ans2 = 36000 / ans1;
//    //	//int ans3 = min(ans2, 4);
//    //	//int ans4 = ans3 % 4;
//    //	//printfDx("POV0: %d(%d, %d, %d, %d)\n", pov_0, ans1, ans2, ans3, ans4);
//    //	//printfDx("POV0: %d(%d)\n", pov_0, ((pov_0 + 4500)%36000)/9000);
//    //	printfDx("POV0: %d(%d)\n", pov_0, (((pov_0 / 4500) + 1) & 7) >> 1);
//    //}
//    //for (int i = 0; i < MAX_PAD_NUM; ++i)
//    //{
//    //	if (isUsePadNum(i) == false)
//    //		continue;
//    //	for (int j = 0; j < PAD_BUTTON_NUM; ++j)
//    //	{
//    //		printfDx("[%d]: %d, ", j, pad_buttons[i][pad_now()][j]);
//    //	}
//    //	printfDx("\n");
//    //}
//    //unsigned int Color = GetColor(255, 255, 255);
//    ////for (const auto& input_state : pad_input_states)
//    //for (int i = 0; i < MAX_PAD_NUM; ++i)
//    //{
//    //	if (isUsePadNum(i) == false)
//    //		continue;
//    //	DINPUT_JOYSTATE input_state = pad_input_states[i];
//    //	// 左側アナログスティック
//    //	//	X: 左-1000 / 右1000 / 押してない0
//    //	//	Y: 上-1000 / 下1000 / 押してない0
//    //	//	Z: LT/RT(0～-1000)
//    //	DrawFormatString(0, 32 * i, Color,  "X:%d Y:%d Z:%d", input_state.X, input_state.Y, input_state.Z);
//    //	//// 右側アナログスティック
//    //	////	Rx: 左-1000 / 右1000 / 押してない0
//    //	////	Ry: 上-1000 / 下1000 / 押してない0
//    //	////	Rz: 不明
//    //	//DrawFormatString(0, 16, Color, "Rx:%d Ry:%d Rz:%d",	input_state.Rx, input_state.Ry, input_state.Rz);
//    //	//// 不明
//    //	//DrawFormatString(0, 32, Color, "Slider 0:%d 1:%d",	input_state.Slider[0], input_state.Slider[1]);
//    //	//// 左側方向キー入力
//    //	////	※ 上: 0, 右: 9000, 下: 1800, 左: 2700
//    //	////	※ PAD1の場合、POV[0]が反応。他のパッドがあるときはそれぞれが反応？
//    //	//DrawFormatString(0, 48, Color, "POV 0:%d 1:%d 2:%d 3:%d", input_state.POV[0], input_state.POV[1], input_state.POV[2], input_state.POV[3]);
//    //	//DrawString(0, 64, "Button", Color);
//    //	//// Button[ 0]: Aボタン(0 or 128)
//    //	//// Button[ 1]: Bボタン(0 or 128)
//    //	//// Button[ 2]: Xボタン(0 or 128)
//    //	//// Button[ 3]: Yボタン(0 or 128)
//    //	//// Button[ 4]: LBボタン(左人差し指側/0 or 128)
//    //	//// Button[ 5]: RBボタン(右人差し指側/0 or 128)
//    //	//// Button[ 6]: BACKボタン(0 or 128)
//    //	//// Button[ 7]: STARTボタン(0 or 128)
//    //	//// Button[ 8]: 左アナログスティック押し込み(0 or 128)
//    //	//// Button[ 9]: 右アナログスティック押し込み(0 or 128)
//    //	//// Button[10] ～ Button[31]: 不明
//    //	//for (int i = 0; i < 32; i++)
//    //	//{
//    //	//	DrawFormatString(64 + i % 8 * 64, 64 + i / 8 * 16, Color,
//    //	//		"%2d:%d",
//    //	//		i, input_state.Buttons[i]);
//    //	//}
//    //}
//}

//---------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------
void InputPadExit()
{
    ;
}

//---------------------------------------------------------------------------
// 特定パッドボタンの1回だけ押下検証
//---------------------------------------------------------------------------
bool IsPadOn(PAD_ID pad_id, PAD_NO pad_no)
{
    unsigned int pad_index = static_cast<unsigned int>(pad_no);
    if(isOverPadNum(pad_index))
        return false;
    if(!isUsePadNum(pad_index))
        return false;

    unsigned int id_index = static_cast<unsigned int>(pad_id);
    if(isOverPadID(id_index))
        return false;

    DINPUT_JOYSTATE input_state = pad_input_states[pad_index];
    //int             pov_0_index = ((input_state.POV[0] + 4500) % 36000) / 9000;
    unsigned int direct_id = static_cast<unsigned int>(pad_id) - static_cast<unsigned int>(PAD_ID::PAD_D_UP);

    bool all_repeat_direct_key = false;
    bool all_on_direct_key     = false;
    for(int i = 0; i < MAX_PAD_DIRECT_NUM; ++i) {
        if(pad_direct_keys[pad_index][i] > 1) {
            all_repeat_direct_key = true;
            break;
        }
    }
    // 方向キーのいずれかがすでに押されていなかった場合、
    // 方向キーのいずれか押下フラグをfalseにする
    if(all_repeat_direct_key == false) {
        is_all_on_direct_key = false;
    }
    for(int i = 0; i < MAX_PAD_DIRECT_NUM; ++i) {
        if(pad_direct_keys[pad_index][i] != 1)
            continue;
        if(is_all_on_direct_key == true)
            continue;

        is_all_on_direct_key = true;
        all_on_direct_key    = true;
        break;
    }

    switch(pad_id) {
    // 各ボタン
    case PAD_ID::PAD_A:
    case PAD_ID::PAD_B:
    case PAD_ID::PAD_X:
    case PAD_ID::PAD_Y:
    case PAD_ID::PAD_L:
    case PAD_ID::PAD_R:
    case PAD_ID::PAD_START:
    case PAD_ID::PAD_BACK:
    case PAD_ID::PAD_L_PUSH:
    case PAD_ID::PAD_R_PUSH:
        return pad_buttons[pad_index][id_index] == 1;

    // 左アナログスティック
    case PAD_ID::PAD_UP:
        return input_state.Y == -1;
    case PAD_ID::PAD_DOWN:
        return input_state.Y == 1;
    case PAD_ID::PAD_LEFT:
        return input_state.X == -1;
    case PAD_ID::PAD_RIGHT:
        return input_state.X == 1;
    // 右アナログスティック
    case PAD_ID::PAD_R_UP:
        return input_state.Ry == -1;
    case PAD_ID::PAD_R_DOWN:
        return input_state.Ry == 1;
    case PAD_ID::PAD_R_LEFT:
        return input_state.Rx == -1;
    case PAD_ID::PAD_R_RIGHT:
        return input_state.Rx == 1;
    // 方向キー
    case PAD_ID::PAD_D_UP:
    case PAD_ID::PAD_D_DOWN:
    case PAD_ID::PAD_D_LEFT:
    case PAD_ID::PAD_D_RIGHT:
        return pad_direct_keys[pad_index][direct_id] == 1;
    case PAD_ID::PAD_D_ALL:
        return all_on_direct_key;

    // ------------------------------
    //	以下未対応のキー
    // ------------------------------
    case PAD_ID::PAD_C:
    case PAD_ID::PAD_Z:
    case PAD_ID::PAD_M:
        return false;
    }

    // ここに入ることはない
    return false;
}

//---------------------------------------------------------------------------
// 特定パッドボタンの解放検証
//	押されていないことの確認
//---------------------------------------------------------------------------
bool IsPadRelease(PAD_ID pad_id, PAD_NO pad_no)
{
    unsigned int pad_index = static_cast<unsigned int>(pad_no);
    if(isOverPadNum(pad_index))
        return false;
    if(!isUsePadNum(pad_index))
        return false;

    unsigned int id_index = static_cast<unsigned int>(pad_id);
    if(isOverPadID(id_index))
        return false;

    DINPUT_JOYSTATE input_state = pad_input_states[pad_index];
    //int             pov_0_index = ((input_state.POV[0] + 4500) % 36000) / 9000;
    unsigned int direct_id             = static_cast<unsigned int>(pad_id) - static_cast<unsigned int>(PAD_ID::PAD_D_UP);
    bool         all_release_dirct_key = true;
    for(int i = 0; i < MAX_PAD_DIRECT_NUM; ++i) {
        if(pad_direct_keys[pad_index][i] != 0) {
            all_release_dirct_key = false;
            break;
        }
    }

    switch(pad_id) {
    // 各ボタン
    case PAD_ID::PAD_A:
    case PAD_ID::PAD_B:
    case PAD_ID::PAD_X:
    case PAD_ID::PAD_Y:
    case PAD_ID::PAD_L:
    case PAD_ID::PAD_R:
    case PAD_ID::PAD_START:
    case PAD_ID::PAD_BACK:
    case PAD_ID::PAD_L_PUSH:
    case PAD_ID::PAD_R_PUSH:
        return pad_buttons[pad_index][id_index] == 0;

    // 左アナログスティック
    case PAD_ID::PAD_UP:
        return input_state.Y == 0;
    case PAD_ID::PAD_DOWN:
        return input_state.Y == 0;
    case PAD_ID::PAD_LEFT:
        return input_state.X == 0;
    case PAD_ID::PAD_RIGHT:
        return input_state.X == 0;
    // 右アナログスティック
    case PAD_ID::PAD_R_UP:
        return input_state.Ry == 0;
    case PAD_ID::PAD_R_DOWN:
        return input_state.Ry == 0;
    case PAD_ID::PAD_R_LEFT:
        return input_state.Rx == 0;
    case PAD_ID::PAD_R_RIGHT:
        return input_state.Rx == 0;

    // 方向キー
    case PAD_ID::PAD_D_UP:
    case PAD_ID::PAD_D_DOWN:
    case PAD_ID::PAD_D_LEFT:
    case PAD_ID::PAD_D_RIGHT:
        return pad_direct_keys[pad_index][direct_id] == 1;
    case PAD_ID::PAD_D_ALL:
        return all_release_dirct_key;
    // ------------------------------
    //	以下未対応のキー
    // ------------------------------
    case PAD_ID::PAD_C:
    case PAD_ID::PAD_Z:
    case PAD_ID::PAD_M:
        return false;
    }

    // ここに入ることはない
    return false;
}

//---------------------------------------------------------------------------
// 特定パッドボタンの長押し検証
//---------------------------------------------------------------------------
bool IsPadRepeat(PAD_ID pad_id, PAD_NO pad_no)
{
    unsigned int pad_index = static_cast<unsigned int>(pad_no);
    if(isOverPadNum(pad_index))
        return false;
    if(!isUsePadNum(pad_index))
        return false;

    unsigned int id_index = static_cast<unsigned int>(pad_id);
    if(isOverPadID(id_index))
        return false;

    DINPUT_JOYSTATE input_state = pad_input_states[pad_index];
    //int             pov_0_index = ((input_state.POV[0] + 4500) % 36000) / 9000;
    unsigned int direct_id            = static_cast<unsigned int>(pad_id) - static_cast<unsigned int>(PAD_ID::PAD_D_UP);
    bool         all_repeat_dirct_key = false;
    for(int i = 0; i < MAX_PAD_DIRECT_NUM; ++i) {
        if(pad_direct_keys[pad_index][i] <= 1)
            continue;

        all_repeat_dirct_key = true;
        break;
    }
    switch(pad_id) {
    // 各ボタン
    case PAD_ID::PAD_A:
    case PAD_ID::PAD_B:
    case PAD_ID::PAD_X:
    case PAD_ID::PAD_Y:
    case PAD_ID::PAD_L:
    case PAD_ID::PAD_R:
    case PAD_ID::PAD_START:
    case PAD_ID::PAD_BACK:
    case PAD_ID::PAD_L_PUSH:
    case PAD_ID::PAD_R_PUSH:
        return pad_buttons[pad_index][id_index] > 1;

    // 左アナログスティック
    case PAD_ID::PAD_UP:
        return input_state.Y <= -1;
    case PAD_ID::PAD_DOWN:
        return input_state.Y >= 1;
    case PAD_ID::PAD_LEFT:
        return input_state.X <= -1;
    case PAD_ID::PAD_RIGHT:
        return input_state.X >= 1;
    // 右アナログスティック
    case PAD_ID::PAD_R_UP:
        return input_state.Ry <= -1;
    case PAD_ID::PAD_R_DOWN:
        return input_state.Ry >= 1;
    case PAD_ID::PAD_R_LEFT:
        return input_state.Rx <= -1;
    case PAD_ID::PAD_R_RIGHT:
        return input_state.Rx >= 1;

    // 方向キー
    case PAD_ID::PAD_D_UP:
    case PAD_ID::PAD_D_DOWN:
    case PAD_ID::PAD_D_LEFT:
    case PAD_ID::PAD_D_RIGHT:
        return pad_direct_keys[pad_index][direct_id] >= 1;
        //return pad_direct_keys[pad_index][pov_0_index] >= 1;
    case PAD_ID::PAD_D_ALL:
        return all_repeat_dirct_key;

    // ------------------------------
    //	以下未対応のキー
    // ------------------------------
    case PAD_ID::PAD_C:
    case PAD_ID::PAD_Z:
    case PAD_ID::PAD_M:
        return false;
    }

    // ここに入ることはない
    return false;
}

//---------------------------------------------------------------------------
// 指定パッドの入力状態の詳細を取得
//---------------------------------------------------------------------------
bool GetPadInputState(DINPUT_JOYSTATE& pad_state, PAD_NO pad_no)
{
    unsigned int pad_index = static_cast<unsigned int>(pad_no);
    if(isOverPadNum(pad_index))
        return false;
    if(!isUsePadNum(pad_index))
        return false;

    // 引数に渡されたDINPUT_JOYSTATEに渡す
    pad_state = pad_input_states[pad_index];
    return true;
}

//---------------------------------------------------------------------------
// 接続されているパッドの数を取得
//  ※ DXライブラリの「GetJoypadNum関数」のラッパ
//---------------------------------------------------------------------------
s32 GetConnecetdPadNum()
{
    int pad_num = GetJoypadNum();

    return (pad_num == 0) ? -1 : pad_num;
}

//---------------------------------------------------------------------------
// 指定パッドの左アナログスティックのX方向の入力値を取得
//---------------------------------------------------------------------------
f32 GetPadInputAnalogLX(PAD_NO pad_no)
{
    unsigned int pad_num = static_cast<unsigned int>(pad_no);

    if(isOverPadNum(pad_num))
        return 0;
    if(!isUsePadNum(pad_num))
        return 0;

    return pad_input_states[pad_num].X / MAX_ANALOG_INPUT_VALUE;
}

//---------------------------------------------------------------------------
// 指定パッドの左アナログスティックのY方向の入力値を取得
//---------------------------------------------------------------------------
f32 GetPadInputAnalogLY(PAD_NO pad_no)
{
    unsigned int pad_num = static_cast<unsigned int>(pad_no);

    if(isOverPadNum(pad_num))
        return 0;
    if(!isUsePadNum(pad_num))
        return 0;

    return pad_input_states[pad_num].Y / MAX_ANALOG_INPUT_VALUE;
}

//---------------------------------------------------------------------------
// 指定パッドの右アナログスティックのX方向の入力値を取得
//---------------------------------------------------------------------------
f32 GetPadInputAnalogRX(PAD_NO pad_no)
{
    unsigned int pad_num = static_cast<unsigned int>(pad_no);

    if(isOverPadNum(pad_num))
        return 0;
    if(!isUsePadNum(pad_num))
        return 0;

    return pad_input_states[pad_num].Rx / MAX_ANALOG_INPUT_VALUE;
}

//---------------------------------------------------------------------------
// 指定パッドの右アナログスティックのY方向の入力値を取得
//---------------------------------------------------------------------------
f32 GetPadInputAnalogRY(PAD_NO pad_no)
{
    unsigned int pad_num = static_cast<unsigned int>(pad_no);

    if(isOverPadNum(pad_num))
        return 0;
    if(!isUsePadNum(pad_num))
        return 0;

    return pad_input_states[pad_num].Ry / MAX_ANALOG_INPUT_VALUE;
}

bool IsPadInputAnalogLDirection(f32* dir_nums)
{
    for(int i = 0; i < 4; ++i) {
        if(dir_nums[i] > 0) {
            return true;
        }
    }
    return false;
}

s32 GetPadInputAnalogLDirection(PAD_NO pad_no)
{
    float analog_x = GetPadInputAnalogLX(pad_no);
    float analog_y = GetPadInputAnalogLY(pad_no);
    float dir[4]   = {0};    // 0: 左 / 1: 右 / 2: 上 / 3: 下
    if(analog_x < 0) {
        dir[0] = abs(analog_x);
    }
    else {
        dir[1] = analog_x;
    }
    if(analog_y < 0) {
        dir[2] = abs(analog_y);
    }
    else {
        dir[3] = analog_y;
    }

    if(IsPadInputAnalogLDirection(dir) == false) {
        // 入力なし
        return -1;
    }

    int max_dir_index = 0;
    for(int i = 1; i < 4; ++i) {
        if(dir[max_dir_index] > dir[i])
            continue;

        max_dir_index = i;
    }

    return max_dir_index;
}
