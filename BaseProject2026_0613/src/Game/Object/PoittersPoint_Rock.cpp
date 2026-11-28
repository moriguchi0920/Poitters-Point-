//---------------------------------------------------------------------------
//! @file   PoittersPoint_Rock.cpp
//! @brief  PoittersPoint_Rock (投げ物：岩)
//---------------------------------------------------------------------------
#include "PoittersPoint_Rock.h"
#include "Game/Component/ComponentGrabbable.h"
#include "Game/System/PoittersPoint_PhysicsHelper.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentPhysics.h>

namespace PoittersPoint {

bool Rock::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();

    SetName("Rock");

    // 物理ボディは作成時の位置に置かれるため、先に位置を決めておく
    SetTranslate({10, 10, 10});
    SetScaleAxisXYZ({1.0f});

    auto model = AddComponent<ComponentModel>("data/Game/Models/Rock/rock.mv1");

    // 重力・慣性・摩擦・跳ね・転がり・当たり判定は全て ComponentPhysics(Jolt) が解く
    auto physics = AddComponent<ComponentPhysics>();
    radius_      = PhysicsHelper::CreateSphereFitModel(physics, model, 5.0f, density_);
    physics->SetPhysicsStatus(ComponentPhysics::PhysicsBit::Actived, true);    // 重力有効
    physics->SetPhysicsReal(1.0f);                                             // 物理 100%
    physics->SetCollisionType(ComponentPhysics::CollisionType::ITEM);
    physics->SetFriction(0.5f);        //!< 摩擦（滑りの調整は GUI の「摩擦力」でも可）
    physics->SetRestitution(0.05f);    //!< 跳ね返り係数（石なのでほぼ跳ねない。大きすぎると静止接触でもマイクロ跳ね→上下カクツキの原因）

    // 速度減衰（指数的減衰。0で無効）。Jolt の転がり摩擦は弱く放置だとほぼ永久に転がるため掛ける。
    // 直線減衰は GROUND 接地中のみ適用＝空中の飛距離は殺さず、着地後は素早く止まる。
    physics->SetLinearDamping(2.0f);     //!< 着地後の並進減衰（接地時のみ）
    physics->SetAngularDamping(1.5f);    //!< 転がりそのものへの抵抗（転がりすぎはここを上げる）

    auto grabbable = AddComponent<ComponentGrabbable>();
    grabbable->SetLiftTime(1.0f);
    grabbable->SetDamage(1.0f);
    grabbable->SetKnockbackPower(120.0f);
    grabbable->SetRollSpin(0.5f, radius_);    // 少しだけ前転させて投げる

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

}    // namespace PoittersPoint
