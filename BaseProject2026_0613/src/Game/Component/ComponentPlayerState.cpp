#pragma once
#include <Game/Component/ComponentPlayerState.h>
#include "ComponentStateIdleWalk.h"
#include "ComponentStateGrab.h"
#include "ComponentStateThrow.h"
#include "ComponentGrabbable.h"
#include "ComponentHitPoints.h"

void ComponentPlayerState::Init()
{
    __super::Init();

    auto owner = GetOwner();

    // オブジェクトの制御を行うコンポーネントを追加
    owner->AddComponent<ComponentStateIdleWalk>()->SetMoveSpeed(0.3f)->SetRotateSpeed(20.0f);

    owner->AddComponent<ComponentHitPoints>()->SetMaxAndCurrentHP(5.0f);

    can_grab_ = true;
}

void ComponentPlayerState::Update()
{
    __super::Update();

    auto owner = GetOwner();

    if(Input::IsKeyDown(KEY_INPUT_SPACE)) {
        if(owner->GetComponent<ComponentStateIdleWalk>() && !grabbing_object_ptr_.expired() && can_throw_) {
            can_throw_ = false;
            can_grab_  = true;
            ChangeState<ComponentStateThrow>()->SetThrowObject(grabbing_object_ptr_);
        }
    }

    if(Input::IsKeyDown(KEY_INPUT_SPACE)) {
        if(owner->GetComponent<ComponentStateIdleWalk>()) {
            if(!grabbing_object_ptr_.expired()) {
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

    if(auto component_grab = owner->GetComponent<ComponentStateGrab>()) {
        if(component_grab->GetFinished()) {
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
            ChangeState<ComponentStateIdleWalk>()->SetIsHolding(true);
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

void ComponentPlayerState::GrabbableHit(ObjectPtr target)
{
    auto owner = GetOwner();
    if(owner->GetComponent<ComponentStateIdleWalk>()) {
        grabbing_object_ptr_ = target;
    }
}

CEREAL_REGISTER_TYPE(ComponentPlayerState)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentPlayerState)
