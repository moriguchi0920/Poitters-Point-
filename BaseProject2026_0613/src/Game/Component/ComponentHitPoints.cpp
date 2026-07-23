#pragma once
#include <Game/Component/ComponentHitPoints.h>

float ComponentHitPoints::max_hit_points_ = 5.0f;

void ComponentHitPoints::Init()
{
    __super::Init();

    // オブジェクトの制御を行うコンポーネントを追加
    //GetOwner()->AddComponent<ComponentStateIdleWalk>()->SetMoveSpeed(0.3f)->SetRotateSpeed(20.0f);

    hit_points_ = max_hit_points_;
}

float ComponentHitPoints::GetHitPoints()
{
    return hit_points_;
}

void ComponentHitPoints::TakeDamage(float damage)
{
    // HPが0以下なら
    if(GetHitPoints() < 0) {
        // これ以上ダメージを与える必要は無いのでここで弾く
        return;
    }

    // HPをダメージ分減算する
    hit_points_ -= damage;

    // HPがもし0を下回っていたら
    if(hit_points_ < 0) {
        // 0に補正してあげる
        hit_points_ = 0;
    }
}

void ComponentHitPoints::SetMaxHP(float new_max_hp)
{
    // 最大HPを設定
    max_hit_points_ = new_max_hp;

    // もし最大HPが減らされた場合、現在のHPが上限を突破しないように調整する
    if(max_hit_points_ < hit_points_) {
        hit_points_ = max_hit_points_;
    }
}

void ComponentHitPoints::SetMaxAndCurrentHP(float new_hp)
{
    // 最大HPと現在のHPを設定
    max_hit_points_ = new_hp;
    hit_points_     = new_hp;
}

void ComponentHitPoints::SetMaxAndCurrentHP(float new_max_hp, float new_hp)
{
    // 最大HPと現在のHPをそれぞれ設定
    max_hit_points_ = new_max_hp;
    hit_points_     = new_hp;
}

void ComponentHitPoints::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("Hit Points")) {
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

            // 基本情報(最後の引数で一回ドラッグした時の変化量を調節可能)
            ImGui::DragFloat(u8"最大HP", &max_hit_points_, 0.1f);
            ImGui::DragFloat(u8"現在のHP", &hit_points_, 0.1f);

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentHitPoints)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentHitPoints)
