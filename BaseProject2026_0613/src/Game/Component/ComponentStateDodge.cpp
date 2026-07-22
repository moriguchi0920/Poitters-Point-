#pragma once
#include <Game/Component/ComponentStateDodge.h>
#include "ComponentStateIdleWalk.h"
#include "Game/Scene/PoittersPoint_Stage.h"

void ComponentStateDodge::Init()
{
    __super::Init();
    SetName<Component>("State Dodge");

    // GetOnwer：オーナー(自分がAddComponentされたObject)を取得します
    // 処理されるときは必ずOwnerは存在しますので基本的にnullptrチェックは必要ありません
    auto owner = GetOwner();
}

void ComponentStateDodge::Update()
{
    __super::Update();
}

void ComponentStateDodge::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("State Dodge")) {
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

CEREAL_REGISTER_TYPE(ComponentStateDodge)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateDodge)
