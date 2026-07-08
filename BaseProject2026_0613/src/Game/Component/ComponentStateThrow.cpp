#pragma once
#include <Game/Component/ComponentStateThrow.h>
#include "ComponentStateIdleWalk.h"
#include "Game/Scene/PoittersPoint_Stage.h"

void ComponentStateThrow::Init()
{
    __super::Init();

    // GetOnwer：オーナー(自分がAddComponentされたObject)を取得します
    // 処理されるときは必ずOwnerは存在しますので基本的にnullptrチェックは必要ありません
    auto owner = GetOwner();

    const float3 up_len = {0.0f, 5.0f, 0.0f};
    float3       vec    = {0.0f, 1.0f, 0.0f};

    matrix mat = matrix::identity();
    if(auto model = owner->GetComponent<ComponentModel>()) {
        // モデルの完全な向きは、GetWorldMatrix()でとる必要があります。
        mat = model->GetWorldMatrix();

        constexpr float front_len = 5.0f;

        vec = -(model->GetWorldVectorAxisZ() * front_len);

        model->PlayAnimationNoSame("throw");
    }

    auto bullet = Scene::Object::Create<PoittersPoint::Bullet>();
    bullet->RemoveComponent<ComponentCollisionSphere>();
    bullet->SetDirection(vec);
    bullet->SetTranslate(owner->GetTranslate() + up_len + vec);

    auto attach = bullet->AddComponent<ComponentAttachModel>();

    // rig(リグ)：
    // 作成したモデル（キャラクターや乗り物など）を動かすための
    // 骨組み（ボーン）と、それを直感的に操作するためのコントローラーがセットになった
    // 制御システム全体のこと
    attach->SetAttachObject("Player", "mixamorig:RightHand");
    attach->SetAttachOffset({12, 17, 0});

    bullet_ = bullet;
}

void ComponentStateThrow::Update()
{
    __super::Update();

    // オーナー(自分がAddComponentされたObject)を取得します
    // 処理されるときは必ずOwnerは存在しますので基本的にnullptrチェックは必要ありません
    if(auto model = GetOwner()->GetComponent<ComponentModel>()) {
        // オーナーの投げるアニメーションが0.42以上であれば
        if(0.42 <= model->GetAnimationTime()) {
            // シェアードポインタに変換
            // バレットがあれば(nullptrかチェック)
            if(auto bullet = bullet_.lock()) {
                // アタッチモデルを取り外す
                bullet->RemoveComponent<ComponentAttachModel>();
                // 投げるモーションが終わったので当たり判定を
                bullet->AddComponent<ComponentCollisionSphere>()->SetRadius(3.0f);

                bullet->ResetDirection();

                // 引数が無しでresetが呼ばれているのでユニークポインタがリソースを保持していない状態となる
                bullet_.reset();
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

CEREAL_REGISTER_TYPE(ComponentStateThrow)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateThrow)
