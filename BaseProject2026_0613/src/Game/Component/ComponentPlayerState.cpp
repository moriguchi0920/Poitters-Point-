#pragma once
//---------------------------------------------------------------------------
//! @file   ComponentPlayerState.cpp
//! @brief  プレイヤー状態制御コンポーネント
//---------------------------------------------------------------------------
#include <Game/Component/ComponentPlayerState.h>
#include <Game/Component/ComponentStateControllerWalk.h>
#include <Game/Component/ComponentStateGrab.h>
#include <Game/Component/ComponentStateThrow.h>
#include <Game/Component/ComponentStatePlayerJump.h>
#include <Game/Component/ComponentGrabbable.h>
#include <Game/Object/PoittersPoint_Player.h>

//===========================================================================
// 初期化処理
//===========================================================================
void ComponentPlayerState::Init()
{
    __super::Init();
    auto owner                  = GetOwner();
    auto character_casted_owner = dynamic_cast<PoittersPoint::Player*>(owner);

    // オブジェクトの制御を行うコンポーネントを追加
    if(character_casted_owner) {
        owner->AddComponent<ComponentStateControllerWalk>()->SetMoveSpeed(character_casted_owner->GetMoveSpeed());
    }

    can_grab_ = true;
}

//===========================================================================
// 更新処理
//===========================================================================
void ComponentPlayerState::Update()
{
    __super::Update();

    auto owner                  = GetOwner();
    auto character_casted_owner = dynamic_cast<PoittersPoint::Player*>(owner);

    if(auto grabbable = owner->GetComponent<ComponentGrabbable>()) {
        if(grabbable->GetIsGrabbed()) {
            return;
        }
    }

    // Eキーが押された時の処理
    if(Input::IsKeyDown(KEY_INPUT_E)) {
        // オブジェクトが近くにある、または持っている状態
        if(!grabbing_object_ptr_.expired()) {
            // 【投げ処理】すでにアイテムを持っている (can_throw_ が true)
            if(can_throw_) {
                bool is_walk = (owner->GetComponent<ComponentStateControllerWalk>() != nullptr);

                // ジャンプステートの取得
                auto jump_comp  = owner->GetComponent<ComponentStatePlayerJump>();
                bool is_jumping = (jump_comp != nullptr);

                // 地上(ControllerWalk) または 空中(PlayerJump) のどちらでも投げを許可
                if(is_walk || is_jumping) {
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
        }
        // 【持ち上げ処理】まだアイテムを持っていない (地上 ControllerWalk のみ許可)
        else if(owner->GetComponent<ComponentStateControllerWalk>()) {
            StartGrab();
        }
    }

    // 掴みアニメーションが終了したら、掴み状態を完了させて歩きステートに戻る
    if(FinishGrab()) {
        if(character_casted_owner) {
            ChangeState<ComponentStateControllerWalk>()->SetMoveSpeed(character_casted_owner->GetMoveSpeed())->SetIsHolding(true);
        }
    }

    // 投げアニメーションが終了したら歩きステートに戻る
    if(auto component_throw = owner->GetComponent<ComponentStateThrow>()) {
        if(component_throw->GetIsFinished()) {
            if(character_casted_owner) {
                ChangeState<ComponentStateControllerWalk>()->SetMoveSpeed(character_casted_owner->GetMoveSpeed());
            }
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

CEREAL_REGISTER_TYPE(ComponentPlayerState)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentPlayerState)
