//------------------------------------------------------------------------------
//! @file   PoittersPoint_Log.cpp
//! @brief  PoittersPoint_Log
//------------------------------------------------------------------------------
#include "PoittersPoint_Log.h"
#include "Game/Scene/PoittersPoint_Stage.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include "Game/Component/ComponentGrabbable.h"

namespace PoittersPoint {
bool Log::Init()
{
    SetTranslate({10.0f, 10.0f, -20.0f});

    SetName("Log");
    auto model = AddComponent<ComponentModel>("data/Game/Models/Log/Log.mv1");
    if(model) {
        // 丸太の物理軸（Y=+3.3 位置）をオブジェクト原点へ一致させるためモデルをローカルで -3.3 下げる
        // カプセル側も同量移動させるため、見た目と当たり判定の相対配置は変わらない
        model->SetTranslate({0.0f, -3.3f, 0.0f});
    }

    // 丸太の形状に合致する単一カプセル(ComponentCollisionModelはリグなどがないのかアタッチ出来ない)
    // カプセルは初期で縦向きだが、丸太は横向きなので回転させる
    auto col = AddComponent<ComponentCollisionCapsule>();
    if(col) {
        col->SetRadius(6.0f);
        col->SetHeight(46.33f);
        // 軸をオブジェクト原点(Y=0)へ移す（モデルを -3.3 した分と相殺して見た目は不変）
        // これにより X 軸回転が丸太の物理軸まわりの pure な転がりになり首振りが消える
        col->SetTranslate({23.36f, 2.530f, 0.0f});

        // カプセルを横向き（丸太の長軸に沿わせる）に回転
        // 回転: {-90, 90, 0} (度)
        float matrixT[3], matrixR[3], matrixS[3];
        DecomposeMatrixToComponents(col->GetMatrixFloat(), matrixT, matrixR, matrixS);
        matrixR[0] = -90.0f;
        matrixR[1] = 90.0f;
        matrixR[2] = 0.0f;
        RecomposeMatrixFromComponents(matrixT, matrixR, matrixS, col->GetMatrixFloat());

        // 重力を適用
        col->UseGravity(true);
        col->SetGravity(-0.2f);
        col->SetCollisionGroup(ComponentCollision::CollisionGroup::LOG);
        col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY | (u32)ComponentCollision::CollisionGroup::GROUND |
                                  (u32)ComponentCollision::CollisionGroup::ITEM | (u32)ComponentCollision::CollisionGroup::PLAYER);
        col->SetMass(200.0f);
    }

    auto grabbable = AddComponent<ComponentGrabbable>();
    grabbable->SetLiftTime(1.5f);
    grabbable->SetDamage(1.0f);

    return true;
}

void Log::Update()
{
    __super::Update();

    // 投擲処理
    if(just_thrown_) {
        just_thrown_ = false;
    }
}

void Log::GUI()
{
}

void Log::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    // 最後に入れる。ここでめりこみの解消などの処理を行っています。
    Super::OnHit(hit_info);
}

void Log::PrePhysics()
{
    // 物理演算前の位置を記録（転がり回転計算用）
    prev_pos_ = GetTranslate();
}

void Log::PostPhysics()
{
    // 物理演算後の位置から速度を算出し、転がり回転を適用
    float3 now_pos = GetTranslate();
    velocity_      = now_pos - prev_pos_;

    if(is_on_ground_ && (velocity_.x != 0.0f || velocity_.z != 0.0f)) {
        is_rolling_ = true;
        roll_speed_ = 0.2f;
    }
}

}    // namespace PoittersPoint
