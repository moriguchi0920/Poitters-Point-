#include <Game/Component/ComponentMagicPoint.h>

float ComponentMagicPoint::max_magic_points_ = 5.0f;

void ComponentMagicPoint::Init()
{
    __super::Init();

    magic_points_ = max_magic_points_;
}

float ComponentMagicPoint::GetMagicPoints()
{
    return magic_points_;
}

void ComponentMagicPoint::UseMP(float cost)
{
    // MPが0以下なら
    if(GetMagicPoints() <= 0) {
        // これ以上消費する必要は無いのでここで弾く
        return;
    }

    // MPを消費量分減算する
    magic_points_ -= cost;

    // MPがもし0を下回っていたら
    if(magic_points_ < 0) {
        // 0に補正してあげる
        magic_points_ = 0;
    }
}

void ComponentMagicPoint::SetMaxMP(float new_max_mp)
{
    // 最大MPを設定
    max_magic_points_ = new_max_mp;

    // もし最大MPが減らされた場合、現在のMPが上限を突破しないように調整する
    if(max_magic_points_ < magic_points_) {
        magic_points_ = max_magic_points_;
    }
}

void ComponentMagicPoint::SetMaxAndCurrentMP(float new_mp)
{
    // 最大MPと現在のMPを設定
    max_magic_points_ = new_mp;
    magic_points_     = new_mp;
}

void ComponentMagicPoint::SetMaxAndCurrentMP(float new_max_mp, float new_mp)
{
    // 最大MPと現在のMPをそれぞれ設定
    max_magic_points_ = new_max_mp;
    magic_points_     = new_mp;
}

void ComponentMagicPoint::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("Magic Points")) {
            //-------------------------------------------------------
            // 共通部分(共通化したい)

            // 有効/無効
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable))
                SetStatus(StatusBit::Enable, enable);

            // GUI上でオーナーから自分を削除します
            if(ImGui::Button(u8"削除"))
                GetOwner()->RemoveComponent(shared_from_this());
            //-------------------------------------------------------

            // 基本情報(最後の引数で一回ドラッグした時の変化量を調節可能)
            ImGui::DragFloat(u8"最大MP", &max_magic_points_, 0.1f);
            ImGui::DragFloat(u8"現在のMP", &magic_points_, 0.1f);

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentMagicPoint)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentMagicPoint)
