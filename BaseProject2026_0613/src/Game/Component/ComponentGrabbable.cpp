#pragma once
//---------------------------------------------------------------------------
//! @file   ComponentGrabbable.cpp
//! @brief  掴み可能コンポーネント(投げ物・キャラクター共通)
//---------------------------------------------------------------------------
#include <Game/Component/ComponentGrabbable.h>
#include <Game/Component/ComponentStateControllerWalk.h>
#include <System/Component/ComponentAttachModel.h>    // 掴み中の追従（手の位置）取得用
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

    // Jolt 剛体を持つか（Rock/Log/Slime 等の物理を Jolt に載せ替えたオブジェクトか）
    if(auto physics = owner->GetComponent<ComponentPhysics>()) {
        body_ = physics->GetRigidBody();
    }
    else {
        body_ = nullptr;
    }
}

//============================================================================
// 更新処理
//============================================================================
void ComponentGrabbable::Update()
{
    __super::Update();
    auto owner = GetOwner();

    if(body_) {
        // Jolt 対応：位置・速度は剛体が担うので translation_ を適用しない。
        // 掴み可否だけ更新する（移動中は掴めない）。
        if(IsMoving()) {
            can_grab_this_ = false;
        }
        else if(!is_grabbed_) {
            can_grab_this_ = true;
        }
        return;
    }

    //--- 旧来パス（未移行：ComponentCollision 自作重力＋translation_ 慣性）---
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
// 物理シミュレーション前処理
//============================================================================
void ComponentGrabbable::PrePhysics()
{
    __super::PrePhysics();

    // Jolt 対応：掴み中は Kinematic（静的追従）にして手の位置へ持っていく。
    // 揺れ・物理的な吊り下げは一旦行わず、 AttachModel の手の位置へ等速で寄せる（要求どおり揺れなし）。
    if(!body_ || !is_grabbed_)
        return;

    body_->setMotionType(physics::MotionType::Kinematic, true);

    if(auto attach = GetOwner()->GetComponent<ComponentAttachModel>()) {
        matrix hand_mat = attach->GetPutOnMatrix();
        body_->moveKinematic(hand_mat.translate(), body_->rotation(), GetDeltaTime());
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

            // Jolt 対応の閾値（掴み中の追従では使わない。移動中/静止の判定だけ）
            if(body_)
                ImGui::DragFloat(u8"移動中しきい値", &move_speed_threshold_, 1.0f, 1.0f, 200.0f, "%.1f");

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
    if(body_) {
        // Jolt 対応：速度がほぼゼロなら「静止＝接地扱い」とみなす。
        // （本物の接地判定は物理内部状態だが、掴み可否・SE 用途には静止近似で足る）
        float3 v  = body_->linearVelocity();
        float  vx = (float)v.x, vy = (float)v.y, vz = (float)v.z;
        return (vx * vx + vy * vy + vz * vz) < (move_speed_threshold_ * move_speed_threshold_);
    }
    return bounce_offset_ < 0.0f;
}

//============================================================================
// 移動中かどうかを取得
//============================================================================
bool ComponentGrabbable::IsMoving()
{
    if(body_) {
        float3 v  = body_->linearVelocity();
        float  vx = (float)v.x, vz = (float)v.z;
        return (vx * vx + vz * vz) > (move_speed_threshold_ * move_speed_threshold_);
    }
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
    return (int)hit_targets_.count(target) > 0;
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
