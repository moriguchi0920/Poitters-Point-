#include <System/Scene.h>
#include <System/Component/Component.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentSpringArm.h>
#include <System/Component/ComponentObjectController.h>

namespace Tutorial
{

	//-------------------------------------------------------
	class Tutorial_11 : public Scene::Base
	{
	public:
		BP_CLASS_DECL(Tutorial_11, u8"(11)Tutorial カメラの使い方");

		//! @brief 初期化
		//! @return 初期化が終わったか
		bool Init() override
		{
			//-------------------------------------------------------------------------------
			// 地面
			//-------------------------------------------------------------------------------
			auto ground = Scene::Object::Create<Object>()	 //< Object作成
							  ->SetName(u8"地面");			 //< 名前設定

			auto model = ground->AddComponent<ComponentModel>("data/Sample/SwordBout/Stage/Stage00.mv1");
			ground->AddComponent<ComponentCollisionModel>()->AttachToModel();

			//-------------------------------------------------------------------------------
			// Mouse
			//-------------------------------------------------------------------------------
			auto obj = Scene::Object::Create<Object>()	  //< Object作成
						   ->SetName(u8"ネズミ")		  //< 名前設定
						   ->SetTranslate({0, 0, 0});	  //< 位置設定
			// モデル
			obj->AddComponent<ComponentModel>("data/Sample/Player/model.mv1")
				->SetAnimation({
					{"idle", "data/Sample/Player/Anim/Idle2.mv1", 0, 1.0f}, // Idle
					{"jump", "data/Sample/Player/Anim/Jump2.mv1", 0, 1.0f}  // Jump
			});

			// コリジョン
			obj->AddComponent<ComponentCollisionCapsule>()	  //
				->SetRadius(3.0f)							  //
				->SetHeight(12.5f)
				->UseGravity();

			obj->AddComponent<ComponentObjectController>()->SetMoveSpeed(2.0f);

			//-------------------------------------------------------------------------------
			// Boss
			//-------------------------------------------------------------------------------
			auto boss = Scene::Object::Create<Object>()		  //< Object作成
							->SetName(u8"Boss")				  //< 名前設定
							->SetTranslate({200, 0, 200});	  //< 位置設定
			// モデル
			boss->AddComponent<ComponentModel>("data/Sample/Boss/model.mv1")
				->SetAnimation({
					{"idle",	 "data/Sample/Player/Anim/Idle.mv1", 0, 1.0f}, // Idle
					{"idle",	 "data/Sample/Player/Anim/Walk.mv1", 0, 1.0f}, // Walk
					{"jump", "data/Sample/Player/Anim/Death.mv1", 0, 1.0f}  // Deah
			});

			// コリジョン
			boss->AddComponent<ComponentCollisionCapsule>()	   //
				->SetRadius(3.5f)							   //
				->SetHeight(22.3f)
				->UseGravity();

			//-------------------------------------------------------------------------------
			// カメラ
			//-------------------------------------------------------------------------------
			auto cam_obj = Scene::Object::Create<Object>()	  //< Object作成
							   ->SetName(u8"カメラ");		  //< 名前設定

			cam_obj
				->AddComponent<ComponentCamera>()				 //< 標準カメラ
				->SetPerspective(45)							 //< 画角
				->SetPositionAndTarget({0, 0, 0}, {0, 0, 1});	 //< SpringArmとコリジョンを使う場合はこれで

			cam_obj->AddComponent<ComponentSpringArm>()
				->SetSpringArmRotate({-45, 0, 0})	   //< X軸周りで回転させ45度の角度からObjectを見る
				->SetSpringArmLength(30.0f)			   //< Objectとの距離 (オブジェクトにはカメラがついている)
				->SetSpringArmLookOffset({0, 8, 0})	   //< 少し上を見る
				->SetSpringArmObject(u8"ネズミ");	   //< オブジェクトはネズミ(すでに存在しなければいけない)

			cam_obj->AddComponent<ComponentCollisionSphere>()
				->SetName(u8"CameraCollision")
				->SetRadius(1.0f)
				->SetCollisionGroup(ComponentCollision::CollisionGroup::CAMERA)
				->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::GROUND |	//Ground
									   (u32)ComponentCollision::CollisionGroup::PLAYER |	//Player
									   (u32)ComponentCollision::CollisionGroup::ENEMY		// Enemy
				);

			return true;
		}

		//! @brief 更新
		//! @param delta 更新周期
		void Update() override
		{
			auto boss  = Scene::Object::Get<Object>(u8"Boss");
			auto mouse = Scene::Object::Get<Object>(u8"ネズミ");

			// ボスか操作キャラがいなければ何もしない
			if(boss == nullptr || mouse == nullptr)
				return;

			printfDx("\n\nEnemyがいる 角度: %f.1\n", mouse->GetDegreeToPosition(boss->GetTranslate()));
			if(auto model = mouse->GetComponent<ComponentModel>())
				printfDx("\n\nEnemyがいる 角度: %f.1(MODEL)\n", model->GetDegreeToPosition(boss->GetTranslate()));

			if(auto ctrl = mouse->GetComponent<ComponentObjectController>())
			{
				// 距離が100以内
				float len = length(boss->GetTranslate() - mouse->GetTranslate());
				if(len < 100)
				{
					// Bossの方に向ける
					ctrl->SetTarget(boss);
				}
				else
				{
					ctrl->SetTarget(nullptr);
				}
			}
		}

		//! @brief 描画
		void Draw() override
		{
		}

		//! @brief GUI
		void GUI() override
		{
		}

		//! @brief 終了
		void Exit() override
		{
		}

	private:
	};

}	 // namespace Tutorial
