#include "Camera.h"
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentSpringArm.h>

namespace Sample::GameSample
{

	//! カメラ初期化
	bool Camera::Init()
	{
		bool ret = __super::Init();
		if(!ret)
			return false;

		// カメラの設定
		auto camera = AddComponent<ComponentCamera>();
		camera->SetPositionAndTarget({0, 0, -1}, {0, 0, 0});

		// カメラの当たり判定
		auto col = AddComponent<ComponentCollisionSphere>();
		col->SetRadius(2.0f);
		col->SetMass(0.0f);

		// スプリングアーム設定
		auto spring_arm = AddComponent<ComponentSpringArm>();

		spring_arm->SetSpringArmObject(target_obj_.lock());

		spring_arm->SetSpringArmVector({0, 1.2, 2});
		spring_arm->SetSpringArmLength(20);
		spring_arm->SetSpringArmLookOffset({0, 4, 0});

		return true;
	}

	// カメラ作成関数
	CameraPtr Camera::Create(ObjectPtr obj)
	{
		auto camobj			= Scene::Object::CreateDelayInitialize<Camera>();
		camobj->target_obj_ = obj;
		return camobj;
	}

// マウスで動作させるための設定
#ifdef USE_MOUSE_CAMERA
	void Update() override
	{
		auto spring_arm = GetComponent<ComponentSpringArm>();

		// SprintArmの回転にマウスの移動量を足しこむ
		rot_ += {GetMouseMoveY(), GetMouseMoveX(), 0};

		// 足しこんだ回転を利用する
		spring_arm->SetSpringArmRotate(rot_);
	}
#endif

}	 // namespace Sample::GameSample
