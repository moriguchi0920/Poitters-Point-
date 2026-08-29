//---------------------------------------------------------------------------
//! @file   PoittersPoint_Title.cpp
//! @brief  自作チュートリアルシーンXのタイトル
//---------------------------------------------------------------------------
// 自分が一番上
#include "PoittersPoint_Title.h"
// 自作系
#include "PoittersPoint_Stage.h"
// システム系
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionModel.h>

namespace PoittersPoint {
//! @brief 初期化
//! @return 初期化済み
bool PoittersPoint_Title::Init()
{
    // 最初に1回動作する
    // ただし trueを返さなければ Initに何回も来る仕様。

    // create<>(名前、transformがいるか、更新の優先順位、描画の優先順位);
    // カメラ生成
    {
    }

    // 地面生成
    {
        auto ground = Scene::Object::Create<Object>("Ground");
        ground->AddComponent<ComponentModel>("data/Sample/SwordBout/Stage/Stage00.mv1");
        ground
            ->AddComponent<ComponentCollisionModel>()
            // 所属するグループを「GROUND」とします
            ->SetCollisionGroup(ComponentCollision::CollisionGroup::GROUND)
            ->AttachToModel();    // コリジョンをモデルに合わせる
    }

    return true;
}

void PoittersPoint_Title::Update()
{
    if(Input::IsKeyDown(KEY_INPUT_SPACE))
        Scene::Change(Scene::GetScene<PoittersPoint_Stage>());
}

void PoittersPoint_Title::LateDraw()
{
    DrawBoxAA(0, 0, WINDOW_W, WINDOW_H, GetColor(245, 240, 230), TRUE);
    DrawBoxAA(0, 150, WINDOW_W, 450, GetColor(240, 240, 200), TRUE);

    float ADJUST_X = (WINDOW_W / 1280.0f);
    float ADJUST_Y = (WINDOW_H / 720.0f);

    SetFontSize(static_cast<int>(title_size_ * ADJUST_X));
    DrawFormatString(static_cast<int>(title_x_ * ADJUST_X), static_cast<int>(title_y_ * ADJUST_Y), GetColor(255, 20, 80), "タイトル名");

    unsigned int button_color = ((GetNowCount() / 150) % 2 == 0) ? GetColor(150, 255, 150)     // 緑
                                                                 : GetColor(255, 255, 140);    // 黄
    SetFontSize(static_cast<int>(button_size_ * ADJUST_X));
    DrawFormatString(static_cast<int>(button_x_ * ADJUST_X), static_cast<int>(button_y_ * ADJUST_Y), button_color, "PRESS START!!");
}

void PoittersPoint_Title::GUI()
{
    ImGui::DragInt("TitleLeft", &title_x_);
    ImGui::DragInt("TitleTop", &title_y_);
    ImGui::DragInt("TitleSize", &title_size_);
}

}    // namespace PoittersPoint
