#include <System/Component/ComponentSpringArm.h>
#include <System/Component/ComponentTransform.h>
#include <System/Object.h>
#include <System/Scene.h>
#include <System/ImGui.h>

//---------------------------------------------------------
//! 初期化
//---------------------------------------------------------
void ComponentSpringArm::Init()
{
	__super::Init();

	// 向きはPostUpdateの最初で行っておく(カメラのPost向きよりも前になるように)
	Scene::GetCurrentScene()->SetPriority(shared_from_this(), ProcTiming::PostUpdate, ProcPriority::HIGHEST);

	spring_arm_status_.on(SpringArmBit::Initialized);
}

//---------------------------------------------------------
//! 更新
//---------------------------------------------------------
void ComponentSpringArm::PrePhysics()
{
	auto owner = GetOwner();
	if(owner)
	{
		auto target = object_.lock();
		if(target == nullptr)
		{
			SetSpringArmObject(object_name_);
		}

		owner->SetMatrix(GetPutOnMatrix());

		if(!owner->GetStatus(Object::StatusBit::Located))
		{
			owner->UseWarp();
		}

		if(auto physics = owner->GetComponent<ComponentPhysics>())
		{
			physics->MoveToWorldMatrix();
		}
	}
	// 一旦地面に潜るがここから弾を発射させることがあるため
	// ほぼ正しい位置まで移動させる
	// 自分の位置はそのまま使用し、ターゲット位置のほうを向くようにする
	if(auto object = object_.lock())
	{
		auto my_pos		= GetOwner()->GetTranslate();
		auto target_pos = object->GetTranslate();

		matrix mat = HelperLib::Math::LookAtMatrixForObject(my_pos, target_pos);

		float3 ofs = mul(float4(spring_arm_offset_, 0), mat).xyz;
		float3 lok = mul(float4(spring_arm_look_, 0), mat).xyz;
		mat		   = HelperLib::Math::LookAtMatrixForObject(my_pos + ofs, target_pos + lok);

		GetOwner()->SetMatrix(mat);
	}
}

void ComponentSpringArm::PostUpdate()
{
	if(auto physics = GetOwner()->GetComponent<ComponentPhysics>())
	{
		if(auto body = physics->GetRigidBody())
		{
			auto mat = body->worldMatrix();
			GetOwner()->SetMatrix(mat);
		}
	}

	if(spring_arm_strong_ < 1.0f)
		return;

	// 自分の位置はそのまま使用し、ターゲット位置のほうを向くようにする
	if(auto object = object_.lock())
	{
		auto my_pos		= GetOwner()->GetTranslate();
		auto target_pos = object->GetTranslate();

		matrix mat = HelperLib::Math::LookAtMatrixForObject(my_pos, target_pos);

		float3 ofs = mul(float4(spring_arm_offset_, 0), mat).xyz;
		float3 lok = mul(float4(spring_arm_look_, 0), mat).xyz;

		mat = HelperLib::Math::LookAtMatrixForObject(my_pos + ofs, target_pos + lok);

		GetOwner()->SetMatrix(mat);
	}
}
//! @brief スプリングアームの先からのマトリクス取得
//! @return マトリクス
matrix ComponentSpringArm::GetPutOnMatrix() const
{
	float  trans[3] = {0, 0, 0};
	float  rot[3]	= {spring_arm_rotate_.x, spring_arm_rotate_.y, spring_arm_rotate_.z};
	float  scale[3] = {1.0f, 1.0f, 1.0f};
	matrix rmat;
	RecomposeMatrixFromComponents(trans, rot, scale, rmat.f32_128_0);

	matrix tmat = matrix::translate(0, 0, spring_arm_length_);

	matrix mat = matrix::identity();
	if(auto object = object_.lock())
	{
		if(auto transform = object->GetComponent<ComponentTransform>())
		{
			mat = transform->GetMatrix();
		}
	}
	mat = mul(rmat, mat);
	mat = mul(tmat, mat);

	if(on_position_)
	{
		float3 pos	   = mat.translate();
		velocity_	   = (pos - old_position_) * 0.1f + velocity_ * 0.9f;
		pos			   = (pos - old_position_) * spring_arm_strong_ + old_position_;
		pos			  += velocity_ * spring_arm_return_;
		old_position_  = pos;
		mat[3]		   = {pos, 0};
	}
	on_position_ = true;

	return mat;
}

void ComponentSpringArm::SetSpringArmObject(ObjectPtr object)
{
	if(object)
	{
		object_		 = object;
		object_name_ = object->GetName();
		GetOwner()->SetStatus(Object::StatusBit::Located, false);
	}
}

void ComponentSpringArm::SetSpringArmObject(std::string_view name)
{
	if(!name.empty())
	{
		if(auto obj = Scene::Object::Get<Object>(name))
			SetSpringArmObject(obj);
	}

#if 0
	for ( auto obj : Scene::GetObjectsPtr<Object>() )
	{
		if ( obj->GetName() == name )
		{
			SetSpringArmObject( obj );
			break;
		}
	}
#endif
}

//---------------------------------------------------------
//! デバッグ表示
//---------------------------------------------------------
void ComponentSpringArm::Draw()
{
}

//---------------------------------------------------------
//! カメラ終了処理
//---------------------------------------------------------
void ComponentSpringArm::Exit()
{
	__super::Exit();
}

//---------------------------------------------------------
//! GUI処理
//---------------------------------------------------------
void ComponentSpringArm::GUI()
{
	assert(GetOwner());
	auto obj_name = GetOwner()->GetName();

	ImGui::Begin(obj_name.data());
	{
		ImGui::Separator();
		if(ImGui::TreeNode(u8"SpringArm"))
		{
			if(ImGui::Button(u8"削除"))
			{
				GetOwner()->RemoveComponent(shared_from_this());
			}

			u32* bit = &spring_arm_status_.get();
			u32	 val = *bit;
			ImGui::CheckboxFlags(u8"初期化済", &val, 1 << (int)SpringArmBit::Initialized);

			if(ImGui::BeginCombo("Object", object_name_.data()))
			{
				auto objs = Scene::GetObjectsPtr<Object>();
				for(int i = 0; i < objs.size(); i++)
				{
					auto		obj			= objs[i];
					std::string object_name = std::string(obj->GetName());
					if(obj == GetOwnerPtr())
						continue;

					bool is_selected = (object_name_ == object_name);
					if(ImGui::Selectable(object_name.data(), is_selected))
					{
						object_name_ = object_name;
						object_		 = obj;
					}
				}
				ImGui::EndCombo();
			}

			ImGui::DragFloat3(u8"SpringArm回転", (float*)&spring_arm_rotate_, 0.1f, -10000.0f, 10000.0f, "%.1f");
			ImGui::DragFloat3(u8"SpringArm位置オフセット", (float*)&spring_arm_offset_, 0.1f, -10000.0f, 10000.0f, "%.1f");
			ImGui::DragFloat3(u8"SpringArm注視オフセット", (float*)&spring_arm_look_, 0.1f, -10000.0f, 10000.0f, "%.1f");
			ImGui::DragFloat(u8"SpringArm長さ", (float*)&spring_arm_length_, 0.1f, -0.1f, 1000.0f, "%.1f");

			ImGui::DragFloat(u8"SpringArm 固さ", (float*)&spring_arm_strong_, 0.1f, 0.0f, 1.0f);
			ImGui::DragFloat(u8"SpringArm 戻り", (float*)&spring_arm_return_, 0.1f, 0.0f, 1.0f);

			ImGui::TreePop();
		}
	}
	ImGui::End();
}
