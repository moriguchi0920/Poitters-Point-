//---------------------------------------------------------------------------
//! @file   PoittersPoint_Clear.cpp
//! @brief  PoittersPointのクリアシーン
//---------------------------------------------------------------------------
// 自分が一番上
#include "PoittersPoint_Clear.h"
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
bool PoittersPoint_Clear::Init()
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

void PoittersPoint_Clear::Update()
{
    if(3.0f <= GetSceneTime()) {
        Scene::Change(Scene::GetScene<PoittersPoint_Title>());
    }
}

void PoittersPoint_Clear::LateDraw()
{
    float ADJUST_X = (WINDOW_W / 1280.0f);
    float ADJUST_Y = (WINDOW_H / 720.0f);

    SetFontSize(static_cast<int>(clear_size_ * ADJUST_X));
    DrawFormatString(static_cast<int>(clear_x_ * ADJUST_X), static_cast<int>(clear_y_ * ADJUST_Y), GetColor(255, 255, 255), "GameOver");
}

void PoittersPoint_Clear::GUI()
{
    ImGui::DragInt("GameOverLeft", &clear_x_);
    ImGui::DragInt("GameOverTop", &clear_y_);
    ImGui::DragInt("GameOverSize", &clear_size_);
}

}    // namespace PoittersPoint
