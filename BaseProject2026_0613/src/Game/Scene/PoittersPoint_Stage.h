//---------------------------------------------------------------------------
//! @file   PoittersPoint_Stage.h
//! @brief  PoittersPointのステージシーンのヘッダ
//---------------------------------------------------------------------------
#pragma once
#include <System/Scene.h>

namespace PoittersPoint {

class Ground;
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

    //! @brief ステージ外へ落ちた物・キャラクターを中央へ戻す
    void RespawnFallenObjects();

    int enemy_dead_count = 0;

    std::weak_ptr<Ground> ground_;    //!< ステージ(範囲・地面の高さの取得用)
};
// ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑
// ここまでがシーンクラス
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
