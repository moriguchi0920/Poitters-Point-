#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Ground.h
//! @brief  PoittersPoint_Ground
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace PoittersPoint {
// namespace PoittersPoint

USING_PTR(Ground);
class Ground : public Object
{
public:
    BP_OBJECT_DECL(Ground, "PoittersPoint::Ground");

    //! @brief 初期化
    //! @return 初期化終了
    bool Init() override;
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Ground);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Ground);
