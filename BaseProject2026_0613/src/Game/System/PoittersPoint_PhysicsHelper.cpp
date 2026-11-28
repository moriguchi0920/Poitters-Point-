//---------------------------------------------------------------------------
//! @file   PoittersPoint_PhysicsHelper.cpp
//! @brief  ComponentPhysics を使う際のゲーム共通ヘルパー
//---------------------------------------------------------------------------
#include "PoittersPoint_PhysicsHelper.h"

#include <System/Physics/PhysicsLayer.h>
#include <System/Physics/RigidBody.h>

namespace PoittersPoint::PhysicsHelper {

bool GetModelBounds(const ComponentModelPtr& model, Bounds& out)
{
    if(!model)
        return false;

    int handle = model->GetModel();
    if(handle < 0)
        return false;

    // フレーム階層の変換は含めたいが、オブジェクトの位置は含めたくないため
    // 一時的にモデル行列を単位行列にしてから参照メッシュを作る
    MATRIX old_matrix = MV1GetMatrix(handle);
    MV1SetMatrix(handle, MGetIdent());
    MV1SetupReferenceMesh(handle, -1, TRUE);
    MV1RefreshReferenceMesh(handle, -1, TRUE);
    MV1_REF_POLYGONLIST ref = MV1GetReferenceMesh(handle, -1, TRUE);
    MV1SetMatrix(handle, old_matrix);

    if(ref.PolygonNum <= 0)
        return false;

    // モデルコンポーネントのローカル行列(スケール・オフセット)を掛けてオーナー空間にする
    const matrix& local = model->GetMatrix();
    const float3  mn    = cast(ref.MinPosition);
    const float3  mx    = cast(ref.MaxPosition);

    bool first = true;
    for(int i = 0; i < 8; ++i) {
        float3 corner = {(i & 1) ? mx.x : mn.x, (i & 2) ? mx.y : mn.y, (i & 4) ? mx.z : mn.z};
        float3 p      = mul(float4(corner, 1.0f), local).xyz;
        if(first) {
            out.min_ = p;
            out.max_ = p;
            first    = false;
            continue;
        }
        out.min_ = min(out.min_, p);
        out.max_ = max(out.max_, p);
    }
    return true;
}

float CreateSphereFitModel(const ComponentPhysicsPtr& physics, const ComponentModelPtr& model, float fallback_radius, float density)
{
    float radius = fallback_radius;

    Bounds bounds;
    if(GetModelBounds(model, bounds)) {
        float3 half = bounds.HalfExtent();
        radius      = ((float)half.x + (float)half.y + (float)half.z) / 3.0f;
        // ボディの中心をモデルの中心に合わせる(Create前に設定しないとボディ位置に反映されない)
        physics->SetTranslate(bounds.Center());
    }

    physics->CreateSphere({0.0f, 0.0f, 0.0f}, radius, density);
    return radius;
}

bool CreateCylinderFitModel(const ComponentPhysicsPtr& physics, const ComponentModelPtr& model, float density, float& out_radius, float& out_half_height)
{
    Bounds bounds;
    if(!GetModelBounds(model, bounds))
        return false;

    float3 half    = bounds.HalfExtent();
    float  e[3]    = {(float)half.x, (float)half.y, (float)half.z};
    int    long_ax = 0;
    for(int i = 1; i < 3; ++i) {
        if(e[i] > e[long_ax])
            long_ax = i;
    }
    int a = (long_ax + 1) % 3;
    int b = (long_ax + 2) % 3;

    out_half_height = e[long_ax];
    out_radius      = (e[a] + e[b]) * 0.5f;

    // ボディのローカルY軸(円柱の軸)をモデルの長軸へ向ける回転
    // 行ベクトル規約のため、各行が「ボディの各軸がオーナー空間でどちらを向くか」になる
    float3 axis[3]   = {float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1)};
    float3 row_y     = axis[long_ax];
    float3 row_x     = axis[a];
    float3 row_z     = cross(row_x, row_y);
    matrix offset    = matrix::identity();
    offset._11_12_13 = row_x;
    offset._21_22_23 = row_y;
    offset._31_32_33 = row_z;
    offset._41_42_43 = bounds.Center();

    physics->Matrix() = offset;
    physics->CreateCylinder(out_half_height, out_radius, density);
    return true;
}

void Warp(Object* obj, const float3& position)
{
    if(!obj)
        return;

    obj->SetTranslate(position);

    if(auto physics = obj->GetComponent<ComponentPhysics>()) {
        physics->SetPhysicsMatrix(physics->GetWorldMatrix());
        if(auto body = physics->GetRigidBody()) {
            body->setLinearVelocity({0.0f, 0.0f, 0.0f});
            body->setAngularVelocity({0.0f, 0.0f, 0.0f});
        }
    }
}

void MakeStatic(const ComponentPhysicsPtr& physics)
{
    if(!physics)
        return;

    physics->SetPhysicsStatus(ComponentPhysics::PhysicsBit::Static, true);
    physics->SetGravityFactor(0.0f);
    if(auto body = physics->GetRigidBody()) {
        body->setGravityFactor(0.0f);
        body->setMotionType(physics::MotionType::Static, false);
        body->setLayer(physics::ObjectLayers::NON_MOVING);
    }
}

}    // namespace PoittersPoint::PhysicsHelper
