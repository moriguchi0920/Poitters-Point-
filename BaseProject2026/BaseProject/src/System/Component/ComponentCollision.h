//---------------------------------------------------------------------------
//! @file   ComponentCollision.h
//! @brief  コリジョンコンポーネント(ベースクラス)
//---------------------------------------------------------------------------
#pragma once

#include <System/Component/Component.h>
#include <System/Component/ComponentHitInfo.h>
#include <ImGuizmo/ImGuizmo.h>

#ifdef USE_JOLT_PHYSICS
	#include <System/Physics/PhysicsEngine.h>
	#include <System/Physics/PhysicsLayer.h>
	#include <System/Physics/RigidBody.h>
	#include <System/Physics/Shape.h>
	#include <System/Physics/PhysicsCharacter.h>
using RigidBodyPtr = std::shared_ptr<physics::RigidBody>;
#endif

USING_PTR(ComponentCollision);
USING_PTR(ComponentCollisionCapsule);
USING_PTR(ComponentCollisionSphere);
USING_PTR(ComponentCollisionModel);
USING_PTR(ComponentCollisionLine);
USING_PTR(ComponentCollisionBox);

//! @brief モデルコンポーネントクラス
class ComponentCollision : public Component
{
public:
	//! @brief ヒット情報
	struct HitInfo
	{
		bool				  hit_			 = false;				  //!< ヒットしたか
		ComponentCollisionPtr collision_	 = nullptr;				  //!< 自分のコリジョン
		float3				  push_			 = {0.0f, 0.0f, 0.0f};	  //!< めり込み量
		float3				  hit_position_	 = {0.0f, 0.0f, 0.0f};	  //!< 当たった地点
		ComponentCollisionPtr hit_collision_ = nullptr;				  //!< 当たったコリジョン
	};

	ComponentCollision();
	void Construct(ObjectPtr owner);

	virtual void LateUpdate() override;	   //!< LateUpdate
	virtual void PostUpdate() override;	   //!< PostUpdate
	virtual void GUI() override;		   //!< GUI

	//! @brief コリジョンデータ内容を表示する
	//! @param use_attach
	void GUICollisionData(bool use_attach = true);

	//! @brief 当たりをチェックします
	//! @param collision 相手コリジョン
	//! @return 当たりの情報
	virtual HitInfo IsHit(ComponentCollisionPtr)
	{
		// オーバーライドしてください
		return HitInfo();
	}

	//! @brief 当たった情報はコールバックで送られてくる
	//! @param hitInfo 当たった情報
	//! @details 当たった回数分ここに来ます
	virtual void OnHit(const HitInfo& hitInfo);

	//! @brief ラムダ当たり処理実装
	std::function<void(const HitInfo& hit_info)> OnHitFunc;

#if 0	 // 通常コンポーネントへ移動
    void SetName(std::string_view name);

	std::string_view GetName();
#endif

	//---------------------------------------------------------------------------
	//! コリジョンステータス
	//---------------------------------------------------------------------------
	enum struct CollisionBit : u32
	{
		Initialized,		 //!< 初期化済み
		DisableHit,			 //!< 当たらない
		ShowInGame,			 //!< ゲーム中にも当たりが見える
		IsGround,			 //!< グランド上にいる
		UsePhysics,			 //!< 移動でPhysicsが有効になります
		DisableTransform,	 //!< Transformを影響を受けなくする(ワールド空間の座標で扱う)
	};

	bool IsCollisionStatus(CollisionBit bit) const
	{
		return collision_status_.is(bit);
	}

	void SetCollisionStatus(CollisionBit bit, bool b)
	{
		collision_status_.set(bit, b);
	}

	//! @brief タイプは増えたら登録する必要がある
	enum struct CollisionType : u32
	{
		NONE = (u32)-1,
		LINE = 0,
		TRIANGLE,
		SPHERE,
		CAPSULE,
		MODEL,
		BOX,
	};

	//! @brief コリジョンタイプ名
	const std::vector<std::string> CollisionTypeName{
		"LINE",		   //< ライン
		"TRIANGLE",	   //< トライアングル
		"SPHERE",	   //< スフィア
		"CAPSULE",	   //< カプセル
		"MODEL",	   //< モデル
		"BOX",		   //< ボックス
	};

