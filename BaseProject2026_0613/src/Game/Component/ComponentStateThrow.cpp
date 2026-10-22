#pragma once
#include <Game/Component/ComponentStateThrow.h>
#include "ComponentStateIdleWalk.h"
#include "Game/Scene/PoittersPoint_Stage.h"
#include <Game/Component/ComponentGrabbable.h>

void ComponentStateThrow::Init()
{
    __super::Init();

    SetName<Component>("State Throw");
    auto owner = GetOwner();

    is_released_ = false;

    if(auto model = owner->GetComponent<ComponentModel>()) {
        if(is_in_air_) {
            // ★空中投げ：ThrowFast を上半身のみ 1.5倍速で再生
            // PlayAnimationEx の引数（name, node, loop, blend_time, start_time）
            model->PlayAnimationEx("throw_fast", "mixamorig:Spine", false);
        }
        else {
            // ★地上投げ：通常版（1.0倍速）を全身で再生
            model->PlayAnimationNoSame("throw", false);
        }
    }
}

void ComponentStateThrow::Update()
{
    __super::Update();

    auto owner            = GetOwner();
    bool was_in_air_start = is_in_air_;

    // ★1. 空中投げの移動処理（カプセル本体の物理移動）
    if(is_in_air_) {
        float effective_gravity = is_released_ ? gravity_ : (gravity_ * 0.1f);

        owner->AddTranslate({0.0f, current_velocity_, 0.0f}, true);
        current_velocity_ -= effective_gravity;

        // ★投げ切るまでは着地しない（ジャンプ＋投げを保つ）
        if(is_released_) {
            auto pos = owner->GetTranslate();
            if(pos.y <= ground_y_) {
                pos.y = ground_y_;
                owner->SetTranslate(pos);
                is_in_air_ = false;    // 着地
            }
        }
    }

    if(auto model = owner->GetComponent<ComponentModel>()) {
        // ★2. 空中投げ：モデルのY座標を固定（ルートモーションの打ち消し）
        if(was_in_air_start) {
            matrix mat        = model->GetMatrix();
            mat.translate().y = 0.0f;
            model->SetMatrix(mat);
        }

        // 手離し判定閾値（1.5倍速の ThrowFast に合わせて手離しタイミングも 0.20f 付近に調整）
        float release_threshold = was_in_air_start ? 0.20f : 0.65f;

        // 手を離す処理
        if(!is_released_ && release_threshold <= model->GetAnimationTime()) {
            if(auto throw_object = throw_weak_ptr_.lock()) {
                throw_object->RemoveComponent<ComponentAttachModel>();

                auto grabbable = throw_object->GetComponent<ComponentGrabbable>();
                grabbable->SetIsGrabbed(false);

                float3 vec = {0.0f, 1.0f, 0.0f};
                if(auto model = owner->GetComponent<ComponentModel>()) {
                    constexpr float front_len = 5.0f;
                    vec                       = -(model->GetWorldVectorAxisZ() * front_len);
                }

                throw_object->AddTranslate(vec * 5.0f);
                grabbable->SetTranslation(vec * 5.0f);

                if(auto collider = throw_object->GetComponent<ComponentCollision>()) {
                    collider->SetCollisionStatus(ComponentCollision::CollisionBit::DisableHit, false);
                }

                grabbable->SetThrower(GetOwnerPtr());

                throw_weak_ptr_.reset();
                is_released_ = true;
            }
        }

        // アニメーション再生終了時のクリーンアップ処理
        bool is_playing = was_in_air_start ? model->IsExPlaying("mixamorig:Spine") : model->IsPlaying();

        if(!is_playing) {
            matrix mat        = model->GetMatrix();
            mat.translate().y = 0.0f;
            model->SetMatrix(mat);

            RemoveThisComponent();
            auto idle = owner->AddComponent<ComponentStateIdleWalk>();
            idle->SetMoveSpeed(0.3f)->SetRotateSpeed(20.0f);
        }
    }
}

void ComponentStateThrow::GUI()
{
    __super::GUI();

    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode("State Throw")) {
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable))
                SetStatus(StatusBit::Enable, enable);

            if(ImGui::Button(u8"削除"))
                GetOwner()->RemoveComponent(shared_from_this());

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ComponentStateThrow::SetThrowObject(ObjectWeakPtr object_ptr)
{
    throw_weak_ptr_ = object_ptr;
}

CEREAL_REGISTER_TYPE(ComponentStateThrow)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateThrow)
