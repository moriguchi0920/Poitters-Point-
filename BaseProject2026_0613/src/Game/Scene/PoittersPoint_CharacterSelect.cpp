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

#include <Game/Object/PoittersPoint_Player.h>
#include <Game/Object/PoittersPoint_Camera.h>

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

    select_idx_       = 0;
    Background_image_ = LoadGraph("data/Game/Image/c_b_.png");    //　背景画像

    se_cursor_ = LoadSoundMem("data/Game/SE/CharacterSelect/Cursor.mp3");      // キャラ選択SE
    se_decide_ = LoadSoundMem("data/Game/SE/CharacterSelect/Decision.mp3");    // キャラ選択SE

    // 3Dカメラ生成
    auto camera = Scene::Object::Create<Camera>();
    if(auto c = camera->GetComponent<ComponentCamera>()) {
        c->SetPositionAndTarget({0.0f, 0.0f, -30.0f}, {0.0f, 0.0f, 0.0f});
    }

    // プレイヤー生成
    auto player = Scene::Object::Create<Object>("Player");
    player->SetTranslate({-15.0f, -7.0f, 0.0f});         // 画面中央に配置
    player->SetRotationAxisXYZ({0.0f, -20.0f, 0.0f});    // 手前(カメラ方向)を向かせる
    player->AddComponent<ComponentModel>("data/Game/Models/Player/Player.mv1");
    if(auto model = player->GetComponent<ComponentModel>()) {
        model->SetAnimation({
            {"idle", "data/Game/Models/Player/Anims/Idle.mv1", 1, 1.0f},
        });
        model->SetScaleAxisXYZ({0.09f, 0.09f, 0.09f});

        model->PlayAnimation("idle", true);
    }

    return true;
}

void PoittersPoint_CharacterSelect::Update()
{
    // キャラクター選択中
    if(state_ == SelectState::SelectCharacter) {
        // 左移動
        if(Input::IsKeyDown(KEY_INPUT_LEFT) || Input::IsKeyDown(KEY_INPUT_A)) {
            select_idx_ = (select_idx_ + 3) % 4;    // 0~3のループ
            StopSoundMem(se_cursor_);
            PlaySoundMem(se_cursor_, DX_PLAYTYPE_BACK);
        }
        // 右移動
        if(Input::IsKeyDown(KEY_INPUT_RIGHT) || Input::IsKeyDown(KEY_INPUT_D)) {
            select_idx_ = (select_idx_ + 1) % 4;    // 0~3のループ
            StopSoundMem(se_cursor_);
            PlaySoundMem(se_cursor_, DX_PLAYTYPE_BACK);
        }
        // スペースキーまたは下キーで「決定ボタン」にフォーカスを移す
        if(Input::IsKeyDown(KEY_INPUT_SPACE) || Input::IsKeyDown(KEY_INPUT_DOWN) || Input::IsKeyDown(KEY_INPUT_S)) {
            state_ = SelectState::FocusDecideButton;
            StopSoundMem(se_cursor_);
            PlaySoundMem(se_cursor_, DX_PLAYTYPE_BACK);
        }
    }

    // 決定ボタンフォーカス中
    else if(state_ == SelectState::FocusDecideButton) {
        // 上キーでキャラ選択に戻る
        if(Input::IsKeyDown(KEY_INPUT_UP) || Input::IsKeyDown(KEY_INPUT_W)) {
            state_ = SelectState::SelectCharacter;
            StopSoundMem(se_cursor_);
            PlaySoundMem(se_cursor_, DX_PLAYTYPE_BACK);
        }
        // もう一度スペースキーを押してステージへ遷移！
        if(Input::IsKeyDown(KEY_INPUT_SPACE) || Input::IsKeyDown(KEY_INPUT_RETURN)) {
            selected_character_id_ = select_idx_;    // 選んだIDを保存
            // 決定音があれば鳴らす
            if(se_decide_ != -1)
                PlaySoundMem(se_decide_, DX_PLAYTYPE_BACK);

            Scene::Change(Scene::GetScene<PoittersPoint_Stage>());
        }
    }
}

void PoittersPoint_CharacterSelect::Draw()
{
    // 1. 全画面背景画像
    if(Background_image_ != -1) {
        DrawExtendGraph(0, 0, WINDOW_W, WINDOW_H, Background_image_, TRUE);
    }

    // 2. キャラクター枠の配置計算
    const int   CHAR_COUNT   = 4;
    const float CARD_WIDTH   = 270.0f;
    const float CARD_HEIGHT  = 480.0f;
    const float CARD_SPACING = 20.0f;
    const float START_Y      = 80.0f;

    float total_width = (CARD_WIDTH * CHAR_COUNT) + (CARD_SPACING * (CHAR_COUNT - 1));
    float start_x     = (WINDOW_W - total_width) / 2.0f;

    // 灰色の枠背景のみを3Dモデルの後ろに描画
    for(int i = 0; i < CHAR_COUNT; ++i) {
        float left   = start_x + i * (CARD_WIDTH + CARD_SPACING);
        float top    = START_Y;
        float right  = left + CARD_WIDTH;
        float bottom = top + CARD_HEIGHT;

        // 枠内背景（暗いグレー）
        DrawBoxAA(left, top, right, bottom, GetColor(40, 40, 40), TRUE);
        // 枠線
        DrawBoxAA(left, top, right, bottom, GetColor(100, 100, 100), FALSE);

        // 赤い枠
        if(i == select_idx_) {
            for(int t = 0; t < 3; ++t) {
                DrawBoxAA(left - t, top - t, right + t, bottom + t, GetColor(255, 0, 0), FALSE);
            }
            //デバッグ
            // DrawFormatStringF(left, top, GetColor(255, 255, 255), "Slot %d", i + 1);
        }
    }
}

void PoittersPoint_CharacterSelect::LateDraw()
{
    // --- 決定ボタンの位置・サイズ設定 ---
    float btn_w = 200.0f;
    float btn_h = 60.0f;
    float btn_x = (WINDOW_W - btn_w) / 2.0f;
    float btn_y = WINDOW_H - 100.0f;

    // 1. ボタン背景描画
    unsigned int bg_color     = GetColor(60, 60, 60);
    unsigned int border_color = GetColor(150, 150, 150);

    // 決定ボタンが選択されている時は黄色/明るくハイライト
    if(state_ == SelectState::FocusDecideButton) {
        bg_color     = GetColor(200, 160, 0);
        border_color = GetColor(255, 255, 0);
    }

    DrawBoxAA(btn_x, btn_y, btn_x + btn_w, btn_y + btn_h, bg_color, TRUE);
    DrawBoxAA(btn_x, btn_y, btn_x + btn_w, btn_y + btn_h, border_color, FALSE);

    // 2. 「決定」の文字描画
    unsigned int text_color = GetColor(255, 255, 255);
    DrawStringF(btn_x + 75.0f, btn_y + 20.0f, "決 定", text_color);

    DrawStringF(50 + 50.0f, 20 + 20.0f, "左右キーで選択、スペースキーで決定", GetColor(255, 255, 255));
}

void PoittersPoint_CharacterSelect::GUI()
{
}

}    // namespace PoittersPoint
