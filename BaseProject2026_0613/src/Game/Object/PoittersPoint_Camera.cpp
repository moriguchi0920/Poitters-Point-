//---------------------------------------------------------------------------
//! @file   PoittersPoint_Camera.cpp
//! @brief  PoittersPoint_Camera
//---------------------------------------------------------------------------
#include "PoittersPoint_Camera.h"
#include "Game/Scene/PoittersPoint_Stage.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>

namespace PoittersPoint {
// namespace PoittersPoint

//! @brief 初期化
//! @return 初期化終了
bool Camera::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();
    //__super::Init();

    SetName("Camera");

    AddComponent<ComponentCamera>();    //カメラコンポーネントを付ける
    if(auto c = GetComponent<ComponentCamera>()) {
        c->SetPositionAndTarget({0, 70, 120}, {0, 0, -60});
    }

    //// プレイヤーがいるなら追従カメラにする
    //if(Scene::Object::Get<Object>("Player")) {
    //    AddComponent<ComponentSpringArm>();
    //    if(auto c = GetComponent<ComponentSpringArm>()) {
    //        c->SetSpringArmObject("Player");
    //    }
    //}

    return true;
}

}    // namespace PoittersPoint
