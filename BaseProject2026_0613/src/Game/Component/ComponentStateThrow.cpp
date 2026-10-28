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
    if(auto model = owner->GetComponent<ComponentModel>()) {
        model->PlayAnimationNoSame("throw");
    }
}

void ComponentStateThrow::Update()
{
    __super::Update();

    // オーナー(自分がAddComponentされたObject)を取得します
    // 処理されるときは必ずOwnerは存在しますので基本的にnullptrチェックは必要ありません
    if(auto model = GetOwner()->GetComponent<ComponentModel>()) {
        // オーナーの投げるアニメーションが0.42以上であれば
        if(0.65 <= model->GetAnimationTime()) {
            // シェアードポインタに変換
            // バレットがあれば(nullptrかチェック)
            if(auto throw_object = throw_weak_ptr_.lock()) {
                // アタッチモデルを取り外す
                throw_object->RemoveComponent<ComponentAttachModel>();

                auto grabbable = throw_object->GetComponent<ComponentGrabbable>();
                grabbable->SetIsGrabbed(false);

                const float3 up_len = {0.0f, 5.0f, 0.0f};
                float3       vec    = {0.0f, 1.0f, 0.0f};

                // GetOnwer：オーナー(自分がAddComponentされたObject)を取得します
                // 処理されるときは必ずOwnerは存在しますので基本的にnullptrチェックは必要ありません
                auto   owner = GetOwner();
                matrix mat   = matrix::identity();
                if(auto model = owner->GetComponent<ComponentModel>()) {
                    // モデルの完全な向きは、GetWorldMatrix()でとる必要があります。
                    mat = model->GetWorldMatrix();

                    constexpr float front_len = 5.0f;

                    vec = -(model->GetWorldVectorAxisZ() * front_len);
                }
                if(auto throw_object = throw_weak_ptr_.lock()) {
                    auto grabbable = throw_object->GetComponent<ComponentGrabbable>();
                    throw_object->AddTranslate(vec * 5.0f);
                    grabbable->SetTranslation(vec * 5.0f);
                }
                // 投げるモーションが終わったので当たり判定を
                if(auto collider = throw_object->GetComponent<ComponentCollision>()) {
                    collider->SetCollisionStatus(ComponentCollision::CollisionBit::DisableHit, false);
                }

                // 投げ主を記録（自分自身への初期ダメージをスキップするため）
                if(auto grabbable = throw_object->GetComponent<ComponentGrabbable>()) {
                    grabbable->SetThrower(GetOwnerPtr());
                }

                throw_weak_ptr_.reset();
                //ChangeState<ComponentStateIdleWalk>();
            }

            /*
            * // ここに実装すると投擲コンポーネントがプレイヤーに依存するのでだめだが、
            * こういった実装をするとコンボも可能
            if(Input::IsKeyDown(KEY_INPUT_SPACE))
            {
                RemoveThisComponent();
                model->AddComponent<ComponentStateThrow2>;
            }
            */
        }

        // アニメーションが止まっているか
        if(!model->IsPlaying()) {
            // 自分自身を取り除く
            RemoveThisComponent();
            // 歩きのコンポーネントを再付与する
            GetOwner()->AddComponent<ComponentStateIdleWalk>()->SetMoveSpeed(0.3f)->SetRotateSpeed(20.0f);
        }
    }
}

void ComponentStateThrow::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("State Throw")) {
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

void ComponentStateThrow::SetThrowObject(ObjectWeakPtr object_ptr)
{
    throw_weak_ptr_ = object_ptr;
}

CEREAL_REGISTER_TYPE(ComponentStateThrow)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateThrow)
