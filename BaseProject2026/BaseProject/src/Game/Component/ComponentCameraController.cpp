#pragma once
#include <Game/Component/ComponentCameraController.h>

void ComponentCameraController::Init()
{
	__super::Init();
}

void ComponentCameraController::Update()
{
	__super::Update();

	// オーナー(自分がAddComponentされたObject)を取得します
	// 処理されるときは必ずOwnerは存在しますので基本的にnullptrチェックは必要ありません
	auto owner = GetOwner();

	// カメラが存在している場合
	if(auto camera = Scene::GetCurrentCamera().lock())
	{
		// SpringArmのオブジェクトが自分の場合のみ
		auto cam_owner = camera->GetOwner();
		if(auto arm = cam_owner->GetComponent<ComponentSpringArm>())
		{
			// そのターゲットはオーナーのオブジェクトであることが条件となる
			auto obj = arm->GetSpringArmObject().lock();
			if(obj.get() == owner)
			{
				// オブジェクトコントローラーにターゲットがいる場合
				if(auto target = target_.lock())
				{
					// 徐々に敵の方に向ける(1フレーム最大3度)
					owner->SetRotationToPositionWithLimit(target->GetTranslate(), target_cam_side_speed_);

					// カメラローテーションをロック方向にしておくと
					// 戻った時に違和感がない
					cam_ry_ = owner->GetRotationAxisXYZ().y;
					cam_rx_ = (cam_rx_ + target_cam_up_down) * 0.95f - target_cam_up_down;

					arm->SetSpringArmRotate({cam_rx_, 0, 0});
				}
				else
				{
					// ターゲットがない場合
					// マウスで動作せる場合の処理
					float mouse_ud = mouse_up_down_ * GetDeltaTime60();
					float mouse_lr = mouse_left_right_ * GetDeltaTime60();
					float cam_spd  = cam_speed_ * GetDeltaTime60();
					if(use_mouse_)
					{
						cam_rx_ += GetMouseMoveY() * (mouse_ud / 100.0f);
						if(cam_rx_ > limit_cam_up_)
							cam_rx_ = limit_cam_up_;
						if(cam_rx_ < limit_cam_down_)
							cam_rx_ = limit_cam_down_;
						cam_ry_ += GetMouseMoveX() * (mouse_lr / 100.0f);
						if(cam_ry_ > 360.0f)
							cam_ry_ -= 360.0f;
						if(cam_ry_ < -360.0f)
							cam_ry_ += 360.0f;
					}
					// キーでカメラを移動させる
					if(IsKey(cam_up_))
					{
						cam_rx_ += cam_spd;
						if(cam_rx_ > limit_cam_up_)
							cam_rx_ = limit_cam_up_;
					}
					if(IsKey(cam_down_))
					{
						cam_rx_ -= cam_spd;
						if(cam_rx_ < limit_cam_down_)
							cam_rx_ = limit_cam_down_;
					}
					if(IsKey(cam_right_))
					{
						cam_ry_ += cam_spd;
						if(cam_ry_ > 360.0f)
							cam_ry_ -= 360.0f;
					}
					if(IsKey(cam_left_))
					{
						cam_ry_ -= cam_spd;
						if(cam_ry_ < -360.0f)
							cam_ry_ += 360.0f;
					}

					// Y軸はオブジェクトそのものの向きを使う
					owner->SetRotationAxisXYZ({0, cam_ry_, 0});
					// 上下はSpringArmで制御しておく
					arm->SetSpringArmRotate({cam_rx_, 0, 0});
				}
			}
		}
	}
}

ComponentCameraControllerPtr ComponentCameraController::SetCameraKeys(int up, int down, int left, int right)
{
	// カメラキーの設定
	cam_up_	   = up;
	cam_down_  = down;
	cam_left_  = left;
	cam_right_ = right;
	return std::dynamic_pointer_cast<ComponentCameraController>(shared_from_this());
}

ComponentCameraControllerPtr ComponentCameraController::SetTarget(ObjectPtr target)
{
	target_ = target;
	return std::dynamic_pointer_cast<ComponentCameraController>(shared_from_this());
}

ComponentCameraControllerPtr ComponentCameraController::SetCameraRotate(float3 rot)
{
	cam_rx_ = rot.x;
	cam_ry_ = rot.y;
	return std::dynamic_pointer_cast<ComponentCameraController>(shared_from_this());
}

ComponentCameraControllerPtr ComponentCameraController::SetUseMouse(bool use)
{
	use_mouse_ = use;
	return std::dynamic_pointer_cast<ComponentCameraController>(shared_from_this());
}

bool ComponentCameraController::IsUseMouse() const
{
	return use_mouse_;
}

void ComponentCameraController::GUI()
{
	__super::GUI();

	// GUI内に出現させる
	ImGui::Begin(GetOwner()->GetName().data());
	{
		ImGui::Separator();
		if(ImGui::TreeNode("Camera Controller"))
		{
			// 有効/無効
			bool enable = GetStatus(StatusBit::Enable);
			if(ImGui::Checkbox(u8"有効", &enable))
				SetStatus(StatusBit::Enable, enable);

			// GUI上でオーナーから自分を削除します
			if(ImGui::Button(u8"削除"))
				GetOwner()->RemoveComponent(shared_from_this());

			// カメラの基本情報
			ImGui::Checkbox(u8"カメラ方向にMouseを使用", &use_mouse_);
			ImGui::DragFloat(u8"カメラ左右回転スピード", &mouse_left_right_);
			ImGui::DragFloat(u8"カメラ上下回転スピード", &mouse_up_down_);

			ImGui::DragFloat(u8"見る方向上リミット", &limit_cam_up_);
			ImGui::DragFloat(u8"見る方向下リミット", &limit_cam_down_);

			ImGui::TreePop();
		}
	}
	ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentCameraController)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentCameraController)
