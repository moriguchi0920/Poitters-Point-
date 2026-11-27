#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Slime.h
//! @brief  PoittersPoint_Slime (投げ物：スライム)
//---------------------------------------------------------------------------
#include <System/Scene.h>
#include <System/Object.h>

namespace PoittersPoint {

class Slime;
USING_PTR(Slime);

class Slime : public Object
{
public:
    BP_OBJECT_DECL(Slime, u8"スライム");

    //! @brief 初期化
    bool Init() override;

    //! @brief 更新(爆発要求の処理)
    void Update() override;

    //! @brief GUI
    void GUI() override;

private:
    //! @brief 爆発処理
    void Explode();

    bool explode_requested_ = false;    //!< 投げられて何かに当たった(次の Update で爆発する)

    float radius_           = 5.0f;     //!< 物理形状の半径(モデルの大きさから自動計算。取得できない時の既定値)
    float density_          = 300.0f;   //!< 密度(軽い)
    float explosion_radius_ = 45.0f;    //!< 爆発範囲
    float explosion_damage_ = 2.0f;     //!< 爆発ダメージ
    float explosion_power_  = 180.0f;   //!< 爆風で吹き飛ばす強さ
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Slime);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Slime);
