//---------------------------------------------------------------------------
//! @file   PoittersPoint_Camera.cpp
//! @brief  PoittersPoint_Camera
//---------------------------------------------------------------------------
#include "PoittersPoint_Camera.h"
#include "PoittersPoint_Player.h"
#include "Game/Scene/PoittersPoint_Stage.h"
#include "Game/Component/ComponentStateDead.h"

#include <System/Scene.h>
#include <System/ImGui.h>
#include <System/Component/ComponentModel.h>

#include <algorithm>
#include <cmath>

namespace PoittersPoint {
// namespace PoittersPoint

namespace {

//! @brief フレームレートに依存しない補間係数を求める
//! @param speed 追従速度(1秒あたり)
//! @param dt    経過時間(秒)
float SmoothFactor(float speed, float dt)
{
    return 1.0f - expf(-speed * dt);
}

//! @brief XZ平面上の外積(凸包計算用)
float Cross2D(const float3& o, const float3& a, const float3& b)
{
    return (a.x - o.x) * (b.z - o.z) - (a.z - o.z) * (b.x - o.x);
}

//! @brief XZ平面上の凸包を求める(Andrewのモノトーンチェーン法)
//! @return 反時計回りに並んだ凸包の頂点
std::vector<float3> ConvexHullXZ(std::vector<float3> pts)
{
    if(pts.size() < 3) {
        return pts;
    }
    std::sort(pts.begin(), pts.end(), [](const float3& a, const float3& b) {
        return (a.x < b.x) || (a.x == b.x && a.z < b.z);
    });

    std::vector<float3> hull(pts.size() * 2);
    size_t              k = 0;
    // 下側
    for(size_t i = 0; i < pts.size(); ++i) {
        while(k >= 2 && Cross2D(hull[k - 2], hull[k - 1], pts[i]) <= 0.0f) {
            --k;
        }
        hull[k++] = pts[i];
    }
    // 上側
    for(size_t i = pts.size() - 1, t = k + 1; i > 0; --i) {
        while(k >= t && Cross2D(hull[k - 2], hull[k - 1], pts[i - 1]) <= 0.0f) {
            --k;
        }
        hull[k++] = pts[i - 1];
    }
    hull.resize(k - 1);
    return hull;
}

}    // namespace

//! @brief 初期化
//! @return 初期化終了
bool Camera::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();

    SetName("Camera");

    AddComponent<ComponentCamera>();    //カメラコンポーネントを付ける
    if(auto c = GetComponent<ComponentCamera>()) {
        c->SetPerspective(fovy_deg_, aspect_ratio_);
        // 生存者がまだ揃っていない間のステージ全体を見渡す初期位置
        c->SetPositionAndTarget({0, 80, 205}, {0, 0, -60});
    }

    initialized_ = false;
    return true;
}

//! @brief 物理処理後の更新
void Camera::PostPhysics()
{
    Super::PostPhysics();

    survivors_ = CollectSurvivorPositions();

    // 生存者がいなければ直前の状態を維持する
    if(!survivors_.empty()) {
        desired_target_ = CalcShapeCenter(survivors_);
    }

    float dt = GetDeltaTime();
    dt       = std::clamp(dt, 0.0f, 0.1f);    // ヒッチ時に飛びすぎないように制限

    if(!initialized_) {
        if(survivors_.empty()) {
            return;    // 誰もいない間は初期位置のまま
        }
        // 初回は補間せずに一気に合わせる
        current_target_   = desired_target_;
        desired_distance_ = CalcRequiredDistance(survivors_, current_target_);
        current_distance_ = desired_distance_;
        initialized_      = true;
    }
    else {
        // 注視点を滑らかに追従
        current_target_ = lerp(current_target_, desired_target_, SmoothFactor(target_follow_speed_, dt));

        // 実際に見ている(補間後の)注視点から全員が映る距離を求める
        if(!survivors_.empty()) {
            desired_distance_ = CalcRequiredDistance(survivors_, current_target_);
        }

        // 引くときは速く、寄るときはゆっくり(映り切らない瞬間を作らないため)
        float speed       = (desired_distance_ > current_distance_) ? zoom_out_speed_ : zoom_in_speed_;
        current_distance_ = current_distance_ + (desired_distance_ - current_distance_) * SmoothFactor(speed, dt);
    }

    ApplyToCamera();
}

