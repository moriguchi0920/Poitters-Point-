#pragma once
//---------------------------------------------------------------------------
//! @file   ComponentPlayerState.cpp
//! @brief  プレイヤー状態制御コンポーネント
//---------------------------------------------------------------------------
#include <Game/Component/ComponentPlayerState.h>
#include <Game/Component/ComponentStateControllerWalk.h>
#include <Game/Component/ComponentStateGrab.h>
#include <Game/Component/ComponentStateThrow.h>
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

    if (auto grabbable = owner->GetComponent<ComponentGrabbable>())
    {
        if (grabbable->GetIsGrabbed())
        {
            return;
        }
    }

    // スペースキーが押された時の処理
    if(Input::IsKeyDown(KEY_INPUT_SPACE)) {
        // 現在のステートがControllerWalkであり、持っているオブジェクトが存在していて投げられるとき
        if(owner->GetComponent<ComponentStateControllerWalk>()) {
            if(!grabbing_object_ptr_.expired()) {
                //(投げ可能 = すでにアイテムを持っている)
                if(can_throw_) {
                    // 投げ判定変数をfalseに
                    can_throw_ = false;
                    // 持ち上げ可能に
                    can_grab_ = true;
                    // Throwステートに変更
                    ChangeState<ComponentStateThrow>()->SetThrowObject(grabbing_object_ptr_);
                    grabbing_object_ptr_.reset();
                }
                // (投げ不可能 = アイテムはまだ持っていない)
                else {
                    StartGrab();
                }
            }
        }
    }

    // 現在のステートが掴みであるとき
    if(FinishGrab()) {
        if(character_casted_owner) {
            ChangeState<ComponentStateControllerWalk>()->SetMoveSpeed(character_casted_owner->GetMoveSpeed())->SetIsHolding(true);
        }
    }

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
            // 共通部分(共通化したい)

            // 有効/無効
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable))
                SetStatus(StatusBit::Enable, enable);

            // GUI上でオーナーから自分(SampleObjectController)を削除します
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
