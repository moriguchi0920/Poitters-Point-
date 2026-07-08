//---------------------------------------------------------------------------
//! @file   PoittersPoint_Stage.h
//! @brief  PoittersPointのステージシーンのヘッダ
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
class PoittersPoint_Stage : public Scene::Base
{
    static constexpr int MAX_ENEMIES = 5;

public:
    //publicでこの記述をクラスに入れておけばGUIでオブジェクト生成が可能になる
    BP_CLASS_DECL(PoittersPoint_Stage, u8"(stage)ポイッターズポイント ステージシーン");

    //! @brief 初期化
    //! @return 初期化済み
    bool Init() override;

    //! @brief 更新
    void Update() override;

    //! @brief GUI表示
    void GUI() override;

    void AddDeadEnemy();

private:
    void createEnemy();

    int enemy_dead_count = 0;

    // GUIで表示している(int)
    int counter = 0;

    // テスト用変数
    int counter2 = 1000;
    int counter3 = 2000;
};
// ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑
// ここまでがシーンクラス
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// 問題1
// Tutorial_01は、.h/.cppに分けることができない。
// .h/.cppに分けることができるように記述してみましょう
// 「Tutorial_01B」をこの下に作成してください
//-----------------------------------------------------------------------
// ここから
// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

// ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑
// ここまでに記述すること
//-----------------------------------------------------------------------

}    // namespace PoittersPoint

//! @mainpage チュートリアル
//!
//! @ref tutorial01
//!

//! @subpage tutorial01 チュートリアル01
//!
//! class を ヘッダと実行メソッドに分けて書きましょう
//!
//! ヘッダ部分は以下のようになります
//!
//! @code
//!	class Tutorial_01 : public Scene::Base
//!	{
//!	public:
//!		BP_CLASS_DECL(Tutorial_01, u8"(1)Tutorial カラのシーン");
//!
//!		//! @brief 初期化
//!		//! @return 初期化済み
//!		bool Init() override;
//!
//!		//! @brief 更新
//!		void Update() override;
//!
//!		//! @brief GUI表示
//!		void GUI() override;
//!
//!	private:
//!		// GUIで表示している(int)
//!		int counter = 0;
//!	};
//! @endcode
