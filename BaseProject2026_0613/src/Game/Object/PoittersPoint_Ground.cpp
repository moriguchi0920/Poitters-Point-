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
    //元のグラウンド
    //AddComponent<ComponentModel>("data/Sample/SwordBout/Stage/Stage00.mv1");
    
    //ステージ01の試し描画
    //auto model = AddComponent<ComponentModel>("data/Sample/Stage01/FantasyTown.mv1");
    //if(model) {
    //    model->SetScaleAxisXYZ({4.0f, 4.0f, 4.0f});
    //}

    //ステージ02の試し描画
    auto model = AddComponent<ComponentModel>("data/Sample/Stage02/02_Ruin_Main.mv1");
    if(model) {
        model->SetScaleAxisXYZ({1.9f, 1.9f, 1.9f});
    }


    AddComponent<ComponentCollisionModel>();
    if(auto collision = GetComponent<ComponentCollisionModel>()) {
        // 所属するグループを「GROUND」とします
        collision->SetCollisionGroup(ComponentCollision::CollisionGroup::GROUND);
        collision->AttachToModel();    // コリジョンをモデルに合わせる
    }

    return true;
}

}    // namespace PoittersPoint
