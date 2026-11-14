#pragma once
#include <Game/Component/ComponentStateThrow.h>
#include "ComponentStateControllerWalk.h"
#include "Game/Scene/PoittersPoint_Stage.h"
#include <Game/Component/ComponentGrabbable.h>
#include <System/Component/ComponentPhysics.h>    // Jolt 剛体への発射（addLinearVelocity）用

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
                    // 正面ベクトル。モデルのワールド行列はスケール(0.11)や投げアニメーションの
                    // 揺れを内包するため、そのまま使すと「向いた方角によって初速の長さ・上向き成分が
                    // 変わる」原因になる。スケールを外して正規化し、水平成分のみで投げる。
                    vec = -model->GetWorldVectorAxisZ();
                    vec.y = 0.0f;    // 純水平発射。投球アニメーションの俯仰揺れに距離を左右させない
                    if((float)length(vec) < 0.0001f)
                        vec = -normalize(model->GetWorldVectorAxisZ());    // 真上/真下を向いていたらフルベクトルで代替
                    else
                        vec = normalize(vec);
                }

                if(auto physics = throw_object->GetComponent<ComponentPhysics>()) {
                    // Jolt 対応：Kinematic 追従から Dynamic に戻し、正面方向へ初速を付与して発射。
                    // 発射後の重力・慣性・着地・転がりは Jolt 側が担う。
                    if(auto rb = physics->GetRigidBody()) {
                        rb->setMotionType(physics::MotionType::Dynamic, true);
                        // addではなくset：掴み中の手の追従速度(Kinematic追従の残速)を消して
                        // 初速を throw_speed_ に一致させる。追従残速が距離・勢いのバラつきを生んでいた
                        rb->setLinearVelocity(vec * throw_speed_);
                    }
                }
                else {
                    // 旧来パス（未移行）：translation_ を速度として渡す
                    // （vec は従来 front_len=5 倍の長さだったが正規化されたので、同一の効果量 25 を掛ける）
                    throw_object->AddTranslate(vec * 25.0f);
                    grabbable->SetTranslation(vec * 25.0f);
                }

                if(auto collider = throw_object->GetComponent<ComponentCollision>()) {
                    collider->SetCollisionStatus(ComponentCollision::CollisionBit::DisableHit, false);
                }

                grabbable->SetThrower(GetOwnerPtr());

                throw_weak_ptr_.reset();
                is_released_ = true;
            }
        }

        // 空中投げのアニメーション終了時はルートモーションのY補正を解除する
        // (ステート遷移はステートマシン側の GetIsFinished ポーリングで行う)
        if(was_in_air_start && !model->IsExPlaying("mixamorig:Spine")) {
            matrix mat        = model->GetMatrix();
            mat.translate().y = 0.0f;
            model->SetMatrix(mat);
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

            ImGui::DragFloat(u8"Jolt投擲初速", &throw_speed_, 1.0f, 0.0f, 500.0f, "%.1f");

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ComponentStateThrow::SetThrowObject(ObjectWeakPtr object_ptr)
{
    throw_weak_ptr_ = object_ptr;
}

bool ComponentStateThrow::GetIsFinished()
{
    // アニメーションが止まっているか(空中投げは上半身レイヤーの再生終了も待つ)
    if(auto model = GetOwner()->GetComponent<ComponentModel>()) {
        return !model->IsPlaying() && !model->IsExPlaying("mixamorig:Spine");
    }
    return false;
}

CEREAL_REGISTER_TYPE(ComponentStateThrow)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateThrow)
