#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Rock.h
//! @brief  PoittersPoint_Rock
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace PoittersPoint {
// namespace PoittersPoint

USING_PTR(Rock);
class Rock : public Object
{
public:
    BP_OBJECT_DECL(Rock, "PoittersPoint::Rock");

    //! @brief 初期化
    //! @return 初期化終了
    bool Init() override;

    //! @brief 更新
    void Update() override;

    //! @brief GUI
    void GUI() override;

private:
    float radius_  = 5.0f;     //!< 物理形状の半径(モデルの大きさから自動計算)
    float density_ = 1000.0f;  //!< 密度(重さ)
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Rock);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Rock);
