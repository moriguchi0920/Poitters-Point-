#pragma once
#include <Game/Component/StateMachine/ComponentPlayerState.h>
#include <Game/Component/State/ComponentStateControllerWalk.h>
#include <Game/Component/State/ComponentStateGrab.h>
#include <Game/Component/State/ComponentStateThrow.h>
#include <Game/Component/ComponentGrabbable.h>

void ComponentPlayerState::Init()
{
    __super::Init();

    // オブジェクトの制御を行うコンポーネントを追加
    GetOwner()->AddComponent<ComponentStateControllerWalk>();

    can_grab_ = true;
}

void ComponentPlayerState::Update()
{
    __super::Update();

    auto owner = GetOwner();

    // スペースキーが押された時の処理
    if(Input::IsKeyDown(KEY_INPUT_SPACE)) {
        // 現在のステートがIdleWalkであり、持っているオブジェクトが存在していて投げられるとき
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
                    // 持ち上げるオブジェクトのGrabbableコンポーネントを取得
                    auto grabbable = grabbing_object_ptr_.lock()->GetComponent<ComponentGrabbable>();
                    // コンポーネントがあったら
                    if(grabbable) {
                        // 持ち上げ相手が持てる状態なら
                        if(grabbable->GetCanGrab()) {
                            // ステートをGrabステートに
                            ChangeState<ComponentStateGrab>()->SetLiftTime(grabbable->GetLiftTime());
                            can_grab_ = false;
                            grabbable->SetCanGrab(false);
                        }
                    }
                }
            }
        }
    }

    // 現在のステートが掴みであるとき
    if(auto component_grab = owner->GetComponent<ComponentStateGrab>()) {
        // 掴みモーションが終わったら
        if(component_grab->GetFinished()) {
            // 掴みオブジェクトがある時
            if(!grabbing_object_ptr_.expired()) {
                auto object = grabbing_object_ptr_.lock();

                if(auto collider = object->GetComponent<ComponentCollision>()) {
                    collider->SetCollisionStatus(ComponentCollision::CollisionBit::DisableHit, true);
                }

                auto grabbable = object->GetComponent<ComponentGrabbable>();
                grabbable->SetIsGrabbed(true);

                grabbing_object_ptr_.lock()->AddComponent<ComponentAttachModel>()->SetAttachObject(owner->GetName(), "mixamorig:RightHand");
            }
            can_throw_ = true;
            ChangeState<ComponentStateControllerWalk>()->SetIsHolding(true);
        }
    }
}

void ComponentPlayerState::LateUpdate()
{
}

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
