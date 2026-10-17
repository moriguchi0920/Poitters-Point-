//---------------------------------------------------------------------------
//! @file   PoittersPoint_Log.cpp
//! @brief  PoittersPoint_Log
//---------------------------------------------------------------------------
#include "PoittersPoint_Log.h"
#include "Game/Scene/PoittersPoint_Stage.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include "Game/Component/ComponentGrabbable.h"
#include <System/Component/ComponentCollisionCapsule.h>

namespace PoittersPoint {
// namespace PoittersPoint

bool Log::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();

    SetTranslate({10.0f, 10.0f, -20.0f});

    SetName("Log");
    auto model = AddComponent<ComponentModel>("data/Game/Models/Log/Log.mv1");
    if(model) {
        // モデルの読み込み成功時の追加設定はここに
    }

    // 丸太の形状に合致する単一カプセル衝突
    // 5つに分割していた個別カプセルを1つに統合し、
    // 重力・反発・跳ね返りの競合を解消
    auto col = AddComponent<ComponentCollisionCapsule>();
    if(col) {
        col->SetRadius(3.0f);
        col->SetHeight(5.0f);

        col->UseGravity(true);
        col->SetGravity(-0.2f);
        col->SetCollisionGroup(ComponentCollision::CollisionGroup::LOG);
        col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY |
                                  (u32)ComponentCollision::CollisionGroup::GROUND |
                                  (u32)ComponentCollision::CollisionGroup::ITEM |
                                  (u32)ComponentCollision::CollisionGroup::PLAYER);
        col->SetMass(200.0f);
    }

    auto grabbable = AddComponent<ComponentGrabbable>();
    if(grabbable) {
        grabbable->SetBounceOffset(0.0f);    // 初期は跳ねを抑える
        grabbable->SetLiftTime(1.0f);
    }

    //SE
    se_handle_ = LoadSoundMem("data/Game/LogSE/Log1.mp3");
    throw_cool_down_ = 0.0f;

    // 転がり計算用に前フレームの位置を初期化
    prev_pos_ = GetTranslate();

    SetScaleAxisXYZ({1.0f});

    return true;
}

void Log::Update()
{
    __super::Update();

    // 物理演算前の位置を記録（PostPhysicsで転がり回転に使用）
    prev_pos_ = GetTranslate();

    auto grabbable = GetComponent<ComponentGrabbable>();
    if(!grabbable)
        return;

    // 接地中かつ静止中は跳ねを抑制
    if(grabbable->IsGrounded() && !grabbable->IsMoving()) {
        grabbable->SetBounceOffset(0.0f);
    }
}

void Log::PostPhysics()
{
    __super::PostPhysics();

    auto grabbable = GetComponent<ComponentGrabbable>();
    if(!grabbable)
        return;

    // 空中中は視覚的な転がり回転をしない
    if(!grabbable->IsGrounded())
        return;

    // 物理演算による移動量を取得
    float3 delta = GetTranslate() - prev_pos_;

    // 水平面（XZ）の移動量
    float3 horiz_delta = {delta.x, 0.0f, delta.z};
    float  horiz_dist  = length(horiz_delta);

    if(horiz_dist > 0.001f) {
        // 丸太（X軸を中心とするシリンダー）の転がり：
        // 水平移動量 / 半径 = 回転角（ラジアン）
        // ラジアンを度に変換: * (180 / PI)
        float radius = 3.0f;
        float angle_deg = (horiz_dist / radius) * 57.2957795f;    // 180.0f / 3.14159265f

        // Z方向の移動が進行方向の時は負の回転（右手座標系）
        if(delta.z < 0.0f)
            angle_deg = -angle_deg;

        // X軸周りに累積回転
        float3 rot = GetRotationAxisXYZ();
        rot.x += angle_deg;
        SetRotationAxisXYZ(rot);
    }
}

void Log::GUI()
{
    __super::GUI();
}

void Log::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    if(hit_info.hit_collision_->GetCollisionGroup() == ComponentCollision::CollisionGroup::GROUND) {
        // SEクールダウンタイマーの更新（DeltaTimeで減算）
        if(throw_cool_down_ > 0.0f) {
            throw_cool_down_ -= GetDeltaTime();
            if(throw_cool_down_ < 0.0f) {
                throw_cool_down_ = 0.0f;
            }
        }

        if(auto grabbable = GetComponent<ComponentGrabbable>()) {
            grabbable->SetCanGrab(true);

            // クールダウンタイマーが終了している時のみ判定
            if(throw_cool_down_ <= 0.0f) {
                // 【着地した瞬間の判定】
                // 落下・移動中で、かつまだ接地状態になっていない瞬間にSEを鳴らしバウンド
                if(!grabbable->IsGrounded() && grabbable->IsMoving()) {
                    if(se_handle_ != -1) {
                        PlaySoundMem(se_handle_, DX_PLAYTYPE_BACK);
                        throw_cool_down_ = 1.0f;    // 連打防止
                    }
                    grabbable->Bounce();
                }
            }

            // 着地後の跳ね抑制処理
            if(grabbable->IsGrounded()) {
                grabbable->SetBounceOffset(0.0f);
            }
        }
    }

    // 最後にこれを入れてください。ここでめりこみの解消などの処理を行っています。
    Super::OnHit(hit_info);
}

}    // namespace PoittersPoint