//! @brief 現在の生存者の位置を集める
std::vector<float3> Camera::CollectSurvivorPositions() const
{
    std::vector<float3> result;

    for(auto& player : Scene::Object::GetArray<Player>()) {
        if(!player) {
            continue;
        }
        // 死亡状態に入った(死亡処理が終わると名前が"dead"になる)キャラクターは除外
        if(player->GetComponent<ComponentStateDead>() || player->GetName() == "dead") {
            continue;
        }
        // ※体力0では除外しない(現状プレイヤーは体力0でも行動を続けるため。脱落は死亡ステートか場外落下で判定)
        float3 pos = player->GetTranslate();
        // ステージ外へ落下したキャラクターは除外
        if(pos.y < fall_out_height_) {
            continue;
        }
        result.push_back(pos);
    }
    return result;
}

//! @brief 生存者が作る図形の中心点を求める
float3 Camera::CalcShapeCenter(const std::vector<float3>& points)
{
    // 頂点の平均(1人:その位置 / 2人:中点 / 3人:三角形の重心 と一致する)
    float3 mean = {0, 0, 0};
    for(auto& p : points) {
        mean += p;
    }
    mean /= (float)points.size();

    if(points.size() < 3) {
        return mean;
    }

    // 3人以上は生存者を囲む多角形(凸包)の面積重心を中心点とする
    auto hull = ConvexHullXZ(points);
    if(hull.size() < 3) {
        return mean;    // 一直線に並んでいる場合
    }

    float  area2 = 0.0f;    // 面積の2倍
    float3 c     = {0, 0, 0};
    for(size_t i = 0; i < hull.size(); ++i) {
        const float3& a = hull[i];
        const float3& b = hull[(i + 1) % hull.size()];
        float         w = a.x * b.z - b.x * a.z;
        area2 += w;
        c.x += (a.x + b.x) * w;
        c.z += (a.z + b.z) * w;
    }
    if(fabsf(area2) < 1.0e-3f) {
        return mean;
    }
    c.x /= (3.0f * area2);
    c.z /= (3.0f * area2);
    c.y = mean.y;    // 高さは平均
    return c;
}

//! @brief カメラの前方向
float3 Camera::GetForward() const
{
    float pitch = pitch_deg_ * DegToRad;
    float yaw   = yaw_deg_ * DegToRad;
    // yaw=0 のとき +Z 側から -Z 方向を見下ろす
    return float3(-sinf(yaw) * cosf(pitch), -sinf(pitch), -cosf(yaw) * cosf(pitch));
}

//! @brief 全員を画面に収めるために必要なカメラ距離を求める
float Camera::CalcRequiredDistance(const std::vector<float3>& points, const float3& target) const
{
    float3 fwd   = normalize(GetForward());
    float  yaw   = yaw_deg_ * DegToRad;
    float3 right = normalize(float3(cosf(yaw), 0.0f, -sinf(yaw)));
    float3 up    = normalize(cross(fwd, right));

    // 画面端から少し内側に収める
    float tan_v = tanf(fovy_deg_ * 0.5f * DegToRad) * screen_margin_;
    float tan_h = tan_v * aspect_ratio_;

    float required = min_distance_;

    for(auto& p : points) {
        // 足元・頭・体の左右を判定点にする
        const float3 samples[] = {
            p,
            p + float3(0, body_height_, 0),
            p + right * body_radius_,
            p - right * body_radius_,
            p + right * body_radius_ + float3(0, body_height_, 0),
            p - right * body_radius_ + float3(0, body_height_, 0),
        };
        for(auto& s : samples) {
            float3 rel = s - target;
            float  x   = fabsf(dot(rel, right));
            float  y   = fabsf(dot(rel, up));
            float  z   = dot(rel, fwd);    // 注視点より奥ならプラス
            // カメラ空間の奥行き (distance + z) が x/tan_h, y/tan_v 以上なら画面内
            float need = std::max(x / tan_h, y / tan_v) - z;
            required   = std::max(required, need);
        }
    }
    return std::clamp(required, min_distance_, max_distance_);
}

