#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Player.h
//! @brief  PoittersPoint_Player
//---------------------------------------------------------------------------
#include <System/Scene.h>

class ComponentGrabbable;
namespace PoittersPoint {
// namespace PoittersPoint

USING_PTR(Player);
class Player : public Object
{
public:
    BP_OBJECT_DECL(Player, "PoittersPoint::Player");

    //! @brief 初期化
    //! @return 初期化終了
    bool Init() override;

    //! @brief 更新
    //! 視界内の掴み可能対象を検知する
    void Update() override;

    //! @brief 当たり判定のコールバック
    //! 投げられた物体が移動中に当たった場合、ダメージを与える
    void OnHit(const ComponentCollision::HitInfo& hit_info) override;

    //! @brief 視界判定
    //! 正面45度・距離50以内の掴み可能対象を検知する
    void OnEyeSight();

private:
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Player);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Player);
