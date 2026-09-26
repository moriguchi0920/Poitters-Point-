#pragma once
#include <Game/Component/StateMachine/ComponentStateMachine.h>
#include "Game/Scene/PoittersPoint_Stage.h"
#include <Game/Component/ComponentGrabbable.h>
#include<Game/Component/State/ComponentStateGrab.h>

void ComponentStateMachine::Init()
{
    __super::Init();
}

void ComponentStateMachine::Update()
{
    __super::Update();
}

void ComponentStateMachine::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("State Machine")) {
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

const std::string ComponentStateMachine::GetStateName() const
{
    if(auto state = GetOwner()->GetComponent<ComponentState>()) {
        return state->GetName().data();
    }
    return "";
}

bool ComponentStateMachine::GrabbableHit(ObjectPtr target)
{
    if(auto owner = GetOwner()) {
        if(auto grabbable = target->GetComponent<ComponentGrabbable>()) {
            if(grabbable->GetCanGrab() && can_grab_) {
                grabbing_object_ptr_ = target;
                return true;
            }
        }
    }
    return false;
}

bool ComponentStateMachine::StartGrab(bool transition)
{
    if(!grabbing_object_ptr_.expired()) {
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
                return true;
            }
            else {
                return false;
            }
        }
    }
    return false;
}

bool ComponentStateMachine::FinishGrab()
{
    auto owner = GetOwner();
    // 現在のステートが掴みであるとき
    if(auto component_grab = owner->GetComponent<ComponentStateGrab>()) {
        // 掴みモーションが終わったら
        if(component_grab->GetIsFinished()) {
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
            return true;
        }
    }
    return false;
}



void ComponentStateMachine::ReleaseGrabbingObj()
{
    grabbing_object_ptr_.reset();
}

bool ComponentStateMachine::GetCanGrab()
{
    return can_grab_;
}

bool ComponentStateMachine::GetGrabbing()
{
    return !grabbing_object_ptr_.expired() && !can_grab_;
}

CEREAL_REGISTER_TYPE(ComponentStateMachine)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateMachine)
