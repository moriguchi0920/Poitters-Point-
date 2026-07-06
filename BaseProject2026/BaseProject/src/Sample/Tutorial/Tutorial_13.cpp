#include <System/Scene.h>
#include <System/Component/Component.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentSpringArm.h>
#include <System/Component/ComponentObjectController.h>
#include <System/Component/ComponentPhysics.h>
#include <System/Component/ComponentEffect.h>

namespace Tutorial
{

	//-------------------------------------------------------
	class Tutorial_13 : public Scene::Base
	{
	public:
		BP_CLASS_DECL(Tutorial_13, u8"(13)Tutorial Physicsの使用方法");

		//! @brief 初期化
		//! @return 初期化が終わったか
		bool Init() override
		{
			//-------------------------------------------------------------------------------
			// 地面
			//-------------------------------------------------------------------------------
			{
				auto ground = Scene::Object::Create<Object>()	 //< Object作成
								  ->SetName(u8"地面");			 //< 名前設定

				auto model = ground->AddComponent<ComponentModel>("data/Sample/SwordBout/Stage/Stage00.mv1");
				//ground->AddComponent<ComponentCollisionModel>()->AttachToModel();
				ground->AddComponent<ComponentPhysics>()->CreateMesh(model);
			}

			//-------------------------------------------------------------------------------
			// Mouse
			//-------------------------------------------------------------------------------
			{
				auto obj = Scene::Object::Create<Object>()	  //< Object作成
							   ->SetName(u8"ネズミ")		  //< 名前設定
							   ->SetTranslate({0, 10, 0});	  //< 位置設定

				// モデル
				obj->AddComponent<ComponentModel>("data/Sample/Player/model.mv1")
					->SetAnimation({
						{"idle", "data/Sample/Player/Anim/Idle2.mv1", 0, 1.0f}, // Idle
						{"walk",	 "data/Sample/Player/Anim/Walk.mv1", 1, 1.0f}  // Walk
				});

				// コリジョン
				//obj->AddComponent<ComponentCollisionCapsule>()    //
				//    ->SetRadius(3.0f)                             //
				//    ->SetHeight(12.5f);
				//->UseGravity();

				// Physicsコリジョン
				auto physics = obj->AddComponent<ComponentPhysics>();
				physics->CreateCapsule(5, 3);												//< カプセル形状
				physics->SetTranslate({0, 7, 0});											//< 中心位置を移動
				physics->SetPhysicsStatus(ComponentPhysics::PhysicsBit::Actived, true);		//< 重力有効
				physics->SetPhysicsStatus(ComponentPhysics::PhysicsBit::LockRotX, true);	//< X軸固定
				physics->SetPhysicsStatus(ComponentPhysics::PhysicsBit::LockRotZ, true);	//< Z軸固定
				physics->SetCollisionType(ComponentPhysics::CollisionType::PLAYER);

				obj->AddComponent<ComponentObjectController>()->SetMoveSpeed(2.0f)->SetCameraRotate({-45, 0, 0});

				obj->OnHitPhysicsFunc = [](const ComponentPhysics::HitInfo& info)
				{
					if(info.hit_collision_->GetOwner()->GetNameDefault() == u8"Barrel")
						Scene::Object::Release(info.hit_collision_->GetOwnerPtr());
				};
			}

			//-------------------------------------------------------------------------------
			// Boss
			//-------------------------------------------------------------------------------
			{
				auto boss = Scene::Object::Create<Object>()		   //< Object作成
								->SetName(u8"Boss")				   //< 名前設定
								->SetTranslate({200, 10, 200});	   //< 位置設定
				// モデル
				boss->AddComponent<ComponentModel>("data/Sample/Boss/model.mv1")
					->SetAnimation({
						{"idle",	 "data/Sample/Player/Anim/Idle.mv1", 0, 1.0f}, // Idle
						{"idle",	 "data/Sample/Player/Anim/Walk.mv1", 0, 1.0f}, // Walk
						{"jump", "data/Sample/Player/Anim/Death.mv1", 0, 1.0f}  // Deah
				});

				// コリジョン
				//boss->AddComponent<ComponentCollisionCapsule>()    //
				//    ->SetRadius(3.5f)                              //
				//    ->SetHeight(22.3f)
				//    ->UseGravity();

				auto physics = boss->AddComponent<ComponentPhysics>();
				physics->CreateCapsule(7.2f, 2.8f);
				physics->SetTranslate({0, 9, 0});
				physics->SetPhysicsStatus(ComponentPhysics::PhysicsBit::Actived, true);
				physics->SetPhysicsStatus(ComponentPhysics::PhysicsBit::LockRotX, true);
				physics->SetPhysicsStatus(ComponentPhysics::PhysicsBit::LockRotZ, true);
				physics->SetCollisionType(ComponentPhysics::CollisionType::ENEMY);
				//phys->SetCollisionType() SetHitCollision(ComponentPhysics::CollisionType::PLAYER, (u16)ComponentPhysics::CollisionType::ALL);
				//SetHitCollision(ComponentPhysics::CollisionType::ENEMY, (u16)ComponentPhysics::CollisionType::ALL);
			}

			//-------------------------------------------------------------------------------
			// Boss2(ただのドラム缶)
			//-------------------------------------------------------------------------------
			{
				for(int i = 0; i < 20; i++)
				{
					auto boss = Scene::Object::Create<Object>()			//< Object作成
									->SetName(u8"Barrel")				//< 名前設定
									->SetTranslate({100, 100, 200});	//< 位置設定
					// モデル
					auto model = boss->AddComponent<ComponentModel>("data/Sample/oil_barrels_pbr/barrel.mv1");
					model->SetTranslate({0, 4.8, 0});
					model->SetScaleAxisXYZ({10, 10, 10});

					// マテリアルを作成
					ComponentModel::Material mat0;
					mat0.SetDiffuse("data/Sample/oil_barrels_pbr/textures/drum1_base_color.png");
					mat0.SetNormal("data/Sample/oil_barrels_pbr/textures/drum1_normal.png");
					mat0.SetRoughness("data/Sample/oil_barrels_pbr/textures/drum1_roughness.png");
					mat0.SetMetalness("data/Sample/oil_barrels_pbr/textures/drum1_metallic.png");
					model->SetMaterial(0, mat0);

					auto			physics = boss->AddComponent<ComponentPhysics>();
					constexpr float density = 10.0f;	// 質量
					physics->CreateCylinder(4.8f, 3.5f, density);
					physics->SetTranslate({0, 5, 0});
					physics->SetPhysicsStatus(ComponentPhysics::PhysicsBit::Actived, true);

					physics->SetPhysicsReal(1.0f);	  //< Physics利用率100%

					//physics->SetPhysicsStatus( ComponentPhysics::PhysicsBit::LockRotX, true );
					//physics->SetPhysicsStatus( ComponentPhysics::PhysicsBit::LockRotZ, true );
					physics->SetCollisionType(ComponentPhysics::CollisionType::ENEMY);
					//phys->SetCollisionType() SetHitCollision(ComponentPhysics::CollisionType::PLAYER, (u16)ComponentPhysics::CollisionType::ALL);
					//SetHitCollision(ComponentPhysics::CollisionType::ENEMY, (u16)ComponentPhysics::CollisionType::ALL);
				}
			}

			//-------------------------------------------------------------------------------
			// カメラ
			//-------------------------------------------------------------------------------
			{
				auto cam_obj = Scene::Object::Create<Object>()	  //< Object作成
								   ->SetName("camera");			  //< 名前設定

				cam_obj
					->AddComponent<ComponentCamera>()				 //< 標準カメラ
					->SetPerspective(45)							 //< 画角
					->SetPositionAndTarget({0, 0, 0}, {0, 0, 1});	 //< SpringArmとコリジョンを使う場合はこれで

				cam_obj->AddComponent<ComponentSpringArm>()
					->SetSpringArmLength(30.0f)			   //< Objectとの距離 (オブジェクトにはカメラがついている)
					->SetSpringArmLookOffset({0, 8, 0})	   //< 少し上を見る
					->SetSpringArmObject(u8"ネズミ");	   //< オブジェクトはネズミ(すでに存在しなければいけない)

				auto physics = cam_obj->AddComponent<ComponentPhysics>();
				physics->CreateSphere({0, 0, 0}, 1.0f, 1.0f);
				physics->SetCollisionType(ComponentPhysics::CollisionType::CAMERA);

				// これらの項目を入れないとぶれる
				physics->SetPhysicsReal(1.0f);
				physics->SetRestitution(0.0f);
				physics->SetFriction(0.0f);
				physics->SetGravityFactor(0.0f);

				//cam_obj->AddComponent<ComponentCollisionSphere>()
				//	->SetName( u8"CameraCollision" )
				//	->SetRadius( 1.0f )
				//	->SetCollisionGroup( ComponentCollision::CollisionGroup::CAMERA )
				//	->SetHitCollisionGroup( (u32)ComponentCollision::CollisionGroup::GROUND |  //Ground
				//							(u32)ComponentCollision::CollisionGroup::PLAYER |  //Player
				//							(u32)ComponentCollision::CollisionGroup::ENEMY	   // Enemy
				//						  );
			}

			// 全体の当たりの設定
			//ComponentPhysics::SetHitCollision( ComponentPhysics::CollisionType::PLAYER,
			//								   ( (u16)ComponentPhysics::CollisionType::GROUND | (u16)ComponentPhysics::CollisionType::ENEMY ) );

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

			if(Input::IsKeyDown(KEY_INPUT_SPACE))
			{
				if(auto barrel = Scene::Object::Get<Object>(u8"Barrel"))
				{
					if(auto phys = barrel->GetComponent<ComponentPhysics>())
					{
						phys->GetRigidBody()->addForce({0.0, force_, 0.0f});
					}
				}
				if(auto barrel = Scene::Object::Get<Object>(u8"Barrel_1"))
				{
					if(auto phys = barrel->GetComponent<ComponentPhysics>())
					{
						// 一定の大きさで飛ばしたいときなどは、add, set で飛ばすほうが良いと思われます
						phys->GetRigidBody()->addLinearVelocity({0.0, 250.0f, 0.0f});
					}
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
			ImGui::Begin(GetGUIName().data());
			{
				ImGui::DragFloat("Power", &force_);
			}
			ImGui::End();
		}

		//! @brief 終了
		void Exit() override
		{
		}

	private:
		float force_ = 100000000.0f;
	};

}	 // namespace Tutorial
