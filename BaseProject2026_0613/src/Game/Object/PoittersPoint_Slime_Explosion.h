#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Slime_Explosion.h
//! @brief  PoittersPoint_Slime_Explosion
//---------------------------------------------------------------------------
#include <System/Scene.h>
#include <System/Object.h>

namespace PoittersPoint {

class SlimeExplosion;
USING_PTR(SlimeExplosion);

class SlimeExplosion : public Object
{
public:
    BP_OBJECT_DECL(SlimeExplosion, u8"スライム爆発エフェクト");

    //! @brief 初期化
    bool Init() override;

    //! @brief 更新
    void Update() override;

private:
    float lifetime_ = 0.5f;    // エフェクト表示時間（秒）
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::SlimeExplosion);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::SlimeExplosion);
