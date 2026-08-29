//---------------------------------------------------------------------------
//! @file   ComponentStateDead.cpp
//! @brief  死亡状態コンポーネント
//---------------------------------------------------------------------------
#include <Game/Component/ComponentStateDead.h>
#include "Game/Scene/PoittersPoint_Stage.h"

//! @brief 初期化
//! 死亡アニメーションを再生する
void ComponentStateDead::Init()
{
    __super::Init();

    SetName<Component>("State Dead");

    auto owner = GetOwner();

    if(auto model = owner->GetComponent<ComponentModel>()) {
        model->PlayAnimationNoSame("dead", false, 0.1f);
    }
}

//! @brief 更新
//! アニメーション終了後に死亡処理を実行し、自身を削除する
void ComponentStateDead::Update()
{
    __super::Update();

    auto owner = GetOwner();

    if(auto model = owner->GetComponent<ComponentModel>()) {
        if(!model->IsPlaying()) {
            // 死亡処理：アニメ終了後に実行
            // 将来的にはCharacterクラスで共通化予定
            owner->SetName("dead");

            if(auto col = owner->GetComponent<ComponentCollisionCapsule>()) {
                owner->RemoveComponent<ComponentCollisionCapsule>();
            }

            RemoveThisComponent();
        }
    }
}

//! @brief GUI
void ComponentStateDead::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("State Dead")) {
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

CEREAL_REGISTER_TYPE(ComponentStateDead)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateDead)
