#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Player.h
//! @brief  PoittersPoint_Player
//---------------------------------------------------------------------------
#include <System/Scene.h>
#include <System/Object.h>
#include <System/Component/ComponentCollision.h>

class ComponentGrabbable;

namespace PoittersPoint {

class Player;
USING_PTR(Player);

class Player : public Object
{
public:
    BP_OBJECT_DECL(Player, u8"プレイヤー");

    //! @brief 初期化
    bool Init() override;

    //! @brief 更新
    void Update() override;

    //! @brief 当たり判定のコールバック
    void OnHit(const ComponentCollision::HitInfo& hit_info) override;

    //! @brief 視界判定
    void OnEyeSight();

private:
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Player);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Player);
