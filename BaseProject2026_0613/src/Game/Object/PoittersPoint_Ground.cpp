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

    // 遊べる範囲(フィールド)を作り、その外周に壁を置く
    BuildField();
    BuildWalls();

    return true;
}

void Ground::BuildField()
{
    field_vertices_.clear();

    const int segments = std::max(field_segments_, 8);
    for(int i = 0; i < segments; ++i) {
        // 反時計回り(+X → +Z → -X → -Z)
        float angle = TAU * static_cast<float>(i) / static_cast<float>(segments);
        float c     = cosf(angle);
        float s     = sinf(angle);

        // 象限ごとに X方向・Z方向の距離を選び、楕円の式で角度ごとの距離を求める
        float rx = (c >= 0.0f) ? field_right_ : field_left_;
        float rz = (s >= 0.0f) ? field_back_ : field_front_;
        float r  = 1.0f / sqrtf((c * c) / (rx * rx) + (s * s) / (rz * rz));

        field_vertices_.push_back({(float)field_center_.x + c * r, (float)field_center_.y, (float)field_center_.z + s * r});
    }
}

bool Ground::IsInsideField(float x, float z, float margin) const
{
    const size_t count = field_vertices_.size();
    if(count < 3)
        return true;

    // 反時計回りの凸多角形なので、全ての辺の「内側」にあれば中にある
    for(size_t i = 0; i < count; ++i) {
        const float3& a = field_vertices_[i];
        const float3& b = field_vertices_[(i + 1) % count];

        float ex  = (float)(b.x - a.x);
        float ez  = (float)(b.z - a.z);
        float len = sqrtf(ex * ex + ez * ez);
        if(len < 0.0001f)
            continue;

        // 辺の左側(反時計回りの内側)への距離
        float inside = (ex * (z - (float)a.z) - ez * (x - (float)a.x)) / len;
        if(inside < margin)
            return false;
    }
    return true;
}

void Ground::GetFieldRect(float& min_x, float& max_x, float& min_z, float& max_z) const
{
    min_x = min_z = FLT_MAX;
    max_x = max_z = -FLT_MAX;
    for(auto& v : field_vertices_) {
        min_x = std::min(min_x, (float)v.x);
        max_x = std::max(max_x, (float)v.x);
        min_z = std::min(min_z, (float)v.z);
        max_z = std::max(max_z, (float)v.z);
    }
}

void Ground::BuildWalls()
{
    // 作り直す場合は古い壁を消す
    for(auto& weak : walls_) {
        if(auto wall = weak.lock())
            Scene::Object::Release(wall);
    }
    walls_.clear();

    const size_t count = field_vertices_.size();
    if(count < 3)
        return;

    const float half_t = wall_thickness_ * 0.5f;
    const float half_h = wall_height_ * 0.5f;
    const float y      = wall_bottom_ + half_h;

    for(size_t i = 0; i < count; ++i) {
        const float3& a = field_vertices_[i];
        const float3& b = field_vertices_[(i + 1) % count];

        float dx  = (float)(b.x - a.x);
        float dz  = (float)(b.z - a.z);
        float len = sqrtf(dx * dx + dz * dz);
        if(len < 0.0001f)
            continue;
        dx /= len;
        dz /= len;

        // 外向きの法線(反時計回りの辺の右側)。内側の面が辺の上に来るように、厚さの半分だけ外へずらす
        float nx = dz;
        float nz = -dx;
        float mx = ((float)a.x + (float)b.x) * 0.5f + nx * half_t;
        float mz = ((float)a.z + (float)b.z) * 0.5f + nz * half_t;

        auto wall = Scene::Object::Create<Object>("StageWall_" + std::to_string(i));
        wall->SetTranslate({mx, y, mz});
        // 箱のローカルX軸を辺の向きに合わせる(Y軸回転。DxLibの左手系では X軸 → (cos, 0, -sin))
        wall->SetRotationAxisXYZ({0.0f, atan2f(-dz, dx) * RadToDeg, 0.0f});

        auto physics = wall->AddComponent<ComponentPhysics>();
        // 隣の壁との隙間ができないよう、長さを厚さぶん伸ばす
        physics->CreateBox({len * 0.5f + wall_thickness_, half_h, half_t});
        physics->SetCollisionType(ComponentPhysics::CollisionType::WALL);
        physics->SetFriction(0.0f);    // 壁に沿って滑れるように
        PhysicsHelper::MakeStatic(physics);

        walls_.push_back(wall);
    }
}

void Ground::Draw()
{
    Super::Draw();

    if(!show_field_line_)
        return;

    // フィールドの境界線を地面の少し上に描く(調整用)
    const size_t count = field_vertices_.size();
    for(size_t i = 0; i < count; ++i) {
        const float3& a = field_vertices_[i];
        const float3& b = field_vertices_[(i + 1) % count];
        DrawLine3D(VGet((float)a.x, (float)a.y + 0.5f, (float)a.z), VGet((float)b.x, (float)b.y + 0.5f, (float)b.z), GetColor(0, 255, 255));
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
    // フィールドの中心へ戻す
    const float3 center = field_center_;
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
        if(ImGui::TreeNode("Stage Field")) {
            ImGui::Text(u8"ステージモデル min(%.0f, %.0f, %.0f) max(%.0f, %.0f, %.0f)",
                        (float)bounds_.min_.x,
                        (float)bounds_.min_.y,
                        (float)bounds_.min_.z,
                        (float)bounds_.max_.x,
                        (float)bounds_.max_.y,
                        (float)bounds_.max_.z);
            ImGui::Checkbox(u8"境界線を表示", &show_field_line_);

            // 値を変えたらその場でフィールドと壁を作り直す
            bool change  = ImGui::DragFloat(u8"右(+X)までの距離", &field_right_, 1.0f, 10.0f, 1000.0f);
            change      |= ImGui::DragFloat(u8"左(-X)までの距離", &field_left_, 1.0f, 10.0f, 1000.0f);
            change      |= ImGui::DragFloat(u8"奥(+Z)までの距離", &field_back_, 1.0f, 10.0f, 1000.0f);
            change      |= ImGui::DragFloat(u8"手前(-Z)までの距離", &field_front_, 1.0f, 10.0f, 1000.0f);
            change      |= ImGui::DragFloat3(u8"中心", field_center_.RawArray(), 1.0f);
            change      |= ImGui::DragInt(u8"辺の数", &field_segments_, 0.2f, 8, 128);
            change      |= ImGui::DragFloat(u8"壁の高さ", &wall_height_, 1.0f, 10.0f, 5000.0f);
            if(change) {
                BuildField();
                BuildWalls();
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

}    // namespace PoittersPoint
