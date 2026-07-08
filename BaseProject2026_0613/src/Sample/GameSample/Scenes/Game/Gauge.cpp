#include "Gauge.h"
#include <System/Scene.h>
#include <System/SystemMain.h>
#include <DxLib.h>

namespace Sample::GameSample {
namespace HP {
//! @brief HPゲージ
constexpr const int BAR_W        = 320;
constexpr const int BAR_H        = 50;
constexpr const int BAR_MARGIN_R = 20;

const u32 BAR_BASE_COLOR    = GetColor(75, 75, 75);
const u32 BAR_DAMAGE_COLOR  = RED;
const u32 BAR_RECOVER_COLOR = GREEN;
const u32 BAR_NORMAL_COLOR  = YELLOW;
const u32 BAR_SIDE_COLOR    = WHITE;

//! @brief ゲージが動作するまでの秒数
constexpr const float BAR_PROC_TIME   = 0.5f;
constexpr const float BAR_UPDOWN_TIME = 2.0f;

//! @brief HPゲージ
Gauge::Gauge(float hp)
    : hp_(hp)
    , damage_(hp)
    , org_hp_(hp)
{
    // 位置
    pos_ = float3(WINDOW_W - (BAR_W + BAR_MARGIN_R), 0, 0);
}

Gauge::~Gauge()
{
}

void Gauge::Update()
{
    float delta = GetDeltaTime();
    procDamageGauge(delta);
    procRecoverGauge(delta);
}

void Gauge::Draw()
{
    int x = static_cast<int>(pos_.x);
    int y = static_cast<int>(pos_.z);

    // 下地
    DrawBox(x, y, x + BAR_W, y + BAR_H, BAR_BASE_COLOR, TRUE);

    // ダメージ用
    DrawBox(x, y, x + static_cast<int>(BAR_W * ToPerValue(damage_)), y + BAR_H, BAR_DAMAGE_COLOR, TRUE);

    if(recover_delta_) {
        // 回復用
        DrawBox(x, y, x + static_cast<int>(BAR_W * ToPerValue(recover_)), y + BAR_H, BAR_NORMAL_COLOR, TRUE);
        // 現在のHP
        DrawBox(x, y, x + static_cast<int>(BAR_W * ToPerValue(hp_)), y + BAR_H, BAR_RECOVER_COLOR, TRUE);
    }
    else {
        // 現在のHP
        DrawBox(x, y, x + static_cast<int>(BAR_W * ToPerValue(hp_)), y + BAR_H, BAR_NORMAL_COLOR, TRUE);
    }

    // フチ
    DrawBox(x, y, x + BAR_W, y + BAR_H, BAR_SIDE_COLOR, FALSE);
}

// 全回復
void Gauge::Reset()
{
    hp_     = org_hp_;
    damage_ = hp_;
}

Gauge& Gauge::operator++()
{
    *this += 1;
    return *this;
}

Gauge Gauge::operator++(int)
{
    Gauge ret  = *this;
    *this     += 1;
    return ret;
}

Gauge& Gauge::operator--()
{
    *this -= 1;
    return *this;
}

Gauge Gauge::operator--(int)
{
    Gauge ret  = *this;
    *this     -= 1;
    return ret;
}

void Gauge::operator-=(int value)
{
    if(hp_ <= 0)
        return;

    // 減らせるダメージがある場合のみ以下を実行
    damage_frame_  = BAR_PROC_TIME;
    hp_           -= value;
    if(hp_ <= 0)
        hp_ = 0;

    // 赤ゲージを消す増加量を計算
    float delta = GetDeltaTime();
    assert(delta > 0.0f && BAR_UPDOWN_TIME > 0);
    float delta_up = delta / BAR_UPDOWN_TIME;

    damage_delta_ = static_cast<float>(damage_ - hp_) * delta_up;

    // 増えている最中に減る場合は量は一瞬で同じにしておく
    recover_       = hp_;
    recover_delta_ = 0.0f;
}

void Gauge::operator+=(int value)
{
    if(hp_ >= org_hp_)
        return;

    // 増やせる分がある場合のみ以下を実行
    hp_ += value;
    if(hp_ >= org_hp_)
        hp_ = org_hp_;

    // 緑ゲージ量の計算
    float delta = GetDeltaTime();
    assert(delta > 0.0f && BAR_UPDOWN_TIME > 0);
    float delta_up = delta / BAR_UPDOWN_TIME;

    recover_delta_ = static_cast<float>(hp_ - recover_) * delta_up;
}

bool Gauge::operator<=(int value)
{
    return hp_ <= value;
}

bool Gauge::operator<(int value)
{
    return hp_ < value;
}

bool Gauge::operator>=(int value)
{
    return hp_ >= value;
}

bool Gauge::operator>(int value)
{
    return hp_ > value;
}

bool Gauge::operator==(int value)
{
    return hp_ == value;
}

bool Gauge::operator!=(int value)
{
    return !((*this) == value);
}

float Gauge::ToPerValue(float value)
{
    float per = (value / org_hp_);
    return per;
}

void Gauge::procDamageGauge(float delta)
{
    // ダメージ分をちょっとだけ見せてから減らす処理する
    damage_frame_ -= delta;
    if(damage_frame_ > 0)
        return;

    // 通常ゲージに徐々に近づける
    damage_ -= damage_delta_;
    if(damage_ <= hp_) {
        damage_       = hp_;
        damage_delta_ = 0.0f;
    }
}

void Gauge::procRecoverGauge(float delta)
{
    // リカバー分をちょっとだけ見せてから減らす処理する
    recover_frame_ -= delta;
    if(recover_frame_ > 0)
        return;

    // 通常ゲージに徐々に近づける
    recover_ += recover_delta_;
    if(recover_ >= hp_) {
        recover_       = hp_;
        recover_delta_ = 0.0f;
    }
}

}    // namespace HP
}    // namespace Sample::GameSample
