//---------------------------------------------------------------------------
//! @file   ComponentStateDead.cpp
//! @brief  死亡状態コンポーネント
//---------------------------------------------------------------------------
#include <Game/Component/ComponentStateDead.h>
#include "Game/Scene/PoittersPoint_Stage.h"
#include <Game/Component/ComponentCharacterPhysics.h>

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
            // (以前は SetName("dead") で死亡を表していたが、名前は一意でなく状態にも使えないためやめた。
            //  死亡しているかは ComponentHitPoints::IsDead() で判定する)

            // 死体が他の物の邪魔をしないよう、物理ボディごと当たり判定を消す
            if(owner->GetComponent<ComponentCharacterPhysics>())
                owner->RemoveComponent<ComponentCharacterPhysics>();
            if(owner->GetComponent<ComponentPhysics>())
                owner->RemoveComponent<ComponentPhysics>();

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