	enum struct CollisionGroupBit : u32
	{
		WALL   = 0,
		GROUND = 1,
		PLAYER = 2,
		ENEMY  = 3,
		WEAPON = 4,
		ITEM   = 5,
		CAMERA = 6,
		ETC	   = 7,
#if 0
		WALL2	= 8,
		GROUND2	= 9,
		PLAYER2 = 10,
		ENEMY2	= 11,
		WEAPON2 = 12,
		ITEM2	= 13,
		CAMERA2 = 14,
		ETC2	= 15,
#endif
	};

	enum struct CollisionGroup : u32
	{
		WALL   = 1 << static_cast<u32>(CollisionGroupBit::WALL),
		GROUND = 1 << static_cast<u32>(CollisionGroupBit::GROUND),
		PLAYER = 1 << static_cast<u32>(CollisionGroupBit::PLAYER),
		ENEMY  = 1 << static_cast<u32>(CollisionGroupBit::ENEMY),
		WEAPON = 1 << static_cast<u32>(CollisionGroupBit::WEAPON),
		ITEM   = 1 << static_cast<u32>(CollisionGroupBit::ITEM),
		CAMERA = 1 << static_cast<u32>(CollisionGroupBit::CAMERA),
		ETC	   = 1 << static_cast<u32>(CollisionGroupBit::ETC),
#if 0
		WALL2   = 1 << static_cast<u32>( CollisionGroupBit::WALL2 ),
		GROUND2 = 1 << static_cast<u32>( CollisionGroupBit::GROUND2 ),
		PLAYER2 = 1 << static_cast<u32>( CollisionGroupBit::PLAYER2 ),
		ENEMY2  = 1 << static_cast<u32>( CollisionGroupBit::ENEMY2 ),
		WEAPON2 = 1 << static_cast<u32>( CollisionGroupBit::WEAPON2 ),
		ITEM2   = 1 << static_cast<u32>( CollisionGroupBit::ITEM2 ),
		CAMERA2 = 1 << static_cast<u32>( CollisionGroupBit::CAMERA2 ),
		ETC2	= 1 << static_cast<u32>( CollisionGroupBit::ETC2 ),
#endif
	};

	int GetCollisionGroupIndex() const
	{
#pragma warning(disable : 26813)
		CollisionGroup grp = collision_group_;

		// この関数では「grp」はBITを前提としておらず
		// このタイプの名前を知る必要があり、もし&判定した場合、仮にビットが2つついていると
		// 正しい値として返してしまうためワーニングを抑制します

		if(grp == CollisionGroup::WALL)
			return static_cast<u32>(CollisionGroupBit::WALL);
		if(grp == CollisionGroup::GROUND)
			return static_cast<u32>(CollisionGroupBit::GROUND);
		if(grp == CollisionGroup::PLAYER)
			return static_cast<u32>(CollisionGroupBit::PLAYER);
		if(grp == CollisionGroup::ENEMY)
			return static_cast<u32>(CollisionGroupBit::ENEMY);
		if(grp == CollisionGroup::WEAPON)
			return static_cast<u32>(CollisionGroupBit::WEAPON);
		if(grp == CollisionGroup::ITEM)
			return static_cast<u32>(CollisionGroupBit::ITEM);
		if(grp == CollisionGroup::CAMERA)
			return static_cast<u32>(CollisionGroupBit::CAMERA);
		if(grp == CollisionGroup::ETC)
			return static_cast<u32>(CollisionGroupBit::ETC);

		// 登録し忘れの可能性があります
		return -1;
#pragma warning(default : 26813)
	}

	std::string GetCollisionGroupName() const
	{
#pragma warning(disable : 26813)
		CollisionGroup grp = collision_group_;

		// この関数では「grp」はBITを前提としておらず
		// このタイプの名前を知る必要があり、もし&判定した場合、仮にビットが2つついていると
		// 正しい値として返してしまうためワーニングを抑制します

		if(grp == CollisionGroup::WALL)
			return "WALL";
		if(grp == CollisionGroup::GROUND)
			return "GROUND";
		if(grp == CollisionGroup::PLAYER)
			return "PLAYER";
		if(grp == CollisionGroup::ENEMY)
			return "ENEMY";
		if(grp == CollisionGroup::WEAPON)
			return "WEAPON";
		if(grp == CollisionGroup::ITEM)
			return "ITEM";
		if(grp == CollisionGroup::CAMERA)
			return "CAMERA";
		if(grp == CollisionGroup::ETC)
			return "ETC";

		// 登録し忘れの可能性があります
		return "UNKNOWN";
#pragma warning(default : 26813)
	}

