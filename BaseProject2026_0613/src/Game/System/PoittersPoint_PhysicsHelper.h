#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_PhysicsHelper.h
//! @brief  ComponentPhysics を使う際のゲーム共通ヘルパー
//! @details 物理の判定は全て ComponentPhysics(Jolt) に一本化している。
//!          形状をモデルの大きさから自動で合わせる処理や、物理ボディごとの瞬間移動など
//!          各オブジェクトで重複しがちな処理をここにまとめる。
//---------------------------------------------------------------------------
#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentPhysics.h>

namespace PoittersPoint::PhysicsHelper {

//! @brief オーナー空間での軸平行バウンディングボックス
struct Bounds
{
    float3 min_ = {0.0f, 0.0f, 0.0f};
    float3 max_ = {0.0f, 0.0f, 0.0f};

    float3 Center() const { return (min_ + max_) * 0.5f; }
    float3 HalfExtent() const { return (max_ - min_) * 0.5f; }
};

//! @brief モデルの頂点からオーナー空間のAABBを求める(ComponentModel のローカル行列・スケール込み)
//! @retval true 取得成功
bool GetModelBounds(const ComponentModelPtr& model, Bounds& out);

//! @brief モデルの大きさに合わせた球を作成する(中心もモデルに合わせる)
//! @return 作成した半径(失敗時は fallback_radius で作成)
float CreateSphereFitModel(const ComponentPhysicsPtr& physics, const ComponentModelPtr& model, float fallback_radius, float density);

//! @brief モデルの一番長い軸に沿った円柱を作成する(丸太など、転がる物用)
//! @details 円柱の軸はボディのローカルY軸。physics_transform でモデルの長軸へ回転させる
//! @param[out] out_radius 作成した半径
//! @param[out] out_half_height 作成した半分の長さ
//! @retval true モデルから作成できた
bool CreateCylinderFitModel(const ComponentPhysicsPtr& physics, const ComponentModelPtr& model, float density, float& out_radius, float& out_half_height);

//! @brief オブジェクトを物理ボディごと指定位置へ移動する(速度もリセット)
//! @details ComponentPhysics を持つオブジェクトに SetTranslate だけを行うと、
//!          次の PostPhysics で物理ボディの位置に戻されてしまうため、こちらを使う
void Warp(Object* obj, const float3& position);

//! @brief ボディを動かない静的ボディにする(ステージの壁など)
void MakeStatic(const ComponentPhysicsPtr& physics);

}    // namespace PoittersPoint::PhysicsHelper
