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
    //__super::Init();

    SetTranslate({10.0f, 10.0f, -20.0f});

    SetName("Log");
    auto model = AddComponent<ComponentModel>("data/Game/Models/Log/Log.mv1");
    if(model) {
        // 丸太だけに限定してカラー（ディフューズ色）を設定する
        // ※お使いの ComponentModel に用意されている設定関数を呼び出します

        // パターンA: モデルの色を指定できる関数がある場合
        // model->SetColor({ 0.5f, 0.3f, 0.1f, 1.0f }); // 茶色

        // パターンB: マテリアルを取得して設定する場合
        // auto material = model->GetMaterial(0);
        // if(material) material->SetDiffuse({ 0.5f, 0.3f, 0.1f, 1.0f });
    }

    // 左
    auto col1 = AddComponent<ComponentCollisionCapsule>();
    if(col1) {
        col1->SetRadius(3.0f);
        col1->SetHeight(5.0f);
        col1->SetTranslate({-8.5f, 2.0f, 0.0f});

        col1->UseGravity(true);
        col1->SetGravity(-0.2f);
        col1->SetCollisionGroup(ComponentCollision::CollisionGroup::LOG);
        col1->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY | (u32)ComponentCollision::CollisionGroup::GROUND |
                                   (u32)ComponentCollision::CollisionGroup::ITEM | (u32)ComponentCollision::CollisionGroup::PLAYER);
        col1->SetMass(200.0f);
    }

    // 真ん中
    auto col2 = AddComponent<ComponentCollisionCapsule>();
    if(col2) {
        col2->SetRadius(3.0f);
        col2->SetHeight(5.0f);
        col2->SetTranslate({0.0f, 3.0f, 0.0f});

        col2->UseGravity(true);
        col2->SetGravity(-0.2f);
        col2->SetCollisionGroup(ComponentCollision::CollisionGroup::LOG);
        col2->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY | (u32)ComponentCollision::CollisionGroup::GROUND |
                                   (u32)ComponentCollision::CollisionGroup::ITEM | (u32)ComponentCollision::CollisionGroup::ETC |
                                   (u32)ComponentCollision::CollisionGroup::PLAYER);
        col2->SetMass(200.0f);
    }

    // 右
    auto col3 = AddComponent<ComponentCollisionCapsule>();
    if(col3) {
        col3->SetRadius(3.0f);
        col3->SetHeight(5.0f);
        col3->SetTranslate({8.5f, 3.0f, 0.0f});

        col3->UseGravity(true);
        col3->SetGravity(-0.2f);
        col3->SetCollisionGroup(ComponentCollision::CollisionGroup::LOG);
        col3->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY | (u32)ComponentCollision::CollisionGroup::GROUND |
                                   (u32)ComponentCollision::CollisionGroup::ITEM | (u32)ComponentCollision::CollisionGroup::PLAYER);
        col3->SetMass(200.0f);
    }

    // 右2
    auto col4 = AddComponent<ComponentCollisionCapsule>();
    if(col4) {
        col4->SetRadius(3.0f);
        col4->SetHeight(5.0f);
        col4->SetTranslate({-16.0f, 3.0f, 0.0f});

        col4->UseGravity(true);
        col4->SetGravity(-0.2f);
        col4->SetCollisionGroup(ComponentCollision::CollisionGroup::LOG);
        col4->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY | (u32)ComponentCollision::CollisionGroup::GROUND |
                                   (u32)ComponentCollision::CollisionGroup::ITEM | (u32)ComponentCollision::CollisionGroup::PLAYER);
        col4->SetMass(200.0f);
    }

    // 左２
    auto col5 = AddComponent<ComponentCollisionCapsule>();
    if(col5) {
        col5->SetRadius(3.0f);
        col5->SetHeight(5.0f);
        col5->SetTranslate({16.0f, 3.0f, 0.0f});

        col5->UseGravity(true);
        col5->SetGravity(-0.2f);
        col5->SetCollisionGroup(ComponentCollision::CollisionGroup::LOG);
        col5->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY | (u32)ComponentCollision::CollisionGroup::GROUND |
                                   (u32)ComponentCollision::CollisionGroup::ITEM | (u32)ComponentCollision::CollisionGroup::PLAYER);
        col5->SetMass(200.0f);
    }

    const float3 col_offsets[] = {
        { -8.5f, 2.0f, 0.0f},
        {  0.0f, 3.0f, 0.0f},
        {  8.5f, 3.0f, 0.0f},
        {-16.0f, 3.0f, 0.0f},
        { 16.0f, 3.0f, 0.0f}
    };

    auto grabbable = AddComponent<ComponentGrabbable>();
    if(grabbable) {
        grabbable->SetBounceOffset(0.0f);    // 初期は跳ねを抑える
        grabbable->SetLiftTime(1.0f);
    }

    //AddComponent<ComponentCollisionModel>();
    //if(auto collision = GetComponent<ComponentCollisionModel>()) {
    //    collision->AttachToModel();    // コリジョンをモデルに合わせる
    //}

    //SE
    se_handle_ = LoadSoundMem("data/Game/LogSE/Log1.mp3");
    //throw_cool_down_ = 0.0f;
    //is_bounced_      = false;

    // 転がり関連の初期化
    is_rolling_ = false;
    roll_speed_ = 0.0f;

    return true;
}

