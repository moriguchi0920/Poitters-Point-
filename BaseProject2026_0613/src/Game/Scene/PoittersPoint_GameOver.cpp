//---------------------------------------------------------------------------
//! @file   PoittersPoint_GameOver.cpp
//! @brief  PoittersPointのゲームオーバー
//---------------------------------------------------------------------------
// 自分が一番上
#include "PoittersPoint_GameOver.h"
// 自作系
#include "PoittersPoint_Stage.h"
#include "PoittersPoint_Title.h"
// システム
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionModel.h>

namespace PoittersPoint {
//! @brief 初期化
//! @return 初期化済み
bool PoittersPoint_GameOver::Init()
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
        ground->AddComponent<ComponentCollisionModel>();
        if(auto collision = ground->GetComponent<ComponentCollisionModel>()) {
            // 所属するグループを「GROUND」とします
            collision->SetCollisionGroup(ComponentCollision::CollisionGroup::GROUND);
            collision->AttachToModel();    // コリジョンをモデルに合わせる
        }
    }

    return true;
}

void PoittersPoint_GameOver::Update()
{
    if(3.0f <= GetSceneTime()) {
        Scene::Change(Scene::GetScene<PoittersPoint_Title>());
    }
}

void PoittersPoint_GameOver::LateDraw()
{
    float ADJUST_X = (WINDOW_W / 1280.0f);
    float ADJUST_Y = (WINDOW_H / 720.0f);

    SetFontSize(static_cast<int>(gameover_size_ * ADJUST_X));
    DrawFormatString(static_cast<int>(gameover_x_ * ADJUST_X), static_cast<int>(gameover_y_ * ADJUST_Y), GetColor(255, 255, 255), "GameOver");
}

void PoittersPoint_GameOver::GUI()
{
    ImGui::DragInt("GameOverLeft", &gameover_x_);
    ImGui::DragInt("GameOverTop", &gameover_y_);
    ImGui::DragInt("GameOverSize", &gameover_size_);
}

}    // namespace PoittersPoint
