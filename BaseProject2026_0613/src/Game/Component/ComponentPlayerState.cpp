#pragma once
#include <Game/Component/ComponentPlayerState.h>
#include "ComponentStateIdleWalk.h"
#include "ComponentStateThrow.h"

void ComponentPlayerState::Init()
{
    __super::Init();

    // オブジェクトの制御を行うコンポーネントを追加
    GetOwner()->AddComponent<ComponentStateIdleWalk>()->SetMoveSpeed(0.3f)->SetRotateSpeed(20.0f);
}

void ComponentPlayerState::Update()
{
    __super::Update();

    auto owner = GetOwner();

    if(Input::IsKeyDown(KEY_INPUT_SPACE)) {
        // StateThrowがない、つまり投擲していないなら
        // (投げいるかどうかのフラグを作ってあげれば処理を高速化できる)
        if(!owner->GetComponent<ComponentStateThrow>()) {
            // 歩きのモーションを止めて投げるためにIdleWalkを外す
            owner->RemoveComponent<ComponentStateIdleWalk>();
            // 投擲コンポーネントを付与
            owner->AddComponent<ComponentStateThrow>();
        }
    }
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
