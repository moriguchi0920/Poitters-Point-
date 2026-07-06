#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>

USING_PTR(ComponentCameraController);

class ComponentCameraController : public Component
{
public:
	BP_COMPONENT_DECL(ComponentCameraController, u8"カメラコントローラー");

	void Init() override;

	void Update() override;

	ComponentCameraControllerPtr SetCameraKeys(int up, int down, int left, int right);

	ComponentCameraControllerPtr SetTarget(ObjectPtr target);

	ComponentCameraControllerPtr SetCameraRotate(float3 rot);

	ComponentCameraControllerPtr SetUseMouse(bool use = true);

	bool IsUseMouse() const;

	void GUI() override;

private:
	float cam_speed_ = 1.0f;
	int	  cam_up_	 = KEY_INPUT_UP;
	int	  cam_down_	 = KEY_INPUT_DOWN;
	int	  cam_left_	 = KEY_INPUT_LEFT;
	int	  cam_right_ = KEY_INPUT_RIGHT;

	float cam_rx_ = 0;
	float cam_ry_ = 0;

	bool  use_mouse_		= false;
	float mouse_up_down_	= 10.0f;
	float mouse_left_right_ = 10.0f;

	float limit_cam_up_	  = 5.0f;	   //!< 上を眺める
	float limit_cam_down_ = -45.0f;	   //!< 下を見る

	ObjectWeakPtr target_;
	float		  target_cam_side_speed_ = 3.0f;	 //!< ロックオン時のカーソル移動速度
	float		  target_cam_up_down	 = 10.0f;	 //!< ターゲットを見る際の上下固定

	//--------------------------------------------------------------------
	//! @name Cereal処理
	//--------------------------------------------------------------------
	//@{

	//! @brief セーブ
	// @param arc アーカイバ
	// @param ver バージョン
	CEREAL_SAVELOAD(arc, ver)
	{
		arc(CEREAL_NVP(cam_speed_),
			CEREAL_NVP(cam_up_),
			CEREAL_NVP(cam_down_),
			CEREAL_NVP(cam_left_),
			CEREAL_NVP(cam_right_),

			CEREAL_NVP(cam_rx_),
			CEREAL_NVP(cam_ry_),

			CEREAL_NVP(limit_cam_up_),
			CEREAL_NVP(limit_cam_down_),

			CEREAL_NVP(target_),
			CEREAL_NVP(target_cam_side_speed_),
			CEREAL_NVP(target_cam_up_down),

			CEREAL_NVP(use_mouse_),
			CEREAL_NVP(mouse_up_down_),
			CEREAL_NVP(mouse_left_right_));

		arc(cereal::make_nvp("Component", cereal::base_class<Component>(this)));
	}
};

CEREAL_CLASS_VERSION(ComponentCameraController, 1);
