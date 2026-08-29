//---------------------------------------------------------------------------
//! @file   PoittersPoint_Log.cpp
//! @brief  PoittersPoint_Log
//---------------------------------------------------------------------------
#include "PoittersPoint_Log.h"
#include "Game/Scene/PoittersPoint_Stage.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include "Game/Component/ComponentGrabbable.h"

#include <System/Component/ComponentCollisionCapsule.h>    // ★これを追加

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

    // 左
    auto col1 = AddComponent<ComponentCollisionCapsule>();
    if(col1) {
        col1->SetRadius(3.0f);
        col1->SetHeight(5.0f);
        col1->SetTranslate({-8.5f, 2.0f, 0.0f});

        col1->UseGravity(false);
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

        col2->UseGravity(false);
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

        col3->UseGravity(false);
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

    auto grabbable = AddComponent<ComponentGrabbable>();
    if(grabbable) {
        grabbable->SetBounceOffset(0.0f);    // 初期は跳ねを抑える
        grabbable->SetLiftTime(1.0f);
    }

    //AddComponent<ComponentCollisionModel>();
    //if(auto collision = GetComponent<ComponentCollisionModel>()) {
    //    collision->AttachToModel();    // コリジョンをモデルに合わせる
    //}

    //SetScaleAxisXYZ({0.75f});

    //is_rolling_ = false;
    //roll_speed_ = 0.0f;

    return true;
}

void Log::Update()
{
    __super::Update();

    //if(is_rolling_) {
    //    float3 rot  = GetRotationAxisXYZ();
    //    rot.x      += roll_speed_;
    //    SetRotationAxisXYZ(rot);

    //    roll_speed_ *= 0.90f;    // 少し減衰を早める

    //    // ★ 0.5f 以下になったら完全にピタッと止める
    //    if(roll_speed_ < 0.5f) {
    //        roll_speed_ = 0.0f;
    //        is_rolling_ = false;
    //    }
    //}

    // 転がっている間
    if(is_rolling_) {
        // 丸太を回転させる
        float3 rot  = GetRotationAxisXYZ();
        rot.x      += roll_speed_;
        SetRotationAxisXYZ(rot);

        // 徐々に減速
        roll_speed_ *= 0.98f;

        // 十分遅くなったら停止
        if(std::abs(roll_speed_) < 0.01f) {
            roll_speed_ = 0.0f;
            is_rolling_ = false;
        }
    }

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
}

void Log::GUI()
{
    __super::GUI();
}

void Log::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    /*
    // 当たった相手の名前がEnemyだったら消去する
    auto name = hit_info.hit_collision_->GetOwner()->GetNameDefault();
    if(name == "Enemy") {
        hit_info.collision_->SetCollisionStatus(ComponentCollision::CollisionBit::DisableHit, true);
        // 自分を削除する
        Scene::Object::Release(SharedThis());
    }
    */


    //if(hit_info.hit_collision_->GetCollisionGroup() == ComponentCollision::CollisionGroup::GROUND) {
    //    if(auto grabbable = GetComponent<ComponentGrabbable>()) {
    //        grabbable->SetCanGrab(true);

    //        // 座標判定を行わず、着地状態だけチェック
    //        if(grabbable->IsGrounded()) {
    //            grabbable->SetBounceOffset(0.0f);
    //        }
    //        else if(grabbable->IsMoving()) {
    //            grabbable->Bounce();
    //        }
    //    }
    //}

if(hit_info.hit_collision_->GetCollisionGroup() == ComponentCollision::CollisionGroup::GROUND) {
        if(auto grabbable = GetComponent<ComponentGrabbable>()) {
            grabbable->SetCanGrab(true);

            if(grabbable->IsMoving()) {
                // 地面に着いたら丸太を水平にする
                float3 rot = GetRotationAxisXYZ();
                rot.x      = 0.0f;
                rot.z      = 0.0f;
                SetRotationAxisXYZ(rot);

                // 転がり開始
                is_rolling_ = true;
                roll_speed_ = 0.2f;

                // 跳ねさせない
                grabbable->SetBounceOffset(0.0f);
            }
        }
    }

    //// 地面に当たった時の処理
    //if(hit_info.hit_collision_->GetCollisionGroup() == ComponentCollision::CollisionGroup::GROUND) {
    //    if(auto grabbable = GetComponent<ComponentGrabbable>()) {
    //        grabbable->SetCanGrab(true);

    //        // ★一度着地したら（または転がり中なら）、完全に Bounce() を呼ぶのをやめる！
    //        // 落ちてくる「移動中」かつ「転がりが始まっていない」最初の瞬間だけ一度だけバウンドを考慮
    //        if(!is_rolling_ && !grabbable->IsGrounded()) {
    //            // 初回着地時に転がり開始
    //            is_rolling_ = true;
    //            roll_speed_ = 8.0f;    // 転がる勢い

    //            // バウンドはさせずにオフセット固定（これで振動をシャットアウトします）
    //            grabbable->SetBounceOffset(0.0f);
    //        }
    //        else {
    //            // 着地後は絶対に跳ね返り処理（Bounce）を起こさせない
    //            grabbable->SetBounceOffset(0.0f);
    //        }
    // }


    // 最後にこれを入れてください。ここでめりこみの解消などの処理を行っています。
    Super::OnHit(hit_info);
}

}    // namespace PoittersPoint
