#include <System/Scene.h>

namespace Tutorial {
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
class Tutorial_01 : public Scene::Base
{
public:
    BP_CLASS_DECL(Tutorial_01, u8"(1)Tutorial カラのシーン");

    //! @brief 初期化
    //! @return 初期化済み
    bool Init() override
    {
        // 最初に1回動作する
        // ただし trueを返さなければ Initに何回も来る仕様。
        return true;
    }

    //! @brief 更新
    void Update() override
    {
        // 毎フレーム動作する
        counter++;
    }

    //! @brief GUI表示
    void GUI() override
    {
        // 入力(Int)で、privateにある counter を指定
        ImGui::InputInt("Counter", &counter);
    }

private:
    // GUIで表示している(int)
    int counter = 0;
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

}    // namespace Tutorial

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
