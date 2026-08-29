#pragma once
//---------------------------------------------------------------------------
//! @file   ComponentGrabbable.cpp
//! @brief  掴み可能コンポーネント(投げ物・キャラクター共通)
//---------------------------------------------------------------------------
#include <Game/Component/ComponentGrabbable.h>
#include "ComponentStateIdleWalk.h"
#include "Game/Scene/PoittersPoint_Stage.h"

//============================================================================
// 初期化処理
//============================================================================
void ComponentGrabbable::Init()
{
    __super::Init();

    // GetOnwer：オーナー(自分がAddComponentされたObject)を取得します
    // 処理されるときは必ずOwnerは存在しますので基本的にnullptrチェックは必要ありません
    auto owner     = GetOwner();
    is_grabbed_    = false;
    can_grab_this_ = true;
}

//============================================================================
// 更新処理
//============================================================================
void ComponentGrabbable::Update()
{
    __super::Update();
    auto owner = GetOwner();
    if(!is_grabbed_) {
        // 移動量をオーナーに適用
        owner->AddTranslate(translation_);
        // 移動量を減衰させて停止させる
        translation_ = lerp(translation_, float3(0.0f, 0.0f, 0.0f), 0.03);
    }
    if(IsMoving()) {
        // 移動中は掴み不可
        can_grab_this_ = false;
    }
    else if(!is_grabbed_) {
        // 停止中は掴み可能
        can_grab_this_ = true;
    }
}

//============================================================================
// GUI処理
//============================================================================
void ComponentGrabbable::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("Grabbable")) {
            //-------------------------------------------------------
            // 共通部分(共通化したい)

            // 有効/無効
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable))
                SetStatus(StatusBit::Enable, enable);

            // GUI上でオーナーから自分(SampleObjectController)を削除します
            if(ImGui::Button(u8"削除"))
                GetOwner()->RemoveComponent(shared_from_this());
            //-------------------------------------------------------

            //if(ImGui::TreeNode("State IdleWalk")) とセット
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

//============================================================================
// リフト時間を設定
//============================================================================
void ComponentGrabbable::SetLiftTime(float time)
{
    lift_time_ = time;
}

//============================================================================
// リフト時間を取得
//============================================================================
const float& ComponentGrabbable::GetLiftTime() const
{
    return lift_time_;
}

//============================================================================
// 移動量を設定
//============================================================================
void ComponentGrabbable::SetTranslation(float3 new_translation)
{
    translation_ = new_translation;
}

//============================================================================
// 移動量を加算
//============================================================================
void ComponentGrabbable::AddTranslation(float3 add_translation)
{
    translation_ += add_translation;
}

//============================================================================
// 移動量を取得
//============================================================================
const float3& ComponentGrabbable::GetTranslation()
{
    return translation_;
}

//============================================================================
// 掴まれているかどうかを設定
//============================================================================
void ComponentGrabbable::SetIsGrabbed(bool grabbed)
{
    is_grabbed_ = grabbed;
}

//============================================================================
// 掴まれているかどうかを取得
//============================================================================
bool ComponentGrabbable::GetIsGrabbed()
{
    return is_grabbed_;
}

//============================================================================
// 掴むことができるかどうかを取得
//============================================================================
bool ComponentGrabbable::GetCanGrab()
{
    return can_grab_this_;
}

//============================================================================
// 掴むことができるかどうかを設定
//============================================================================
void ComponentGrabbable::SetCanGrab(const bool can_grab)
{
    can_grab_this_ = can_grab;
}

//============================================================================
// バウンスオフセットを設定
//============================================================================
void ComponentGrabbable::SetBounceOffset(float offset)
{
    bounce_offset_ = offset;
}

//============================================================================
// バウンスの実行
//============================================================================
void ComponentGrabbable::Bounce()
{
    AddTranslation({0.0f, bounce_offset_, 0.0f});
    bounce_offset_ -= 0.03f;
}

//============================================================================
// 地面に接地しているかどうかを取得
//============================================================================
bool ComponentGrabbable::IsGrounded()
{
    return bounce_offset_ < 0.0f;
}

//============================================================================
// 移動中かどうかを取得(XZ成分の絶対値が閾値を超えているか)
//============================================================================
bool ComponentGrabbable::IsMoving()
{
    return ((translation_.x < -0.01f || 0.01f < translation_.x) || (translation_.z < -0.01f || 0.01f < translation_.z));
}

//============================================================================
// ダメージ値を設定(投げ物ごとに異なるダメージを持たせる)
//============================================================================
void ComponentGrabbable::SetDamage(float damage)
{
    damage_ = damage;
}

//============================================================================
// ダメージ値を取得
//============================================================================
float ComponentGrabbable::GetDamage() const
{
    return damage_;
}

//============================================================================
// ヒット記録をクリア(一応投げ直した時に呼ぶ)
//============================================================================
void ComponentGrabbable::ClearHitTargets()
{
    //set.clear：要素をすべて削除する
    hit_targets_.clear();
}

//============================================================================
// 既にヒットしたターゲットかどうかを判定
//============================================================================
bool ComponentGrabbable::IsAlreadyHit(ObjectPtr target) const
{
    // set.count：値が含まれている個数を返す
    // setは重複を許さないので、結果は必ず1（存在する）か0（存在しない）になる
    return hit_targets_.count(target) > 0;
}

//============================================================================
// ヒット記録に追加
//============================================================================
void ComponentGrabbable::AddHitTarget(ObjectPtr target)
{
    // set.insert：値を追加する
    // setは重複を許さないので、すでに存在する場合は追加されない
    hit_targets_.insert(target);
}

void ComponentGrabbable::SetThrower(ObjectPtr thrower)
{
    thrower_ = thrower;
}

bool ComponentGrabbable::IsThrower(ObjectPtr target) const
{
    return thrower_.lock() == target;
}

CEREAL_REGISTER_TYPE(ComponentGrabbable)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentGrabbable)
