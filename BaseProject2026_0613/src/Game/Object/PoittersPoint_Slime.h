#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Slime.h
//! @brief  PoittersPoint_Slime
//---------------------------------------------------------------------------
#include <System/Scene.h>
#include <System/Object.h>
#include <System/Component/ComponentCollision.h>

namespace PoittersPoint {

class Slime;
USING_PTR(Slime);

class Slime : public Object
{
public:
    BP_OBJECT_DECL(Slime, u8"スライム");

    //! @brief 初期化
    bool Init() override;

    //! @brief 更新
    void Update() override;

    //! @brief GUI
    void GUI() override;

    //! @brief 当たり判定のコールバック
    void OnHit(const ComponentCollision::HitInfo& hit_info) override;

private:
    //! @brief 爆発処理
    void Explode();
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Slime);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Slime);
