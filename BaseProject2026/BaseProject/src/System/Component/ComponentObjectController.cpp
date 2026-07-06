#pragma once
#include <System/Component/ComponentObjectController.h>
#include <System/Component/ComponentSpringArm.h>
#include <System/Component/ComponentModel.h>

void ComponentObjectController::Init()
{
	__super::Init();
}

void ComponentObjectController::Update()
{
	__super::Update();

	// オーナー(自分がAddComponentされたObject)を取得します
	// 処理されるときは必ずOwnerは存在しますので基本的にnullptrチェックは必要ありません
	auto owner = GetOwner();

	// 移動方向
	float3 dir{0, 0, 0};
	if(IsKey(key_up_))
		dir += {0, 0, -1};

	if(IsKey(key_down_))
		dir += {0, 0, 1};

	if(IsKey(key_right_))
		dir += {-1, 0, 0};

	if(IsKey(key_left_))
		dir += {1, 0, 0};

	// 移動キーが押されているか?
	if((float)length(dir) > 0.0f)
	{
		// 斜めが押されていることを考慮し、
		// その方向の移動スピードを1とし、スピードを掛け合わせる
		dir = normalize(dir);

		// キャラのローカル方向で移動をさせる
		owner->AddTranslate(dir * move_speed_, true);

		// モデルを移動の方向に向けます
		if(auto mdl = owner->GetComponent<ComponentModel>())
		{
			auto rot = quaternion::rotation_axis({0, 1, 0}, front_rot_ * DegToRad);	   //< Y軸1度回転

			mdl->SetRotationToVectorWithLimit(mul(dir, rot), rot_speed_);
			mdl->PlayAnimationNoSame("walk", true);
		}
	}
	else
	{
		// モデルを移動の方向に向けます
		if(auto mdl = owner->GetComponent<ComponentModel>())
			mdl->PlayAnimationNoSame("idle", true);
	}

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

ComponentObjectControllerPtr ComponentObjectController::SetMoveSpeed(const float speed)
{
	move_speed_ = speed;
	return std::dynamic_pointer_cast<ComponentObjectController>(shared_from_this());
}
ComponentObjectControllerPtr ComponentObjectController::SetRotateSpeed(const float speed)
{
	rot_speed_ = speed;
	return std::dynamic_pointer_cast<ComponentObjectController>(shared_from_this());
}

const float ComponentObjectController::GetMoveSpeed() const
{
	return move_speed_;
}

const float ComponentObjectController::GetRotateSpeed() const
{
	return rot_speed_;
}

ComponentObjectControllerPtr ComponentObjectController::SetKeys(int up, int down, int left, int right)
{
	// 移動キーの設定
	key_up_	   = up;
	key_down_  = down;
	key_left_  = left;
	key_right_ = right;
	return std::dynamic_pointer_cast<ComponentObjectController>(shared_from_this());
}

ComponentObjectControllerPtr ComponentObjectController::SetCameraKeys(int up, int down, int left, int right)
{
	// カメラキーの設定
	cam_up_	   = up;
	cam_down_  = down;
	cam_left_  = left;
	cam_right_ = right;
	return std::dynamic_pointer_cast<ComponentObjectController>(shared_from_this());
}

ComponentObjectControllerPtr ComponentObjectController::SetTarget(ObjectPtr target)
{
	target_ = target;
	return std::dynamic_pointer_cast<ComponentObjectController>(shared_from_this());
}

ComponentObjectControllerPtr ComponentObjectController::SetCameraRotate(float3 rot)
{
	cam_rx_ = rot.x;
	cam_ry_ = rot.y;
	return std::dynamic_pointer_cast<ComponentObjectController>(shared_from_this());
}

ComponentObjectControllerPtr ComponentObjectController::SetUseMouse(bool use)
{
	use_mouse_ = use;
	return std::dynamic_pointer_cast<ComponentObjectController>(shared_from_this());
}

bool ComponentObjectController::IsUseMouse() const
{
	return use_mouse_;
}

void ComponentObjectController::GUI()
{
	__super::GUI();

	// GUI内に出現させる
	ImGui::Begin(GetOwner()->GetName().data());
	{
		ImGui::Separator();
		if(ImGui::TreeNode(u8"ObjectController"))
		{
			// 有効/無効
			bool enable = GetStatus(StatusBit::Enable);
			if(ImGui::Checkbox(u8"有効", &enable))
				SetStatus(StatusBit::Enable, enable);

			// GUI上でオーナーから自分(SampleObjectController)を削除します
			if(ImGui::Button(u8"削除"))
				GetOwner()->RemoveComponent(shared_from_this());

			// 移動の基本情報
			ImGui::DragFloat(u8"移動速度", &move_speed_, 0.1f);
			ImGui::DragFloat(u8"移動回転角度", &rot_speed_, 1.0f);

			// カメラの基本情報
			ImGui::Checkbox(u8"カメラ方向にMouseを使用", &use_mouse_);
			ImGui::DragFloat(u8"カメラ左右回転スピード", &mouse_left_right_);
			ImGui::DragFloat(u8"カメラ上下回転スピード", &mouse_up_down_);

			ImGui::DragFloat(u8"見る方向上リミット", &limit_cam_up_);
			ImGui::DragFloat(u8"見る方向下リミット", &limit_cam_down_);

			// 移動の基本情報
			ImGui::DragFloat(u8"オブジェクト オフセット回転", &front_rot_, 1.0f);

			ImGui::TreePop();
		}
	}
	ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentObjectController)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentObjectController)