	inline const CollisionType GetCollisionType() const
	{
		return collision_type_;
	}

	inline const CollisionGroup GetCollisionGroup() const
	{
		return collision_group_;
	}

#if 0
	inline ComponentCollisionPtr SetHitCollisionGroup( u32 hit_group )
	{
		collision_hit_ = hit_group;
		return std::dynamic_pointer_cast<ComponentCollision>( shared_from_this() );
	}

    inline ComponentCollisionPtr SetCollisionGroup(CollisionGroup grp)
    {
        collision_group_ = grp;
		return std::dynamic_pointer_cast<ComponentCollision>( shared_from_this() );
    }

	inline ComponentCollisionPtr SetMass( float mass )
    {
        collision_mass_ = mass;
		return std::dynamic_pointer_cast<ComponentCollision>( shared_from_this() );
	}
#endif

	inline float GetMass() const
	{
		return collision_mass_;
	}

	inline u32 GetId() const
	{
		return collision_id_;
	}

	//----------------------------------------------------------
	// @name アタッチ関係
	//----------------------------------------------------------
	void AttachToModel(int node);

	void AttachToModel(const std::string_view node);

#if 1
	//! @brief コリジョンマトリクス
	float* GetColMatrixFloat()
	{
		return collision_transform_.f32_128_0;
	}

	float3 GetColMatrixTranslate() const
	{
		return collision_transform_._41_42_43;
	}
#endif

	//! @brief  押し戻し量の再計算
	//! @param collision        相手のコリジョン
	//! @param vec              自分100%の戻り量
	//! @param push [out]       本当の自分の押し戻し量
	//! @param other_push [out] 相手の押し戻し量
	//! @return 押し戻しできる状態か?
	bool CalcPush(ComponentCollisionPtr collision, float3 vec, float3* push, float3* other_push) const
	{
		float mass1 = GetMass();
		float mass2 = collision->GetMass();
		if(mass1 < 0 && mass2 < 0)
		{
			// 両方不動のものがぶつかった
			*push		= vec * 0.5f;
			*other_push = -vec * 0.5f;
			return false;
		}
		else if(mass1 < 0)
		{
			// 相手に跳ね返す
			*push		= {0, 0, 0};
			*other_push = -vec;
		}
		else if(mass2 < 0)
		{
			// 相手に跳ね返す
			*push		= vec;
			*other_push = {0, 0, 0};
		}
		else
		{
			// 重さに合わせて移動させる
			mass1		+= 0.001f;
			mass2		+= 0.001f;
			float all	 = mass1 + mass2;
			*push		 = vec * mass2 / all;
			*other_push	 = -vec * mass1 / all;
		}
		return true;
	}

	bool IsGroupHit(ComponentCollisionPtr collision) const
	{
		// コリジョンが当たらない設定になっている
		if(collision_status_.is(CollisionBit::DisableHit) || collision->collision_status_.is(CollisionBit::DisableHit))
			return false;

		// 自分か相手のコリジョンタイプが不定
		if(collision_type_ == CollisionType::NONE || collision->collision_type_ == CollisionType::NONE)
			return false;

		// 自分は相手のコリジョングループに当たらない
		if(((u32)collision_group_ & collision->collision_hit_) == 0)
			return false;

		// 相手は自分のコリジョングループに当たらない
		if(((u32)collision->collision_group_ & collision_hit_) == 0)
			return false;

		// 当たる設定となる
		return true;
	}

	float3 GetGravity()
	{
		return gravity_;
	}
	void SetGravity(float3 g)
	{
		gravity_ = g;
	}

	void UseGravity(bool b = true)
	{
		use_gravity_ = b;
	}

	bool IsUseGravity()
	{
		return use_gravity_;
	}

	void [[deprecated("Overlap()は古い命名です。SetOverlapCollisionGroup()を使用してください")]] Overlap(u32 bit)
	{
		collision_overlap_ = bit;
	}

	bool IsOverlap(CollisionGroup bit)
	{
		return (collision_overlap_ & (u32)bit) != 0 ? true : false;
	}

