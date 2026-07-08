//---------------------------------------------------------------------------
//! @file   PoittersPoint_Rock.cpp
//! @brief  PoittersPoint_Rock
//---------------------------------------------------------------------------
#include "PoittersPoint_Rock.h"
#include "Game/Scene/PoittersPoint_Stage.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include"Game/Component/ComponentGrabbable.h"

namespace PoittersPoint {
// namespace PoittersPoint

bool Rock::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();
    //__super::Init();

    SetTranslate({0, 5, 0});

    SetName("Rock");
    AddComponent<ComponentModel>("data/Game/Models/Rock/rock.mv1");

    auto col = AddComponent<ComponentCollisionSphere>()->SetRadius(3.0f);

    col->SetCollisionGroup(ComponentCollision::CollisionGroup::ETC);
    col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY | (u32)ComponentCollision::CollisionGroup::GROUND |
                              (u32)ComponentCollision::CollisionGroup::ITEM);
    AddComponent<ComponentGrabbable>();

    //AddComponent<ComponentCollisionModel>();
    //if(auto collision = GetComponent<ComponentCollisionModel>()) {
    //    collision->AttachToModel();    // コリジョンをモデルに合わせる
    //}



    return true;
}

void Rock::Update()
{
    __super::Update();

}

void Rock::GUI()
{
    __super::GUI();
}



void Rock::OnHit(const ComponentCollision::HitInfo& hit_info)
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

    // 最後にこれを入れてください。ここでめりこみの解消などの処理を行っています。
    Super::OnHit(hit_info);
}

}    // namespace PoittersPoint