void Log::Update()
{
    __super::Update();

    // 地面に固定されている時は跳ねを抑える
    if(auto grabbable = GetComponent<ComponentGrabbable>()) {
        if(grabbable->IsGrounded() && !grabbable->IsMoving()) {
            grabbable->SetBounceOffset(0.0f);
        }
    }

    // 地面に固定されている時は無駄な力を抑え込んで滑りを止める
    if(auto grabbable = GetComponent<ComponentGrabbable>()) {
        if(grabbable->IsGrounded() && !grabbable->IsMoving()) {
            grabbable->SetBounceOffset(0.0f);
        }
    }

    if(auto grabbable = GetComponent<ComponentGrabbable>()) {
        // 転がるアニメーション＆移動処理
        if(is_rolling_ && roll_speed_ > 0.01f) {
            // 現在の座標と回転を取得
            float3 pos = GetTranslate();
            float3 rot = GetRotationAxisXYZ();

            // 速度に合わせて移動 ( roll_dir_ 方向へ移動 )
            pos += roll_dir_ * roll_speed_ * GetDeltaTime();

            // 速度に合わせてX軸回りに転がる（回転）
            rot.x += roll_speed_ * 2.0f * GetDeltaTime();

            SetTranslate(pos);
            SetRotationAxisXYZ(rot);

            // 地面の摩擦によって減速（毎フレーム92%に減衰）
            roll_speed_ *= 0.92f;

            // 十分遅くなったら転がり停止
            if(roll_speed_ <= 0.05f) {
                roll_speed_ = 0.0f;
                is_rolling_ = false;
            }
        }
        else if(grabbable->IsGrounded() && !grabbable->IsMoving()) {
            // 完全接地時
            grabbable->SetBounceOffset(0.0f);
        }
    }

}

void Log::GUI()
{
    __super::GUI();
}

void Log::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    if(hit_info.hit_collision_->GetCollisionGroup() == ComponentCollision::CollisionGroup::GROUND) {
        if(auto grabbable = GetComponent<ComponentGrabbable>()) {
            grabbable->SetCanGrab(true);

            // 座標判定を行わず、着地状態だけチェック
            if(grabbable->IsGrounded()) {
                grabbable->SetBounceOffset(0.0f);
            }
            else if(grabbable->IsMoving()) {
                grabbable->Bounce();

                // ----------------------------------------------------
                // 丸太を投げて地面に落ちた時に丸太のSEを流す
                // ----------------------------------------------------
                //PlaySoundFile("data/Game/LogSE/Log1.mp3", DX_PLAYTYPE_BACK);
                //再生
                if(se_handle_ != -1) {
                    PlaySoundMem(se_handle_, DX_PLAYTYPE_BACK);
                }
            }
        }
    }
    // 最後にこれを入れてください。ここでめりこみの解消などの処理を行っています。
    Super::OnHit(hit_info);
}

}    // namespace PoittersPoint
