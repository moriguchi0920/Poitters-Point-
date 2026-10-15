#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint.h
//! @brief  PoittersPointのタイトルのヘッダファイル
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace PoittersPoint {
//-----------------------------------------------------------------------
// BPでは
// Sceneクラスを作成する必要がある( Scene::Baseから継承する )
// ●何も表示しないシーン( Tutorial_01 )を作成しています
//
// Game.ini を以下の設定にすると初期で実行されます
// ; シーン
// [Scene]
// ; 初期に読み込むシーン
// Start = Tutorial_01
//-----------------------------------------------------------------------
// ここから
// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
//! シーンクラス
class PoittersPoint_CharacterSelect : public Scene::Base
{
public:
    //publicでこの記述をクラスに入れておけばGUIでオブジェクト生成が可能になる
    BP_CLASS_DECL(PoittersPoint_CharacterSelect, u8"(Title)ポイッターズポイント キャラクターセレクトシーン");

    //! @brief 初期化
    //! @return 初期化済み
    bool Init() override;

    void Update() override;

    void Draw() override;

    void LateDraw() override;

    void GUI() override;

private:
    enum class SelectState
    {
        SelectCharacter,     // キャラ選択中
        FocusDecideButton    // 決定ボタン選択中
    };

    SelectState state_ = SelectState::SelectCharacter;    // 現在の状態

    int selected_character_id_ = 0;    // 確定したキャラID

    int select_idx_       = 0;     // 赤い枠
    int Background_image_ = -1;    //背景

    int se_cursor_ = -1;    // キャラ選択SE
    int se_decide_ = -1;    // 決定SE
};

}    // namespace PoittersPoint
