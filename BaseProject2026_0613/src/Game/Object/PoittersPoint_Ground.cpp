//---------------------------------------------------------------------------
//! @file   PoittersPoint_Ground.cpp
//! @brief  PoittersPoint_Ground (ステージ：地面メッシュ + 外周の見えない壁)
//---------------------------------------------------------------------------
#include "PoittersPoint_Ground.h"

#include <System/Scene.h>
#include <System/Geometry.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentPhysics.h>
#include <System/Physics/PhysicsEngine.h>

namespace PoittersPoint {
// namespace PoittersPoint

//! @brief 初期化
//! @return 初期化終了
bool Ground::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();

    SetName("Ground");

    // ステージ02
    // (他のステージ: "data/Sample/SwordBout/Stage/Stage00.mv1", "data/Sample/Stage01/FantasyTown.mv1" スケール4.0)
    auto model = AddComponent<ComponentModel>("data/Sample/Stage02/02_Ruin_Main.mv1");
    model->SetScaleAxisXYZ({1.9f, 1.9f, 1.9f});

    // 地面は Jolt 物理の静的メッシュのみ(キャラクターも投げ物も同じ地面に乗る)
    // CreateMesh は常に静的ボディになる。モデルのスケールは CreateMesh 内部で GetScaleAxisXYZ() から織り込む
    auto physics = AddComponent<ComponentPhysics>();
    physics->CreateMesh(model);
    physics->SetCollisionType(ComponentPhysics::CollisionType::GROUND);

    // ステージの範囲を求めて外周に壁を置く
    if(PhysicsHelper::GetModelBounds(model, bounds_)) {
        bounds_.min_ += GetTranslate();
        bounds_.max_ += GetTranslate();
    }
    else {
        bounds_.min_ = {-200.0f, -20.0f, -200.0f};
        bounds_.max_ = {200.0f, 100.0f, 200.0f};
    }
    BuildWalls();

    return true;
}

void Ground::BuildWalls()
{
    // 作り直す場合は古い壁を消す
    for(auto& weak : walls_) {
        if(auto wall = weak.lock())
            Scene::Object::Release(wall);
    }
    walls_.clear();

    const float3 mn     = bounds_.min_;
    const float3 mx     = bounds_.max_;
    const float3 center = bounds_.Center();
    const float  half_t = wall_thickness_ * 0.5f;
    const float  half_h = wall_height_ * 0.5f;
    const float  y      = (float)mn.y + half_h;
    const float  inset  = wall_inset_;

    // 内側の面がモデルの端(から inset 内側)に来るように置く
    const float half_x = (float)(mx.x - mn.x) * 0.5f - inset;
    const float half_z = (float)(mx.z - mn.z) * 0.5f - inset;

    struct WallDesc
    {
        float3 position;
        float3 half_extent;
    };
    const WallDesc descs[] = {
        {{(float)center.x + half_x + half_t, y, (float)center.z}, {half_t, half_h, half_z + wall_thickness_}},    // +X
        {{(float)center.x - half_x - half_t, y, (float)center.z}, {half_t, half_h, half_z + wall_thickness_}},    // -X
        {{(float)center.x, y, (float)center.z + half_z + half_t}, {half_x + wall_thickness_, half_h, half_t}},    // +Z
        {{(float)center.x, y, (float)center.z - half_z - half_t}, {half_x + wall_thickness_, half_h, half_t}},    // -Z
    };

    int index = 0;
    for(auto& desc : descs) {
        auto wall = Scene::Object::Create<Object>("StageWall_" + std::to_string(index++));
        wall->SetTranslate(desc.position);

        auto physics = wall->AddComponent<ComponentPhysics>();
        physics->CreateBox(desc.half_extent);
        physics->SetCollisionType(ComponentPhysics::CollisionType::WALL);
        physics->SetFriction(0.0f);    // 壁に沿って滑れるように
        PhysicsHelper::MakeStatic(physics);

        walls_.push_back(wall);
    }
}

bool Ground::FindGroundHeight(float x, float z, float& out_y) const
{
    const float top    = (float)bounds_.max_.y + 50.0f;
    const float bottom = (float)bounds_.min_.y - 50.0f;

    // 上から下へレイを飛ばし、最初に当たった高さを地面とする
    Ray                     ray({x, top, z}, {0.0f, bottom - top, 0.0f});
    physics::RayCastResult  result{};
    if(!physics::Engine::instance()->castRay(ray, result))
        return false;

    out_y = (float)ray.position(result.t_).y;
    return true;
}

float3 Ground::GetRespawnPosition() const
{
    const float3 center = bounds_.Center();
    float        y      = 0.0f;
    if(!FindGroundHeight(center.x, center.z, y))
        y = (float)bounds_.max_.y;
    return {(float)center.x, y + 20.0f, (float)center.z};
}

void Ground::GUI()
{
    __super::GUI();

    ImGui::Begin(GetName().data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode("Stage Wall")) {
            ImGui::Text(u8"ステージ範囲 min(%.0f, %.0f, %.0f) max(%.0f, %.0f, %.0f)",
                        (float)bounds_.min_.x,
                        (float)bounds_.min_.y,
                        (float)bounds_.min_.z,
                        (float)bounds_.max_.x,
                        (float)bounds_.max_.y,
                        (float)bounds_.max_.z);
            bool change  = ImGui::DragFloat(u8"壁を内側へ寄せる量", &wall_inset_, 1.0f, -500.0f, 2000.0f);
            change      |= ImGui::DragFloat(u8"壁の高さ", &wall_height_, 1.0f, 10.0f, 5000.0f);
            if(change)
                BuildWalls();
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

}    // namespace PoittersPoint
