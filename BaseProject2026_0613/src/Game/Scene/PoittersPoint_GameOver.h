//---------------------------------------------------------------------------
//! @file   PoittersPoint_GameOver.h
//! @brief  PoittersPointのゲームオーバーのヘッダファイル
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
class PoittersPoint_GameOver : public Scene::Base
{
public:
    //publicでこの記述をクラスに入れておけばGUIでオブジェクト生成が可能になる
    BP_CLASS_DECL(PoittersPoint_GameOver, u8"(gameOver)ポイッターズポイント ゲームオーバーシーン");

    //! @brief 初期化
    //! @return 初期化済み
    bool Init() override;

    void Update() override;

    void LateDraw() override;

    void GUI() override;

private:
    int gameover_x_    = 20;
    int gameover_y_    = 100;
    int gameover_size_ = 300;
};

}    // namespace PoittersPoint
