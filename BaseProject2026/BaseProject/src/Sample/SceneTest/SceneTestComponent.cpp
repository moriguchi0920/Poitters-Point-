#include "SceneTestComponent.h"
#include <System/Component/ComponentModel.h>

//自分でテストコンポーネントを作成する
class ComponentTest : public Component
{
public:
	BP_COMPONENT_DECL(ComponentTest, u8"コンポーネントTest用機能クラス");
	void Init()
	{
		Super::Init();
		// このコンポーネントは、自分のオーナーにモデルをつけます。

		auto model = GetOwner()->AddComponent<ComponentModel>();
		model->Load("data/Sample/Player/model.mv1");
		model->SetAnimation({
			{"idle", "data/Sample/Player/Anim/Idle.mv1", 1, 1.0f}, // idle
			{"jump", "data/Sample/Player/Anim/Jump.mv1", 1, 1.0f}  // jump
		});
	}

	void Update()
	{
		Super::Update();
		// 常に X方向に 0.1f 進め、回転します。
		auto owner = GetOwner();
		owner->AddTranslate({0.1f, 0, 0});

		auto new_rotate = owner->GetRotationAxisXYZ() + float3{0, 1, 0};
		owner->SetRotationAxisXYZ(new_rotate);

		// スペースを押すとジャンプする
		if(IsKeyOn(KEY_INPUT_SPACE))
		{
			// モデルのコンポーネントを取得してアクションをジャンプにする
			if(auto model = owner->GetComponent<ComponentModel>())
				model->PlayAnimation("jump");
		}

		// ENTERを押すと位置を0,0,0にする
		if(IsKeyOn(KEY_INPUT_RETURN))
		{
			owner->SetTranslate({0, 0, 0});
		}
	}
};

//! @brief シーン初期化関数を継承します
//! @return シーン初期化が終わったらtrueを返します
bool SceneTestComponent::Init()
{
	Super::Init();
	// Componentとは、個別の特殊な単体の能力持っている構造です。
	// Component単体では生成することはできません。あくまでObjectの機能として動作します
	// 作成すると自動的に以下の関数が呼ばれます
	// Init()   <= 最初に一度呼ばれます
	// Update() <= ディスプレイリフレッシュレートに合わせて呼び出されます  例 60Hz : 1秒間に60回呼び出されます
	// Draw()   <= Updateの後に呼び出されます。描画するのに適した関数です
	// Exit()   <= 終了時に呼ばれる関数です

	// Mouseという名前のオブジェクトを作成します
	auto obj = Scene::Object::Create<Object>("Mouse");
	obj->AddComponent<ComponentTest>();

	Scene::Object::Create<Object>("Camera")	   //
		->AddComponent<ComponentCamera>()	   //
		->SetPositionAndTarget({0.0f, 5.0f, -100.0f}, {0, 0, 0});

	return true;
}

//! @brief シーン更新関数。ディスプレイリフレッシュレートに合わせて実行されます
//! @param delta 1秒をベースとした1フレームの数値
//! @detial deltaは、リフレッシュレートが違うと速度が変わってしまう部分を吸収するためにある
void SceneTestComponent::Update()
{
	Super::Update();
}
