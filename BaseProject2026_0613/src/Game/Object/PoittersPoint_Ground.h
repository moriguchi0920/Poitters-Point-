#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Ground.h
//! @brief  PoittersPoint_Ground (ステージ：地面メッシュ + 外周の見えない壁)
//---------------------------------------------------------------------------
#include <System/Scene.h>
#include "Game/System/PoittersPoint_PhysicsHelper.h"

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

    //! @brief GUI(壁の位置調整)
    void GUI() override;

    //! @brief ステージの範囲(ワールド座標のAABB)
    const PhysicsHelper::Bounds& GetStageBounds() const { return bounds_; }

    //! @brief 指定XZ位置の地面の高さを調べる(物理のレイキャスト)
    //! @retval false 地面が無い
    bool FindGroundHeight(float x, float z, float& out_y) const;

    //! @brief ステージ外へ落ちた物を戻す位置(ステージ中央の地面の上)
    float3 GetRespawnPosition() const;

    //! @brief 落下したとみなす高さ
    float GetFallOutHeight() const { return (float)bounds_.min_.y - 100.0f; }

private:
    //! @brief 外周の壁を(作り直して)配置する
    void BuildWalls();

    PhysicsHelper::Bounds      bounds_;    //!< ステージモデルの範囲(ワールド)
    std::vector<ObjectWeakPtr> walls_;     //!< 外周の壁

    float wall_inset_     = 0.0f;      //!< 壁をモデルの端から内側へ寄せる量
    float wall_thickness_ = 20.0f;     //!< 壁の厚さ
    float wall_height_    = 400.0f;    //!< 壁の高さ(モデルの一番低い所から)
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Ground);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Ground);
