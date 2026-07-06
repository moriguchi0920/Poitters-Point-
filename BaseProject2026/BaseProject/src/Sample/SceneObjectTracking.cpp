#include "SceneObjectTracking.h"
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentTargetTracking.h>
#include <System/Component/ComponentCollisionSphere.h>

//! @brief シーン初期化関数を継承します
//! @return シーン初期化が終わったらtrueを返します
bool SceneObjectTracking::Init()
{
	auto obj = Scene::Object::Create<Object>()->SetName("Object");
	obj->SetTranslate({0, 10, -10});
	obj->AddComponent<ComponentCollisionSphere>();
	obj->AddComponent<ComponentModel>("data/Sample/Coin/GoldCoin.mv1");
	obj->SetScaleAxisXYZ({0.1f, 0.1f, 0.1f});

	// カメラ
	Scene::Object::Create<Object>()							 // カメラオブジェクト
		->SetName("Camera")									 // 名前設定
		->AddComponent<ComponentCamera>()					 // カメラコンポーネント
		->SetPositionAndTarget({0, 35, -80}, {0, 20, 0});	 // ポジションと注視点

	auto mouse = Scene::Object::Create<Object>()->SetName("Mouse");

	if(auto model = mouse->AddComponent<ComponentModel>())
	{
		model->Load("data/Sample/Player/model.mv1");
		model->SetAnimation({
			{"walk", "data/Sample/Player/Anim/Walk.mv1", 1, 1.0f}, // idle
			{"jump", "data/Sample/Player/Anim/Jump.mv1", 1, 1.0f}  // jump
		});
		model->PlayAnimation("walk", true);
	}
#if 1
	if(auto tracking = mouse->AddComponent<ComponentTargetTracking>())
	{
		tracking->SetTargetObjectPtr(obj);
		tracking->SetTrackingNode("mixamorig:Neck");
	}
#endif

	return true;
}

void SceneObjectTracking::Draw()
{
	DrawFormatString(100, 100, GetColor(255, 255, 255), "コインをネズミが常にみます。");
	DrawFormatString(100, 130, GetColor(255, 255, 255), "コインとネズミを動かして確認してみましょう。");
}

void SceneObjectTracking::Exit()
{
}
