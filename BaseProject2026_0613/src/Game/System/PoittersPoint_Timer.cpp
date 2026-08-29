//---------------------------------------------------------------------------
//! @file   PoittersPoint_Title.cpp
//! @brief  自作チュートリアルシーンXのタイトル
//---------------------------------------------------------------------------
// システム
#include "PoittersPoint_Timer.h"
#include<algorithm>

namespace PoittersPoint {


float PoittersPoint_Timer::time_ = 0.0f;
// 制限時間の上限(2分)
float PoittersPoint_Timer::max_time_ = 120.0f;

bool PoittersPoint_Timer::is_paused_ = false;

//! @brief 初期化
//! @return 初期化済み
bool PoittersPoint_Timer::Init()
{
    // 最初に1回動作する
    // ただし trueを返さなければ Initに何回も来る仕様。
    time_ = 0.0f;
    time_ = max_time_;    // 制限時間の初期化
    is_paused_ = false;

    return true;
}

void PoittersPoint_Timer::Update()
{
    // 一時停止中なら更新しない
    if(is_paused_) {
        return;
    }

    time_ -= GetDeltaTime();
    if(time_ < 0.0f) {
        time_ = 0.0f;
    }
}

float PoittersPoint_Timer::GetTimer()
{
    return time_;
}

// タイムアップ判定
bool PoittersPoint_Timer::IsTimeUp()
{
    return time_ <= 0.0f;
}

// 時間の加算
void PoittersPoint_Timer::AddTime(float sec)
{
    // 上限を超えないように
    time_ = std::min(time_ + sec, max_time_);
}

// 時間の減算
void PoittersPoint_Timer::SubTime(float sec)
{
    // 0秒未満にならないように
    time_ = std::max(time_ - sec, 0.0f);
}

void PoittersPoint_Timer::SetPause(bool is_pause)
{
    is_paused_ = is_pause;
}

void PoittersPoint_Timer::SetTime(float time)
{
    // マイナス値にならないように
    time_ = std::max(0.0f, time);
}

void PoittersPoint_Timer::SetMaxTime(float max_time)
{
    max_time_ = std::max(0.0f, max_time);

    // 現在の残り時間が新しい上限を超えていたら切り詰める
    if(time_ > max_time_) {
        time_ = max_time_;
    }
}

bool PoittersPoint_Timer::IsPaused()
{
    return is_paused_;
}

void PoittersPoint_Timer::GUI()
{
}

}    // namespace PoittersPoint
