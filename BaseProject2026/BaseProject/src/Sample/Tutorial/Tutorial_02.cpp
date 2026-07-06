#include <System/Scene.h>

namespace Tutorial
{
	//-------------------------------------------------------
	//! シーンクラス
	class Tutorial_02 : public Scene::Base
	{
		static constexpr int invalid_model = -1;

	public:
		BP_CLASS_DECL(Tutorial_02, u8"(2)Tutorial シーンの使用方法");

		//! 初期化
		bool Init() override;

		//! 更新
		void Update() override;

		//! 描画
		void Draw() override;

		//! 終了
		void Exit() override;

		//! デバッグ用UI
		void GUI() override;

	private:
		int model_ = invalid_model;	   //!< モデル
	};

	bool Tutorial_02::Init()
	{
		// 仮モデルの読み込み
		model_ = MV1LoadModel("data/Sample/Player/model.mv1");

		// 仮モデルの設定
		MV1SetPosition(model_, {0.0f, 0.0f, 0.0f});
		MV1SetRotationXYZ(model_, {0.0f, 0.0f, 0.0f});
		MV1SetScale(model_, {0.05f, 0.05f, 0.05f});

		// カメラの設定
		SetCameraPositionAndTarget_UpVecY({0.f, 6.f, -15.f}, {0.f, 5.f, 0.f});
		SetupCamera_Perspective(60.0f * DegToRad);

		return true;
	}

	void Tutorial_02::Update()
	{
	}

	void Tutorial_02::Draw()
	{
		// 仮モデルの描画
		MV1DrawModel(model_);
	}

	void Tutorial_02::Exit()
	{
		// モデルの終了
		MV1DeleteModel(model_);
	}

	void Tutorial_02::GUI()
	{
		// HowToUseというGUIWindowを以下の内容で表示する
		ImGui::Begin("HowToUse");
		{
			ImGui::Separator();
			ImGui::Text("シーンの使用方法\n"									  //
						u8"\n"													  //
						u8"実行したい関数をオーバーライドしますが、\n"			  //
						u8"必要で無ければオーバーライドする必要はありません\n"	  //
						u8"\n"													  //
						u8"このシーンクラスでは以下のものをオーバーライドしています\n"
						u8"bool Init() override;\n"
						u8"void Update( float delta ) override;\n"
						u8"void Draw() override;\n"
						u8"void Exit() override;\n"
						u8"void GUI() override;\n"
						u8"\n"
						u8"オーバーライド可能として用意されている関数は以下のものになります\n"				   //
						u8"bool Init()                : 初期化(シーン実行時1回のみ)\n"						   //
						u8"void Update( float delta ) : 更新  (ディスプレイ表示周期で発生します)\n"			   //
						u8"void Draw()                : 描画  (ディスプレイ表示周期で発生します)\n"			   //
						u8"void GUI()                 : この説明( 情報をWindowで表示するためにあります )\n"	   //
						u8"void Exit()                : シーン終了(終了時に1回のみ)\n"
						u8"\n"	  //
						u8"void PreUpdate()           : Updateの前の処理\n"
						u8"void PrePhysics()          : Updateの後、アクション後、物理挙動当たり処理前\n"
						u8"void PostPhysics()         : Updateの後、アクション後、物理挙動当たり判定後\n"
						u8"void PostUpdate()          : Updateの後、アクション後、物理挙動当たり判定後 PreDraw前の処理\n"
						u8"\n"
						u8"void PreDraw()             : 描画前に呼ばれる\n"
						u8"void LateDraw()            : 標準の描画の後の描画するために使用する(上に置く2Dなど)\n"
						u8"void PostDraw()            : Drawの後の処理として利用する\n");
		}
		ImGui::End();
	}

}	 // namespace Tutorial
