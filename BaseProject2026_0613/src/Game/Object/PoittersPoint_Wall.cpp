//---------------------------------------------------------------------------
//! @file   PoittersPoint_Ground.cpp
//! @brief  PoittersPoint_Ground
//---------------------------------------------------------------------------
#include "PoittersPoint_Ground.h"
#include "Game/Scene/PoittersPoint_Stage.h"
#include "PoittersPoint_Player.h"
#include "PoittersPoint_Wall.h"


#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollision.h>
#include <System/Component/ComponentCollisionModel.h>
#include "Game/Component/ComponentCameraController.h"
#include "Game/Component/ComponentStateIdleWalk.h"
#include "Game/Component/ComponentStateThrow.h"
#include "Game/Component/ComponentPlayerState.h"
#include "Game/Component/ComponentStateMachine.h"

#include "Game/Component/ComponentGrabbable.h"
#include <System/Component/ComponentObjectController.h>
#include "Game/Component/ComponentHitPoints.h"

namespace PoittersPoint {
// namespace PoittersPoint

//! @brief 初期化
//! @return 初期化終了
bool Wall::Init()
{
    Super::Init();

  auto   ground = Scene::Object::Get<Ground>();
    float3 center = {0, 0, 0};

    if(ground) {
        center = ground->GetTranslate();
    }

    float radius = m_radius;
    float height = 35.0f;
    int   div    = 48;

    //コリジョンの高さ補正
    float wallY = center.y - 55.0f;

    for(int i = 0; i < div; i++) {
        float theta = DX_TWO_PI_F * (float)i / (float)div;

        float x = center.x + cosf(theta) * radius;
        float z = center.z + sinf(theta) * radius;

        auto col = AddComponent<ComponentCollisionCapsule>();
        col->SetCollisionGroup(ComponentCollision::CollisionGroup::GROUND);
        col->SetRadius(2.0f);
        col->SetHeight(height);

        col->SetTranslate({x, wallY, z});    //下げた高さを使う
    }


    //float radius = 120.0f;
    //float height = 35.0f;
    //int   div    = 48;

    //float3 center = {0, 0, 0};

    //for(int i = 0; i < div; i++) {
    //    float theta = DX_TWO_PI_F * (float)i / (float)div;

    //    float x = center.x + cosf(theta) * radius;
    //    float z = center.z + sinf(theta) * radius;

    //    auto col = AddComponent<ComponentCollisionCapsule>();
    //    col->SetCollisionGroup(ComponentCollision::CollisionGroup::GROUND);
    //    col->SetRadius(2.0f);
    //    col->SetHeight(height);
    //    col->SetTranslate({x, center.y - 59, z});
    //}

    return true;    // ★ 必須}
}

void Wall::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    auto hitCol = hit_info.hit_collision_;
    if(!hitCol)
        return;

    auto obj = hitCol->GetOwner();
    if(!obj)
        return;

    // ★ Wall 自身なら何もしない（これが最重要）
    if(obj == this) {
        Super::OnHit(hit_info);
        return;
    }

    float3 center = GetTranslate();
    float3 pos    = obj->GetTranslate();

    float3 dir;
    dir.x = pos.x - center.x;
    dir.y = pos.y - center.y;
    dir.z = pos.z - center.z;

    float dist = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);

    float radius = m_radius - 2.0f;

    if(dist > radius) {
        float  inv  = 1.0f / dist;
        float3 ndir = {dir.x * inv, dir.y * inv, dir.z * inv};

        float3 newPos = {center.x + ndir.x * radius, pos.y, center.z + ndir.z * radius};

        obj->SetTranslate(newPos);
    }

    Super::OnHit(hit_info);
}


// Wallクラスや専用の描画用コンポーネントの描画フェーズ（Draw等）で呼ぶ
void Wall::Draw()
{
    // 基底クラスの描画
    Super::Draw();

    //// --- 地面に太い円（範囲を示すライン）を描画する ---
    //int   div    = 100;    // 円の滑らかさ（分割数を少し増やすと綺麗になります）
    //float radius = m_radius;
    //auto  center = GetTranslate();

    //// 太さを出すために、半径を少しずつずらした円を複数重ねて描画する
    //// 例: -0.6f から +0.6f の幅で、0.2f 刻みで線を重ねる
    //for(float rOffset = -0.6f; rOffset <= 0.6f; rOffset += 0.2f) {
    //    float currentRadius = radius + rOffset;

    //    for(int i = 0; i < div; i++) {
    //        float theta1 = (float)i / div * DX_TWO_PI_F;
    //        float theta2 = (float)(i + 1) / div * DX_TWO_PI_F;

    //        float x1 = center.x + cosf(theta1) * currentRadius;
    //        float z1 = center.z + sinf(theta1) * currentRadius;
    //        float x2 = center.x + cosf(theta2) * currentRadius;
    //        float z2 = center.z + sinf(theta2) * currentRadius;

    //        float y = center.y + 0.1f;

    //        // 3D空間に線を引く
    //        DrawLine3D(VGet(x1, y, z1), VGet(x2, y, z2), GetColor(0, 255, 255));
    //    }
    //}

    //// ==========================================
    //// 2. 「色付きの壁（フェンス・円柱）」を3D描画で表現する
    //// ==========================================
    //// 底面から上に向かって高さを変えながら、何段かリングを描画して壁にする
    //float wallHeight  = 35.0f;    // 壁の高さ
    //int   heightSteps = 5;        // 高さ方向の分割数

    //for(int h = 0; h <= heightSteps; h++) {
    //    // 下から上へ段階的に高さを計算
    //    float currentY = center.y + (wallHeight * ((float)h / heightSteps));

    //    for(int i = 0; i < div; i++) {
    //        float theta1 = (float)i / div * DX_TWO_PI_F;
    //        float theta2 = (float)(i + 1) / div * DX_TWO_PI_F;

    //        float x1 = center.x + cosf(theta1) * radius;
    //        float z1 = center.z + sinf(theta1) * radius;
    //        float x2 = center.x + cosf(theta2) * radius;
    //        float z2 = center.z + sinf(theta2) * radius;

    //        // ① 水平方向（リング状）のラインを描画（色：赤紫や好みの色）
    //        DrawLine3D(VGet(x1, currentY, z1), VGet(x2, currentY, z2), GetColor(255, 0, 128));

    //        // ② 垂直方向（柱状）のラインを一定間隔ごとに描画して網目・壁っぽくする
    //        if(h < heightSteps && i % (div / 8) == 0) {
    //            float nextY = center.y + (wallHeight * ((float)(h + 1) / heightSteps));
    //            DrawLine3D(VGet(x1, currentY, z1), VGet(x1, nextY, z1), GetColor(255, 0, 128));
    //        }
    //    }
    //}
}

}    // namespace PoittersPoint
