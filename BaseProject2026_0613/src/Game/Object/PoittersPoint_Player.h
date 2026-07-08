#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Player.h
//! @brief  PoittersPoint_Player
//---------------------------------------------------------------------------
#include <System/Scene.h>

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
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Player);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Player);
