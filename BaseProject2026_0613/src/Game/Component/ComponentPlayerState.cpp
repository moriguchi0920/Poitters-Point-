#pragma once
//---------------------------------------------------------------------------
//! @file   ComponentPlayerState.cpp
//! @brief  プレイヤー状態制御コンポーネント
//---------------------------------------------------------------------------
#include <Game/Component/ComponentPlayerState.h>
#include "ComponentStateIdleWalk.h"
#include "ComponentStateGrab.h"
#include "ComponentStateThrow.h"
#include "ComponentGrabbable.h"
#include "ComponentHitPoints.h"
#include "ComponentStatePlayerJump.h"

//===========================================================================
// 初期化処理
//===========================================================================
void ComponentPlayerState::Init()
{
    __super::Init();

    auto owner = GetOwner();

    // オブジェクトの制御を行うコンポーネントを追加
    owner->AddComponent<ComponentStateIdleWalk>();

    // オブジェクトの体力を管理するコンポーネントを追加
    owner->AddComponent<ComponentHitPoints>()->SetMaxAndCurrentHP(5.0f);

    can_grab_ = true;
}

void ComponentPlayerState::Update()
{
    __super::Update();

    auto owner = GetOwner();

    // Eキーが押された時の処理
    if(Input::IsKeyDown(KEY_INPUT_E)) {
        // オブジェクトが近くにある、または持っている状態
        if(!grabbing_object_ptr_.expired()) {
            // 【投げ処理】すでにアイテムを持っている (can_throw_ が true)
            if(can_throw_) {
                bool is_idle_walk = (owner->GetComponent<ComponentStateIdleWalk>() != nullptr);

                // ジャンプステートの取得
                auto jump_comp  = owner->GetComponent<ComponentStatePlayerJump>();
                bool is_jumping = (jump_comp != nullptr);

                // 地上(IdleWalk) または 空中(PlayerJump) のどちらでも投げを許可
                if(is_idle_walk || is_jumping) {
                    can_throw_ = false;
                    can_grab_  = true;

                    // Throwステートに変更
                    auto state_throw = ChangeState<ComponentStateThrow>();
                    state_throw->SetThrowObject(grabbing_object_ptr_);

                    // ★空中投げ（ジャンプ中）の場合は is_in_air_ を true にして SetInAir を呼ぶ
                    if(is_jumping) {
                        float v_y      = jump_comp->GetVelocityY();
                        float ground_y = jump_comp->GetGroundY();
                        float gravity  = jump_comp->GetGravity();

                        state_throw->SetInAir(true, v_y, ground_y, gravity);
                    }

                    grabbing_object_ptr_.reset();
                }
            }
            // 【持ち上げ処理】まだアイテムを持っていない (地上 IdleWalk のみ許可)
            else if(owner->GetComponent<ComponentStateIdleWalk>()) {
                auto grabbable = grabbing_object_ptr_.lock()->GetComponent<ComponentGrabbable>();
                if(grabbable) {
                    if(grabbable->GetCanGrab()) {
                        ChangeState<ComponentStateGrab>()->SetLiftTime(grabbable->GetLiftTime());
                        can_grab_ = false;
                        grabbable->SetCanGrab(false);
                    }
                }
            }
        }
    }

    // 掴みアニメーションが終了したら、掴み状態を完了させる
    if(auto component_grab = owner->GetComponent<ComponentStateGrab>()) {
        if(component_grab->GetFinished()) {
            if(!grabbing_object_ptr_.expired()) {
                auto object = grabbing_object_ptr_.lock();

                // 掴んでいる間は当たり判定を無効化
                if(auto collider = object->GetComponent<ComponentCollision>()) {
                    collider->SetCollisionStatus(ComponentCollision::CollisionBit::DisableHit, true);
                }

                // 掴み状態に設定
                auto grabbable = object->GetComponent<ComponentGrabbable>();
                grabbable->SetIsGrabbed(true);

                // ヒット記録をクリア
                grabbable->ClearHitTargets();

                // 手にモデルをアタッチ
                grabbing_object_ptr_.lock()->AddComponent<ComponentAttachModel>()->SetAttachObject(owner->GetName(), "mixamorig:RightHand");
            }

            // 掴み状態が完了したら、投げ可能にする
            can_throw_ = true;
            ChangeState<ComponentStateIdleWalk>()->SetIsHolding(true);
        }
    }
}

//===========================================================================
// 更新後処理
//===========================================================================
void ComponentPlayerState::LateUpdate()
{
}

//===========================================================================
// GUI処理
//===========================================================================
void ComponentPlayerState::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("Player State")) {
            //-------------------------------------------------------
            // 共通部分

            // 有効/無効
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable))
                SetStatus(StatusBit::Enable, enable);

            // GUI上でオーナーから自分を削除します
            if(ImGui::Button(u8"削除"))
                GetOwner()->RemoveComponent(shared_from_this());
            //-------------------------------------------------------

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

//===========================================================================
// 掴める物を検知した時の判定処理
//===========================================================================
void ComponentPlayerState::GrabbableHit(ObjectPtr target)
{
    auto owner = GetOwner();
    if(owner->GetComponent<ComponentStateIdleWalk>()) {
        grabbing_object_ptr_ = target;
    }
}

CEREAL_REGISTER_TYPE(ComponentPlayerState)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentPlayerState)
