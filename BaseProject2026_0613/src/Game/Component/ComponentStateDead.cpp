//---------------------------------------------------------------------------
//! @file   ComponentStateDead.cpp
//! @brief  死亡状態コンポーネント
//---------------------------------------------------------------------------
#include <Game/Component/ComponentStateDead.h>
#include "Game/Scene/PoittersPoint_Stage.h"

#include <algorithm>

//! @brief 初期化
//! 死亡アニメーションを再生する
void ComponentStateDead::Init()
{
    __super::Init();

    SetName<Component>("State Dead");

    auto owner = GetOwner();

    if(auto model = owner->GetComponent<ComponentModel>()) {
        if(use_animation_) {
            model->PlayAnimationNoSame("dead", false, 0.1f);
        }
        else {
            // アニメーションを使わない場合は待機モーションのまま倒す
            model->PlayAnimationNoSame("idle", true, 0.1f);
            start_rot_ = model->GetRotationAxisXYZ();
        }
    }
    timer_ = 0.0f;
}

//! @brief 更新
//! アニメーション終了後に死亡処理を実行し、自身を削除する
void ComponentStateDead::Update()
{
    __super::Update();

    auto owner = GetOwner();

    //-----------------------------------------------------------
    // "dead"アニメーションで演出する場合(Enemy)
    //-----------------------------------------------------------
    if(use_animation_) {
        if(auto model = owner->GetComponent<ComponentModel>()) {
            if(!model->IsPlaying()) {
                // 死亡処理：アニメ終了後に実行
                // 将来的にはCharacterクラスで共通化予定
                FinishDead();
            }
        }
        return;
    }

    //-----------------------------------------------------------
    // アニメーションを使わずにモデルを倒して演出する場合(Player)
    //-----------------------------------------------------------
    timer_ += GetDeltaTime();

    // 0 → 1 に進む倒れ具合
    float t = std::clamp(timer_ / fall_time_, 0.0f, 1.0f);
    // 最初はゆっくり、だんだん速く倒れる(t^2 で加速させる)
    float ease = t * t;

    if(auto model = owner->GetComponent<ComponentModel>()) {
        // モデルのX軸回転を足して後ろ向きに倒す(Y軸の向きはそのまま)
        model->SetRotationAxisXYZ({start_rot_.x - fall_angle_ * ease, start_rot_.y, start_rot_.z});
    }

    // 倒れきってから少し待って死亡処理
    if(fall_time_ + wait_time_ <= timer_) {
        FinishDead();
    }
}

//! @brief 死亡処理
void ComponentStateDead::FinishDead()
{
    auto owner = GetOwner();

    owner->SetName("dead");

    if(auto col = owner->GetComponent<ComponentCollisionCapsule>()) {
        owner->RemoveComponent<ComponentCollisionCapsule>();
    }

    RemoveThisComponent();
}

//! @brief GUI
void ComponentStateDead::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("State Dead")) {
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

            ImGui::DragFloat(u8"倒れる時間", &fall_time_, 0.01f, 0.05f, 5.0f, "%.2f");
            ImGui::DragFloat(u8"倒れる角度", &fall_angle_, 1.0f, 0.0f, 180.0f, "%.1f");

            //if(ImGui::TreeNode("State IdleWalk")) とセット
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentStateDead)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateDead)
