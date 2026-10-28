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
class PoittersPoint_Title : public Scene::Base
{
public:
    //publicでこの記述をクラスに入れておけばGUIでオブジェクト生成が可能になる
    BP_CLASS_DECL(PoittersPoint_Title, u8"(Title)ポイッターズポイント タイトルシーン");

    //! @brief 初期化
    //! @return 初期化済み
    bool Init() override;

    void Update() override;

    void LateDraw() override;

    void GUI() override;

private:
    int title_x_    = 100;
    int title_y_    = 170;
    int title_size_ = 170;

    int button_x_    = 500;
    int button_y_    = 560;
    int button_size_ = 50;
};

}    // namespace PoittersPoint