	void ShowInGame(bool b = true)
	{
		collision_status_.set(CollisionBit::ShowInGame, b);
	}

	bool IsShowInGame()
	{
		return collision_status_.is(CollisionBit::ShowInGame);
	}

	void SetClimbLimit(float climb_limit)
	{
		difficult_to_climb_ = climb_limit;
	}

	float GetClimbLimit()
	{
		return difficult_to_climb_;
	}

protected:
	//----------------------------------------------------------------------------
	//! GUI情報
	//----------------------------------------------------------------------------
	//! @brief コリジョン情報表示
	void guiCollisionData();
	//! @brief アタッチ情報のみ表示
	void guiCollisionDataAttach();

	//----------------------------------------------------------------------------
	//! @name 当たり判定処理
	//----------------------------------------------------------------------------
	//@{

	//! @brief Capsule VS Sphere
	//! @param col1 Capsuleコリジョン
	//! @param col2 Sphere コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionCapsulePtr col1, ComponentCollisionSpherePtr col2);

	//! @brief Sphere VS Capsule
	//! @param col1 Sphereコリジョン
	//! @param col2 Capsule コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionSpherePtr col1, ComponentCollisionCapsulePtr col2);

	//! @brief Capsule VS Capsule
	//! @param col1 Capsuleコリジョン
	//! @param col2 Capsule コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionCapsulePtr col1, ComponentCollisionCapsulePtr col2);

	//! @brief Sphere VS Sphere
	//! @param col1 Sphereコリジョン
	//! @param col2 Sphere コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionSpherePtr col1, ComponentCollisionSpherePtr col2);

	//! @brief Model VS Sphere
	//! @param col1 Modelコリジョン
	//! @param col2 Sphere コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionModelPtr col1, ComponentCollisionSpherePtr col2);

	//! @brief Sphere VS Model
	//! @param col1 Shpereコリジョン
	//! @param col2 Model コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionSpherePtr col1, ComponentCollisionModelPtr col2);

	//! @brief Model VS Capsule
	//! @param col1 Modelコリジョン
	//! @param col2 Capsule コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionModelPtr col1, ComponentCollisionCapsulePtr col2);

	//! @brief Capsule VS Model
	//! @param col1 Capsuleコリジョン
	//! @param col2 Model コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionCapsulePtr col1, ComponentCollisionModelPtr col2);

	//! @brief Line VS Sphere
	//! @param col1 Capsuleコリジョン
	//! @param col2 Sphere コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionLinePtr col1, ComponentCollisionSpherePtr col2);

	//! @brief Line VS Capsule
	//! @param col1 Sphereコリジョン
	//! @param col2 Capsule コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionLinePtr col1, ComponentCollisionCapsulePtr col2);

	//! @brief Line VS Model
	//! @param col1 Capsuleコリジョン
	//! @param col2 Capsule コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionLinePtr col1, ComponentCollisionModelPtr col2);

	//! @brief Line VS Sphere
	//! @param col1 Sphere コリジョン
	//! @param col2 Line コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionSpherePtr col1, ComponentCollisionLinePtr col2);

	//! @brief Line VS Capsule
	//! @param col1 Capsuleコリジョン
	//! @param col2 Line コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionCapsulePtr col1, ComponentCollisionLinePtr col2);

	//! @brief Line VS Model
	//! @param col1 Modelコリジョン
	//! @param col2 Line コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionModelPtr col1, ComponentCollisionLinePtr col2);

	//! @brief Box VS Sphere
	//! @param col1 Boxコリジョン
	//! @param col2 Sphere コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionBoxPtr col1, ComponentCollisionSpherePtr col2);

	//! @brief Sphere VS Box
	//! @param col1 Sphereコリジョン
	//! @param col2 Box コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionSpherePtr col1, ComponentCollisionBoxPtr col2);

	//! @brief Box VS Capsule
	//! @param col1 Boxコリジョン
	//! @param col2 Capsule コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionBoxPtr col1, ComponentCollisionCapsulePtr col2);

	//! @brief Capsule VS Box
	//! @param col1 Capsuleコリジョン
	//! @param col2 Box コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionCapsulePtr col1, ComponentCollisionBoxPtr col2);

