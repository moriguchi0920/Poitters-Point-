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

namespace PoittersPoint 
{
// namespace PoittersPoint

bool Log::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();
    //__super::Init();

    SetTranslate({10, 10, 10});

    SetName("Log");
    auto model = AddComponent<ComponentModel>("data/Game/Models/Log/Log.mv1");
    if(model) {
        // ★ モデルの位置を上に持ち上げる（高さ 5.0f の半分である 2.5f 持ち上げる）
        model->SetTranslate({0.0f, 2.5f, 0.0f});
    }

    // カプセルコリジョン
    auto col = AddComponent<ComponentCollisionCapsule>();


    col->SetRotationAxisXYZ({0.0f, 0.0f, 90.0f});

    //// 2. 丸太の幅と太さに合わせてサイズを設定
    //col->SetRadius(5.8f);     // 丸太の半径（高さの半分）
    //col->SetHeight(42.0f);    // 丸太の横幅

    //// 3. 中心位置の調整（必要に応じて）
    //col->SetTranslate({20.0f, 8.4f, 0.0f});
    
    // 2. 丸太の幅と太さに合わせてサイズを設定
    col->SetRadius(5.8f);     // 丸太の半径（高さの半分）
    col->SetHeight(42.0f);    // 丸太の横幅

    // 3. 中心位置の調整（必要に応じて）
    col->SetTranslate({20.0f, 7.0f, 0.0f});

    col->UseGravity(true);
    col->SetGravity(-0.2f);
    col->SetCollisionGroup(ComponentCollision::CollisionGroup::ETC);
    col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY | (u32)ComponentCollision::CollisionGroup::GROUND |
                              (u32)ComponentCollision::CollisionGroup::ITEM | (u32)ComponentCollision::CollisionGroup::PLAYER);
    col->SetMass(500.0f);

    auto grabbable = AddComponent<ComponentGrabbable>();
    //grabbable->SetBounceOffset(0.2f);
    //grabbable->SetLiftTime(1.0f);
    if(grabbable) {
        grabbable->SetBounceOffset(0.0f);    // 初期は跳ねを抑える
        grabbable->SetLiftTime(1.0f);
    }
    //AddComponent<ComponentCollisionModel>();
    //if(auto collision = GetComponent<ComponentCollisionModel>()) {
    //    collision->AttachToModel();    // コリジョンをモデルに合わせる
    //}

    //SetScaleAxisXYZ({0.75f});

    return true;
}

void Log::Update()
{
    __super::Update();

    if(is_rolling_) {
        float3 rot  = GetRotationAxisXYZ();
        rot.x      += roll_speed_;
        SetRotationAxisXYZ(rot);

        roll_speed_ *= 0.90f;    // 少し減衰を早める

        // ★ 0.5f 以下になったら完全にピタッと止める
        if(roll_speed_ < 0.5f) {
            roll_speed_ = 0.0f;
            is_rolling_ = false;
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

    if(hit_info.hit_collision_->GetCollisionGroup() == ComponentCollision::CollisionGroup::GROUND) {
        if(auto grabbable = GetComponent<ComponentGrabbable>()) {
            float3 translation = grabbable->GetTranslation();
            grabbable->SetCanGrab(true);

            if(grabbable->IsGrounded()) {
                grabbable->SetBounceOffset(0.2f);
            }
            else if(grabbable->IsMoving()) {
                grabbable->Bounce();
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
    }

    // 最後にこれを入れてください。ここでめりこみの解消などの処理を行っています。
    Super::OnHit(hit_info);
}

}    // namespace PoittersPoint
