//---------------------------------------------------------------------------
//! @file   PoittersPoint_Ground.cpp
//! @brief  PoittersPoint_Ground
//---------------------------------------------------------------------------
#include "PoittersPoint_Ground.h"
#include "Game/Scene/PoittersPoint_Stage.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>

namespace PoittersPoint {
// namespace PoittersPoint

//! @brief 初期化
//! @return 初期化終了
bool Ground::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();
    //__super::Init();

    SetName("Ground");
    AddComponent<ComponentModel>("data/Sample/SwordBout/Stage/Stage00.mv1");
    AddComponent<ComponentCollisionModel>();
    if(auto collision = GetComponent<ComponentCollisionModel>()) {
        // 所属するグループを「GROUND」とします
        collision->SetCollisionGroup(ComponentCollision::CollisionGroup::GROUND);
        collision->AttachToModel();    // コリジョンをモデルに合わせる
    }

    return true;
}

}    // namespace PoittersPoint
