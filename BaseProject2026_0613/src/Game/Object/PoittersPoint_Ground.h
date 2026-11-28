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

    //! @brief 描画(フィールドの境界線の表示)
    void Draw() override;

    //! @brief GUI(フィールドの形・壁の調整)
    void GUI() override;

    //! @brief ステージモデル全体の範囲(ワールド座標のAABB)
    //! @details 遊べる範囲はこれより狭い。遊べる範囲は IsInsideField / GetFieldVertices を使う
    const PhysicsHelper::Bounds& GetStageBounds() const { return bounds_; }

    //------------------------------------------------------------------
    //! @name フィールド(プレイヤーもアイテムも動ける範囲)
    //! 左右の下り階段・奥の上り階段・手前の瓦礫の内側をつないだ多角形。外周に物理の壁を立てる
    //------------------------------------------------------------------
    //@{
    //! @brief フィールドの内側か
    //! @param margin 境界からこれ以上内側にあるか(0 なら境界上も内側)
    bool IsInsideField(float x, float z, float margin = 0.0f) const;

    //! @brief フィールドの中心(XZ。Y は 0)
    float3 GetFieldCenter() const { return field_center_; }

    //! @brief フィールドの頂点(ワールド座標・反時計回り。Y は地面の高さ)
    const std::vector<float3>& GetFieldVertices() const { return field_vertices_; }

    //! @brief フィールドを囲む四角形(XZ)
    void GetFieldRect(float& min_x, float& max_x, float& min_z, float& max_z) const;
    //@}

    //! @brief 指定XZ位置の地面の高さを調べる(物理のレイキャスト)
    //! @retval false 地面が無い
    bool FindGroundHeight(float x, float z, float& out_y) const;

    //! @brief ステージ外へ落ちた物を戻す位置(ステージ中央の地面の上)
    float3 GetRespawnPosition() const;

    //! @brief 落下したとみなす高さ
    float GetFallOutHeight() const { return (float)bounds_.min_.y - 100.0f; }

private:
    //! @brief フィールドの多角形の頂点を作る
    void BuildField();

    //! @brief フィールドの各辺に壁を(作り直して)配置する
    void BuildWalls();

    PhysicsHelper::Bounds      bounds_;    //!< ステージモデルの範囲(ワールド)
    std::vector<ObjectWeakPtr> walls_;     //!< 外周の壁

    //---------------------------------------------------------------
    // フィールドの形
    //  中心から各方向の境界までの距離を4方向で決め、その間を楕円でつないだ多角形にする。
    //  値はステージ02を中心から物理のレイキャストで調べ、地形(階段・瓦礫)が始まる少し手前に合わせている
    //   右(+X)/左(-X): 下り階段が約172から  奥(+Z): 上り階段が約118から  手前(-Z): 瓦礫が約192から
    //---------------------------------------------------------------
    float3 field_center_   = {0.0f, 0.0f, 0.0f};    //!< フィールドの中心
    float  field_right_    = 165.0f;                //!< 中心から右(+X)の境界までの距離
    float  field_left_     = 165.0f;                //!< 中心から左(-X)の境界までの距離
    float  field_back_     = 112.0f;                //!< 中心から奥(+Z)の境界までの距離
    float  field_front_    = 185.0f;                //!< 中心から手前(-Z)の境界までの距離
    int    field_segments_ = 32;                    //!< 多角形の辺の数

    std::vector<float3> field_vertices_;    //!< フィールドの頂点

    float wall_thickness_ = 10.0f;     //!< 壁の厚さ
    float wall_bottom_    = -50.0f;    //!< 壁の下端の高さ
    float wall_height_    = 400.0f;    //!< 壁の高さ(下端から)

    bool show_field_line_ = true;    //!< フィールドの境界線を表示するか(調整用)
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Ground);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Ground);