//! @brief 現在の注視点・距離をカメラコンポーネントへ反映
void Camera::ApplyToCamera()
{
    if(auto c = GetComponent<ComponentCamera>()) {
        float3 position = current_target_ - normalize(GetForward()) * current_distance_;
        c->SetPerspective(fovy_deg_, aspect_ratio_);
        c->SetPositionAndTarget(position, current_target_);
    }
}

//! @brief 描画(デバッグ表示)
void Camera::Draw()
{
    Super::Draw();

    if(!debug_draw_ || survivors_.empty()) {
        return;
    }

    const unsigned int shape_color  = GetColor(255, 255, 0);
    const unsigned int target_color = GetColor(255, 0, 0);
    const unsigned int center_color = GetColor(0, 255, 0);

    // 生存者が作る図形
    auto shape = (survivors_.size() >= 3) ? ConvexHullXZ(survivors_) : survivors_;
    for(size_t i = 0; i < shape.size(); ++i) {
        const float3& a = shape[i];
        const float3& b = shape[(i + 1) % shape.size()];
        DrawLine3D(VGet(a.x, a.y + 1.0f, a.z), VGet(b.x, b.y + 1.0f, b.z), shape_color);
    }

    // 十字マーカー
    auto drawCross = [](const float3& p, float size, unsigned int color) {
        DrawLine3D(VGet(p.x - size, p.y + 1.0f, p.z), VGet(p.x + size, p.y + 1.0f, p.z), color);
        DrawLine3D(VGet(p.x, p.y + 1.0f, p.z - size), VGet(p.x, p.y + 1.0f, p.z + size), color);
        DrawLine3D(VGet(p.x, p.y + 1.0f - size, p.z), VGet(p.x, p.y + 1.0f + size, p.z), color);
    };
    drawCross(desired_target_, 6.0f, center_color);    // 図形の中心点(目標)
    drawCross(current_target_, 4.0f, target_color);    // 現在の注視点(補間中)
}

//! @brief GUI表示
void Camera::GUI()
{
    Super::GUI();

    ImGui::Begin(GetName().data());
    {
        ImGui::Separator();
        ImGui::Text(u8"生存者数: %d", (int)survivors_.size());
        ImGui::Text(u8"注視点: (%.1f, %.1f, %.1f)", current_target_.x, current_target_.y, current_target_.z);
        ImGui::Text(u8"距離: %.1f (目標 %.1f)", current_distance_, desired_distance_);
        ImGui::Checkbox(u8"図形と中心点を表示", &debug_draw_);
        ImGui::DragFloat(u8"画角(縦)", &fovy_deg_, 0.1f, 10.0f, 120.0f, "%.1f");
        ImGui::DragFloat(u8"見下ろし角度", &pitch_deg_, 0.1f, 5.0f, 89.0f, "%.1f");
        ImGui::DragFloat(u8"水平角度", &yaw_deg_, 0.5f, -180.0f, 180.0f, "%.1f");
        ImGui::DragFloat(u8"画面内に収める割合", &screen_margin_, 0.01f, 0.3f, 1.0f, "%.2f");
        ImGui::DragFloat(u8"最小距離", &min_distance_, 1.0f, 10.0f, 1000.0f, "%.1f");
        ImGui::DragFloat(u8"最大距離", &max_distance_, 1.0f, 10.0f, 2000.0f, "%.1f");
        ImGui::DragFloat(u8"注視点の追従速度", &target_follow_speed_, 0.05f, 0.1f, 20.0f, "%.2f");
        ImGui::DragFloat(u8"寄る速度", &zoom_in_speed_, 0.05f, 0.1f, 20.0f, "%.2f");
        ImGui::DragFloat(u8"引く速度", &zoom_out_speed_, 0.05f, 0.1f, 20.0f, "%.2f");
    }
    ImGui::End();
}

}    // namespace PoittersPoint
