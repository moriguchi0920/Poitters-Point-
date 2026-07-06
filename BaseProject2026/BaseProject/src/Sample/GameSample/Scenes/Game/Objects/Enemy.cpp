#include "Enemy.h"
#include "Mouse.h"

#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentCamera.h>
#include <System/Component/ComponentTargetTracking.h>
#include <System/Component/ComponentSpringArm.h>

#include <cmath>

namespace Sample::GameSample
{

	//! @brief オブジェクト(obj)に近い敵を返す
	//! @param obj
	//! @return 近い敵
	ObjectPtr TrackingNearEnemy(ObjectPtr obj)
	{
		// シーンにいるEnemyをすべて取得する
		auto enemies = Scene::GetObjectsPtr<Enemy>();

		// 一番近いエネミー
		ObjectPtr near_enemy  = nullptr;
		float	  near_length = 99999999.0f;

		float3 obj_pos = obj->GetTranslate();

		for(auto enemy : enemies)
		{
			if(near_enemy == nullptr)
			{
				near_enemy = enemy;
				continue;
			}

			float3 enemy_pos = enemy->GetTranslate();

			float len = length(enemy_pos - obj_pos);
			if(len < near_length)
			{
				near_enemy	= enemy;
				near_length = len;
			}
		}

		return near_enemy;
	}

	EnemyPtr Enemy::Create(const float3& pos, const float3& front)
	{
		// 箱の作成
		auto enemy = Scene::Object::Create<Enemy>();
		enemy->SetName("Enemy");

		// vecの方向に向ける
		auto mat = HelperLib::Math::CreateMatrixByFrontVector(front);
		enemy->SetMatrix(mat);

		// posの位置に設定
		enemy->SetTranslate(pos);

		return enemy;
	}

	//! @brief 初期化
	//! @return 初期化できたかどうか
	bool Enemy::Init()	  // override
	{
		__super::Init();

		// モデルコンポーネント(0.08倍)
		auto model = AddComponent<ComponentModel>("data/Sample/Player/model.mv1");

		model->SetScaleAxisXYZ({0.08f});	//

		model->SetAnimation({
			{	 "idle",	 "data/Sample/Player/Anim/Idle.mv1", 1, 1.0f},
			{	 "jump",	 "data/Sample/Player/Anim/Jump.mv1", 1, 1.0f},
			{	 "walk",	 "data/Sample/Player/Anim/Walk.mv1", 1, 1.0f},
			{ "walk2",  "data/Sample/Player/Anim/Walk2.mv1", 1, 1.0f},
			{"dance1", "data/Sample/Player/Anim/Dance1.mv1", 0, 1.0f},
			{"dance2", "data/Sample/Player/Anim/Dance2.mv1", 0, 1.0f},
			{"dance3", "data/Sample/Player/Anim/Dance3.mv1", 0, 1.0f},
			{"dance4", "data/Sample/Player/Anim/Dance4.mv1", 0, 1.0f},
			{"dance5", "data/Sample/Player/Anim/Dance5.mv1", 0, 1.0f}
		 });

		// コリジョン(カプセル)
		auto col = AddComponent<ComponentCollisionCapsule>();	 //
		col->SetTranslate({0, 0, 0});
		col->SetRadius(2.5);
		col->SetHeight(10);
		col->SetCollisionGroup(ComponentCollision::CollisionGroup::ENEMY);
		col->UseGravity();

		return true;
	}

	//! @brief 処理更新
	//! @param delta フレーム時間
	void Enemy::Update()	//override
	{
		auto mdl = GetComponent<ComponentModel>();

		float3 target = GetTranslate();

		if(auto mouse = Scene::Object::Get<Object>("Mouse"))
			target = mouse->GetTranslate();

		auto pos = GetTranslate();

		float3 move = (target - pos);

		if(length(move).x > 0)
		{
			// 動いてる
			move = normalize(move);

			float x		= -move.x;
			float z		= -move.z;
			float theta = atan2(x, z) * RadToDeg - rot_y_;
#if 0
			// モデルだけ回転 (ついてるカメラは回らない)
			mdl->SetRotationAxisXYZ( { 0, theta, 0 } );
#else
			// 軸ごと回転 (カメラも一緒に回る)
			SetRotationAxisXYZ({0, theta, 0});
#endif
			if(mdl && mdl->GetPlayAnimationName() != "walk")
				mdl->PlayAnimation("walk", true);
		}
		else
		{
			// 止まってる
			if(mdl && mdl->GetPlayAnimationName() != "idle")
				mdl->PlayAnimation("idle", true);
		}
		move *= speed_ * GetDeltaTime60();

		// 地面移動スピードを決定する
		AddTranslate(move);
	}

	//! @brief GUI処理
	void Enemy::GUI()	 //override
	{
		__super::GUI();

		// GUI描画
		ImGui::Begin(GetName().data());
		{
			ImGui::Separator();

			ImGui::DragFloat(u8"速度", &speed_, 0.01f, 0.01f, 10.0f, "%2.2f");
		}
		ImGui::End();
	}

	//! @brief 当たりのコールバック
	//! @param hitInfo
	void Enemy::OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo)	  //override
	{
		// 当たりで移動させる(これが無ければめり込みます)
		__super::OnHit(hitInfo);
	}

	//! @brief スピードを設定する
	//! @param s スピード
	void Enemy::SetSpeed(float s)
	{
		speed_ = s;
	}

	//! @brief スピードを取得する
	//! @return 現在のスピード
	float Enemy::GetSpeed()
	{
		return speed_;
	}

	//! @brief 変数として扱う関数
	//! @detail Set/Get系と比べデバックはしにくい
	//! @return 現在のスピードの変数(speed_)
	float& Enemy::Speed()
	{
		return speed_;
	}

}	 // namespace Sample::GameSample

// オブジェクトセーブ構造を追加します
CEREAL_REGISTER_TYPE(Sample::GameSample::Enemy)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, Sample::GameSample::Enemy)
