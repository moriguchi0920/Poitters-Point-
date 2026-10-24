//---------------------------------------------------------------------------
//! @file   PoittersPoint_CharacterSelect.cpp
//! @brief  自作チュートリアルシーンXのタイトル
//---------------------------------------------------------------------------
// 自分が一番上
#include "PoittersPoint_CharacterSelect.h"
// 自作系
#include "PoittersPoint_Stage.h"
// システム
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionModel.h>

namespace PoittersPoint {
//! @brief 初期化
//! @return 初期化済み
bool PoittersPoint_CharacterSelect::Init()
{
    // 最初に1回動作する
    // ただし trueを返さなければ Initに何回も来る仕様。

    // create<>(名前、transformがいるか、更新の優先順位、描画の優先順位);
    // カメラ生成
    {
    }

    // 地面生成
    {
    }

    return true;
}

void PoittersPoint_CharacterSelect::Update()
{
    if(Input::IsKeyDown(KEY_INPUT_SPACE))
        Scene::Change(Scene::GetScene<PoittersPoint_Stage>());
}

void PoittersPoint_CharacterSelect::LateDraw()
{
    float ADJUST_X = (WINDOW_W / 1280.0f);
    float ADJUST_Y = (WINDOW_H / 720.0f);
}

void PoittersPoint_CharacterSelect::GUI()
{
}

}    // namespace PoittersPoint
