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

    void Update() override;

    void OnHit(const ComponentCollision::HitInfo& hit_info) override;

    void OnEyeSight();

private:
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Player);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Player);