	//! @brief Box VS Model
	//! @param col1 Boxコリジョン
	//! @param col2 Model コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionBoxPtr col1, ComponentCollisionModelPtr col2);

	//! @brief Model VS Box
	//! @param col1 Modelコリジョン
	//! @param col2 Box コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionModelPtr col1, ComponentCollisionBoxPtr col2);

	//! @brief Box VS Line
	//! @param col1 Boxコリジョン
	//! @param col2 Line コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionBoxPtr col1, ComponentCollisionLinePtr col2);

	//! @brief Line VS Box
	//! @param col1 Lineコリジョン
	//! @param col2 Box コリジョン
	//! @return 当たり情報
	ComponentCollision::HitInfo isHit(ComponentCollisionLinePtr col1, ComponentCollisionBoxPtr col2);

	//@}

	//! コリジョン用のトランスフォーム
	matrix collision_transform_ = matrix::identity();

	//! 1フレーム前の状態 (WorldTransform)
	matrix old_transform_ = matrix::identity();

	CollisionType  collision_type_	  = CollisionType::NONE;
	CollisionGroup collision_group_	  = CollisionGroup::ETC;	//!< 自分のコリジョンタイプ
	u32			   collision_hit_	  = 0xffffffff;				//!< デフォルトではすべてに当たる
	u32			   collision_overlap_ = 0x00000000;				//!< デフォルトではオーバーラップしない

	Status<CollisionBit> collision_status_;	   //!< 状態

	float collision_mass_ = 1;	  //!< 押し戻される量に影響(マイナスは戻されない)
	u32	  collision_id_	  = 0;	  //!< コリジョン識別子

	int	   attach_node_		   = -1;	//!< モデルノードに付くときは0以上
	matrix attach_node_matrix_ = matrix::identity();

	bool		 use_gravity_		 = false;
	const float3 k_gravity_			 = {0, 100, 0};	   // 前の状態に合わせるための係数
	float3		 gravity_			 = {0.0f, -0.98f, 0.0f};
	float3		 now_gravity_		 = {0.0f, 0.0f, 0.0f};
	float3		 prev_gravity_		 = {0.0f, 0.0f, 0.0f};
	float		 difficult_to_climb_ = 0.5f;

	//bool is_overlap_ = false;

#if 0	 // 通常コンポーネントへ
	std::string name_ = "None";
#endif
#ifdef USE_JOLT_PHYSICS

public:
	RigidBodyPtr& RigidBody()
	{
		return rigid_body_;
	}

	RigidBodyPtr GetRigidBody() const
	{
		return rigid_body_;
	}

	void SetRigidBody(RigidBodyPtr ptr)
	{
		rigid_body_ = ptr;
	}

protected:
	RigidBodyPtr rigid_body_{};
#endif

private:
	//--------------------------------------------------------------------
	//! @name Cereal処理
	//--------------------------------------------------------------------
	//@{
	CEREAL_SAVELOAD(arc, ver)
	{
		arc(cereal::make_nvp("owner", owner_),
			cereal::make_nvp("collision_type", (u32)collision_type_),
			cereal::make_nvp("collision_group", (u32)collision_group_),
			cereal::make_nvp("collision_hit", collision_hit_),
			cereal::make_nvp("collision_mass", collision_mass_),
			cereal::make_nvp("collision_id", collision_id_),
			cereal::make_nvp("attach_node", attach_node_),
			cereal::make_nvp("attach_node_matrix", attach_node_matrix_),
			cereal::make_nvp("collision_transform", collision_transform_),
			cereal::make_nvp("old_transform", old_transform_));

		if(ver >= 1)
		{
			arc(cereal::make_nvp("use_gravity", use_gravity_),
				cereal::make_nvp("gravity", gravity_),
				cereal::make_nvp("now_gravity", now_gravity_),
				cereal::make_nvp("difficult_to_climb", difficult_to_climb_));
			if(ver >= 2)
			{
				arc(cereal::make_nvp("collision_overlap", collision_overlap_));
			}
			else
			{
				arc(cereal::make_nvp("name", name_));
			}
		}
		arc(cereal::make_nvp("Component", cereal::base_class<Component>(this)));
	}

	//@}
};

CEREAL_CLASS_VERSION(ComponentCollision, 3);

CEREAL_REGISTER_TYPE(ComponentCollision)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentCollision)
