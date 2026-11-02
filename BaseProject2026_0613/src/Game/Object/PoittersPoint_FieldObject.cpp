//---------------------------------------------------------------------------
//! @file   PoittersPoint_FieldObject.cpp
//! @brief  PoittersPoint_FieldObject
//---------------------------------------------------------------------------
#include "PoittersPoint_FieldObject.h"
#include "Game/Scene/PoittersPoint_Stage.h"

#include <System/Scene.h>

namespace PoittersPoint {
// namespace PoittersPoint

//! @brief 初期化
//! @return 初期化終了
bool FieldObject::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();

    return true;
}

}    // namespace PoittersPoint
