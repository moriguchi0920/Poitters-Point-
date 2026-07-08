#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Camera.h
//! @brief  PoittersPoint_Camera
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace PoittersPoint {
// namespace PoittersPoint

USING_PTR(Camera);
class Camera : public Object
{
public:
    BP_OBJECT_DECL(Camera, "PoittersPoint::Camera");

    //! @brief 初期化
    //! @return 初期化終了
    bool Init() override;
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Camera);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Camera);
