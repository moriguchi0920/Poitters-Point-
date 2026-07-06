#pragma once
#include <Game/Component/ComponentStateIdleWalk.h>

void ComponentStateIdleWalk::Init()
{
	__super::Init();
}

void ComponentStateIdleWalk::Update()
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
		{
			mdl->PlayAnimationNoSame("idle", true);
		}
	}
}

ComponentStateIdleWalkPtr ComponentStateIdleWalk::SetMoveSpeed(const float speed)
{
	move_speed_ = speed;
	return std::dynamic_pointer_cast<ComponentStateIdleWalk>(shared_from_this());
}
ComponentStateIdleWalkPtr ComponentStateIdleWalk::SetRotateSpeed(const float speed)
{
	rot_speed_ = speed;
	return std::dynamic_pointer_cast<ComponentStateIdleWalk>(shared_from_this());
}

const float ComponentStateIdleWalk::GetMoveSpeed() const
{
	return move_speed_;
}

const float ComponentStateIdleWalk::GetRotateSpeed() const
{
	return rot_speed_;
}

ComponentStateIdleWalkPtr ComponentStateIdleWalk::SetKeys(int up, int down, int left, int right)
{
	// 移動キーの設定
	key_up_	   = up;
	key_down_  = down;
	key_left_  = left;
	key_right_ = right;
	return std::dynamic_pointer_cast<ComponentStateIdleWalk>(shared_from_this());
}

void ComponentStateIdleWalk::GUI()
{
	__super::GUI();

	// GUI内に出現させる
	ImGui::Begin(GetOwner()->GetName().data());
	{
		ImGui::Separator();
		if(ImGui::TreeNode("State IdleWalk"))
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

			// 移動の基本情報
			ImGui::DragFloat(u8"オブジェクト オフセット回転", &front_rot_, 1.0f);

			ImGui::TreePop();
		}
	}
	ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentStateIdleWalk)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateIdleWalk)
