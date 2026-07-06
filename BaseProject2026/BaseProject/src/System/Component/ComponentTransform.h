//---------------------------------------------------------------------------
//! @file   ComponentTransform.h
//! @brief  トランスフォームコンポーネント(座標と姿勢と大きさ)
//---------------------------------------------------------------------------
#pragma once

#include <System/Component/Component.h>
#include <ImGuizmo/ImGuizmo.h>
#include <memory>

class ComponentTransform;
class Object;
//------------------------------------------------------------
//! @brief 三軸ギズモの表示
//! @param matrix マトリクス(位置・回転・スケール)
//! @param ope オペレーション
//! @param mode モード
//! @param id 個別ID
//! @return 表示できたか
//------------------------------------------------------------
bool ShowGizmo(float* matrix, ImGuizmo::OPERATION ope, ImGuizmo::MODE mode, uint64_t id = 0);

// ImGuizmoのMatrixからEulerに変換する際に全軸に180度変換がかかってしまうため修正
extern void DecomposeMatrixToComponents(const float* matx, float* translation, float* rotation, float* scale);
extern void RecomposeMatrixFromComponents(const float* translation, const float* rotation, const float* scale, float* matx);

template <class T>
class IMatrix
{
public:
	//! @brief TransformのMatrix情報を取得します
	//! @return Transform の Matrix
	virtual matrix& Matrix() = 0;

	//! @brief TransformのMatrix情報を取得します
	//! @return Transform の Matrix
	virtual const matrix& GetMatrix() const = 0;

	virtual std::shared_ptr<T> SharedThis() = 0;

	//! @brief ワールドMatrixの取得
	//! @return 自分にかかわる他のコンポーネントも含めた位置
	virtual const matrix GetWorldMatrix() const = 0;

	//! @brief 1フレーム前のワールドMatrixの取得
	//! @return 他のコンポーネントも含めた位置
	virtual const matrix GetOldWorldMatrix() const = 0;

	auto SetMatrix(const matrix& mat)
	{
		Matrix() = mat;
		return SharedThis();
	}

	//! @brief TransformのMatrix情報を取得します
	//! @return Transform の Matrix
	float* GetMatrixFloat()
	{
		return Matrix().f32_128_0;
	}

	//! @brief 位置をセットします
	//! @param translate 位置
	//! @return 自分のSharedPtr
	auto SetTranslate(float3 translate)
	{
		(float4&)Matrix().translateVector() = {translate, 1};

		return SharedThis();
	}

	//! @brief 現在の位置から移動させます
	//! @param translate 移動させたい量
	//! @param local ローカル移動
	//! @param direct FPSに関係なく移動させる
	//! @return 自分のSharedPtr
	auto AddTranslate(float3 translate, bool local = false, bool direct = false)
	{
		float delta = direct ? 1.0f : GetDeltaTime60();

		if(local)
		{
			float3 add = mul(float4(translate, 0), GetMatrix()).xyz;
			SetTranslate(GetTranslate() + add * delta);
			return SharedThis();
		}

		SetTranslate(GetTranslate() + translate * delta);

		return SharedThis();
	}

	//! @brief 現在の位置を取得
	//! @return Transformの位置
	const float3 GetTranslate() const
	{
		return GetMatrix().translate();
	}

	//! @brief 現在の位置を取得(親オブジェクトを含む位置)
	//! @return Transformの位置
	const float3 GetWorldTranslate() const
	{
		return GetWorldMatrix().translate();
	}

	//! @brief 現在の位置を取得
	//! @return Transformの位置
	auto& Translate()
	{
		return Matrix()._41_42_43;
	}

	//! @brief X軸を取得する
	//! @return TransformのX軸
	float3 GetVectorAxisX()
	{
		return GetMatrix().axisX();
	}

	//! @brief Y軸を取得する
	//! @return TransformのY軸
	float3 GetVectorAxisY()
	{
		return GetMatrix().axisY();
	}

	//! @brief Z軸を取得する
	//! @return TransformのZ軸
	float3 GetVectorAxisZ()
	{
		return GetMatrix().axisZ();
	}

	//! @brief X軸を取得する(親オブジェクト含む方向)
	//! @return TransformのX軸
	float3 GetWorldVectorAxisX()
	{
		return GetWorldMatrix().axisX();
	}

	//! @brief Y軸を取得する(親オブジェクト含む方向)
	//! @return TransformのY軸
	float3 GetWorldVectorAxisY()
	{
		return GetWorldMatrix().axisY();
	}

	//! @brief Z軸を取得する(親オブジェクト含む方向)
	//! @return TransformのZ軸
	float3 GetWorldVectorAxisZ()
	{
		return GetWorldMatrix().axisZ();
	}

	//! @brief X軸を取得する
	//! @return TransformのX軸
	auto& VectorAxisX()
	{
		return Matrix()._11_12_13;
	}

	//! @brief Y軸を取得する
	//! @return TransformのY軸
	auto& VectorAxisY()
	{
		return Matrix()._21_22_23;
	}

	//! @brief Z軸を取得する
	//! @return TransformのZ軸
	auto& VectorAxisZ()
	{
		return Matrix()._31_32_33;
	}

	//! @brief 位置を取得する
	//! @return Transformの位置
	float* TranslateFloat()
	{
		return (float*)Matrix().f32_128_3;
	}

	//! @brief X軸を取得する
	//! @return ComponentTransformのX軸
	float* VectorAxisXFloat()
	{
		return (float*)Matrix().f32_128_0;
	}

	//! @brief Y軸を取得する
	//! @return ComponentTransformのY軸
	float* VectorAxisYFloat()
	{
		return (float*)Matrix().f32_128_1;
	}

	//! @brief Z軸を取得する
	//! @return ComponentTransformのZ軸
	float* VectorAxisZFloat()
	{
		return (float*)Matrix().f32_128_2;
	}

	//! @brief X軸を設定する
	//! @param vec ベクトル
	//! @return 自分のSharedPtr
	auto SetVectorAxisX(float3 vec)
	{
		Matrix()._11_12_13 = vec;
		return SharedThis();
	}

	//! @brief Y軸を設定する
	//! @param vec ベクトル
	//! @return 自分のSharedPtr
	auto SetVectorAxisY(float3 vec)
	{
		Matrix()._21_22_23 = vec;
		return SharedThis();
	}

	//! @brief Z軸を設定する
	//! @param vec ベクトル
	//! @return 自分のSharedPtr
	auto SetVectorAxisZ(float3 vec)
	{
		Matrix()._31_32_33 = vec;
		return SharedThis();
	}

	//! @brief ポジションの方向へ向ける
	//! @param position 向きたいポジション
	//! @param up_change Y軸も変更するか?
	//! @return 自分のSharedPtr
	auto SetRotationToPosition(float3 position, float3 up = {0, 1, 0}, bool up_change = false)
	{
		if(!up_change)
			position.y = GetWorldMatrix().translate().y;

		float3 vec = position - GetWorldMatrix().translate();
		return SetRotationToVectorOnParent(vec, up, up_change);
	}

	//! @brief ベクトルの方向へ向ける
	//! @param position 向きたいベクトル
	//! @param up_change Y軸も変更するか?
	//! @return 自分のSharedPtr
	auto SetRotationToVector(float3 vec, float3 up = {0, 1, 0}, bool up_change = false)
	{
		auto mat = HelperLib::Math::CreateMatrixByFrontVector(-vec, up, up_change);

		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		float matrixTranslationVec[3], matrixRotationVec[3], matrixScaleVec[3];
		DecomposeMatrixToComponents(GetMatrixFloat(), matrixTranslation, matrixRotation, matrixScale);

		DecomposeMatrixToComponents(mat.f32_128_0, matrixTranslationVec, matrixRotationVec, matrixScaleVec);

		RecomposeMatrixFromComponents(matrixTranslation, matrixRotationVec, matrixScale, GetMatrixFloat());

		return SharedThis();
	}

	//! @brief ベクトルの方向へ向ける
	//! @param position 向きたいベクトル
	//! @param up_change Y軸も変更するか?
	//! @return 自分のSharedPtr
	auto SetRotationToVectorOnParent(float3 vec, float3 up = {0, 1, 0}, bool up_change = false)
	{
#if 1	 // Transformコンポーネントでなければ親の影響分は差し引く
		matrix mat_tmp = matrix::identity();
		if(!dynamic_cast<ComponentTransform*>(this) && !dynamic_cast<Object*>(this))
		{
			if(auto cmp = dynamic_cast<Component*>(this))
				mat_tmp = cmp->GetOwner()->GetMatrix();
			else if(auto obj = dynamic_cast<Object*>(this))
				mat_tmp = obj->GetMatrix();
		}
		auto vec4 = mul(float4(vec, 0), inverse(mat_tmp));
		vec		  = vec4.xyz;
#endif
		return SetRotationToVector(vec, up, up_change);
	}

	//! @brief ポジションの方向へ向ける
	//! @param position 向きたいポジション
	//! @param up_change Y軸も変更するか?
	//! @return 自分のSharedPtr
	auto SetRotationToPositionWithLimit(float3 position, float limit, float3 up = {0, 1, 0}, bool up_change = false)
	{
		if(!up_change)
			position.y = GetMatrix().translate().y;

		float3 vec = position - GetMatrix().translate();
		return SetRotationToVectorOnParentWithLimit(vec, limit, up, up_change);
	}

	//! @brief ベクトルの方向へ向ける
	//! @param vec 向きたいベクトル
	//! @param limit 向ける限界角度
	//! @param up_change Y軸も変更するか?
	//! @return 自分のSharedPtr
	auto SetRotationToVectorWithLimit(float3 vec, float limit, float3 up = {0, 1, 0}, bool up_change = false)
	{
		float delta = GetDeltaTime60();
		limit		= limit * delta;

		// 今の姿勢マトリクス
		auto now_mat = GetMatrix();

		// 向きたい方向のマトリクス
		auto mat = HelperLib::Math::CreateMatrixByFrontVector(-vec, up, up_change);

		// 現在の姿勢
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		// 向きたい姿勢
		float matrixTranslationVec[3], matrixRotationVec[3], matrixScaleVec[3];

		DecomposeMatrixToComponents(GetMatrixFloat(), matrixTranslation, matrixRotation, matrixScale);

		DecomposeMatrixToComponents(mat.f32_128_0, matrixTranslationVec, matrixRotationVec, matrixScaleVec);

		float rot_x = matrixRotationVec[0] - matrixRotation[0];
		if(rot_x > 180)
			rot_x -= 360;
		if(rot_x < -180)
			rot_x += 360;
		float rot_y = matrixRotationVec[1] - matrixRotation[1];
		if(rot_y > 180)
			rot_y -= 360;
		if(rot_y < -180)
			rot_y += 360;
		float rot_z = matrixRotationVec[2] - matrixRotation[2];
		if(rot_z > 180)
			rot_z -= 360;
		if(rot_z < -180)
			rot_z += 360;

		if(rot_x > 0 && rot_x > limit)
			rot_x = limit;
		if(rot_x < 0 && rot_x < -limit)
			rot_x = -limit;

		if(rot_y > 0 && rot_y > limit)
			rot_y = limit;
		if(rot_y < 0 && rot_y < -limit)
			rot_y = -limit;

		if(rot_z > 0 && rot_z > limit)
			rot_z = limit;
		if(rot_z < 0 && rot_z < -limit)
			rot_z = -limit;

		matrixRotationVec[0] = rot_x + matrixRotation[0];
		matrixRotationVec[1] = rot_y + matrixRotation[1];
		matrixRotationVec[2] = rot_z + matrixRotation[2];
		RecomposeMatrixFromComponents(matrixTranslation, matrixRotationVec, matrixScale, GetMatrixFloat());
		return SharedThis();
	}

	//! @brief ベクトルの方向へ向ける
	//! @param vec 向きたいベクトル
	//! @param limit 向ける限界角度
	//! @param up_change Y軸も変更するか?
	//! @return 自分のSharedPtr
	auto SetRotationToVectorOnParentWithLimit(float3 vec, float limit, float3 up = {0, 1, 0}, bool up_change = false)
	{
#if 1	 // Transformコンポーネントでなければ親の影響分は差し引く
		matrix mat_tmp = matrix::identity();
		if(!dynamic_cast<ComponentTransform*>(this) && !dynamic_cast<Object*>(this))
		{
			if(auto cmp = dynamic_cast<Component*>(this))
				mat_tmp = cmp->GetOwner()->GetMatrix();
			else if(auto obj = dynamic_cast<Object*>(this))
				mat_tmp = obj->GetMatrix();
		}
		auto vec4 = mul(float4(vec, 0), inverse(mat_tmp));
		vec		  = vec4.xyz;
#endif
		return SetRotationToVectorWithLimit(vec, limit, up, up_change);
	}

	//! @brief 軸回転量を設定する
	//! @param xyz 軸に合わせた回転量
	//! @return 自分のSharedPtr
	auto SetRotationAxisXYZ(float3 xyz)
	{
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		DecomposeMatrixToComponents(GetMatrixFloat(), matrixTranslation, matrixRotation, matrixScale);

		matrixRotation[0] = xyz.x;
		matrixRotation[1] = xyz.y;
		matrixRotation[2] = xyz.z;

		RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, GetMatrixFloat());
		return SharedThis();
	}

	//! @brief 軸回転量を現在の状態から追加する
	//! @param xyz 軸に合わせた回転量
	//! @return 自分のSharedPtr
	auto AddRotationAxisXYZ(float3 xyz)
	{
		float delta = GetDeltaTime60();
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		DecomposeMatrixToComponents(GetMatrixFloat(), matrixTranslation, matrixRotation, matrixScale);

		matrixRotation[0] += xyz.x * delta;
		matrixRotation[1] += xyz.y * delta;
		matrixRotation[2] += xyz.z * delta;

		RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, GetMatrixFloat());
		return SharedThis();
	}

	//! @brief 軸回転量を取得する
	//! @details 変数としては存在しないため代入は不可
	//! @return xyzローテート量
	const float3 GetRotationAxisXYZ(bool local = true)
	{
		matrix mat = GetMatrix();
		if(!local)
			mat = GetWorldMatrix();

		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		DecomposeMatrixToComponents(mat.f32_128_0, matrixTranslation, matrixRotation, matrixScale);

		return *(float3*)matrixRotation;
	}

	//! @brief  スケールの設定
	//! @param xyz スケール
	//! @return 自分のSharedPtr
	auto SetScaleAxisXYZ(float3 scale)
	{
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		DecomposeMatrixToComponents(GetMatrixFloat(), matrixTranslation, matrixRotation, matrixScale);

		matrixScale[0] = scale.x;
		matrixScale[1] = scale.y;
		matrixScale[2] = scale.z;

		RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, GetMatrixFloat());
		return SharedThis();
	}

	//! @brief  現在のスケールから掛け合わせ倍率スケール
	//! @param xyz スケール
	//! @return 自分のSharedPtr
	auto MulScaleAxisXYZ(float3 scale)
	{
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		DecomposeMatrixToComponents(GetMatrixFloat(), matrixTranslation, matrixRotation, matrixScale);

		matrixScale[0] *= scale.x;
		matrixScale[1] *= scale.y;
		matrixScale[2] *= scale.z;

		RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, GetMatrixFloat());
		return SharedThis();
	}

	//! @brief スケールの取得
	//! @details 変数としては存在しないため代入は不可
	//! @return スケール値
	const float3 GetScaleAxisXYZ(bool local = true)
	{
		matrix mat = GetMatrix();
		if(!local)
			mat = GetWorldMatrix();

		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		DecomposeMatrixToComponents(mat.f32_128_0, matrixTranslation, matrixRotation, matrixScale);

		return *(float3*)matrixScale;
	}

	//! @brief 向いている角度をもとめます
	//! @param target_position ターゲットのポジション
	//! @return 前方向との角度
	float GetDegreeToPosition(float3 target_position)
	{
		auto pos = GetWorldMatrix().translate();
		return GetDegreeToVector(target_position - pos);
	}

	//! @brief 向いている角度をもとめます
	//! @param target_vector ターゲットの方向
	//! @return 前方向との角度
	float GetDegreeToVector(float3 target_vector)
	{
		auto  L1  = cast(-(GetWorldMatrix().axisZ()));
		auto  L2  = cast(target_vector);
		float deg = VRad(L1, L2) * (180.0f / DX_PI_F);

		return deg;
	}
};

USING_PTR(ComponentTransform);

//! トランスフォーム
class ComponentTransform : public Component, public IMatrix<ComponentTransform>
{
	friend class Object;

public:
	BP_COMPONENT_DECL(ComponentTransform, u8"Transform機能クラス");

	ComponentTransform();

	virtual void Init() override;

	virtual void PreUpdate() override
	{
		world_transform_enable_ = false;
	}

	virtual void PrePhysics() override;
	virtual void PostUpdate() override;
	virtual void GUI() override;	//!< GUI処理

	//---------------------------------------------------------------------------
	//! @name IMatrixインターフェースの利用するための定義
	//---------------------------------------------------------------------------
	//@{

	matrix& Matrix() override
	{
		return transform_;
	}

	const matrix& GetMatrix() const override
	{
		return transform_;
	}

	ComponentTransformPtr SharedThis() override
	{
		return std::dynamic_pointer_cast<ComponentTransform>(shared_from_this());
	}

	//! @brief ワールドMatrixの取得
	//! @return 他のコンポーネントも含めた位置
	virtual const matrix GetWorldMatrix() const override
	{
		if(world_transform_enable_)
			return world_transform_;

		return GetMatrix();
	}

	//! @brief 1フレーム前のワールドMatrixの取得
	//! @return 他のコンポーネントも含めた位置
	virtual const matrix GetOldWorldMatrix() const override
	{
		return old_transform_;
	}

	//! @brief 最終ワールドMatrixの設定
	void SetWorldMatrix(const matrix& mat)
	{
		world_transform_enable_ = true;
		world_transform_		= mat;
	}

	//@}

private:
	matrix transform_;
	matrix old_transform_;					   //!< 1フレーム前の位置
	matrix world_transform_;				   //!< 最終ワールドマトリクス
	bool   world_transform_enable_ = false;	   //!< 最終ワールドマトリクスが有効

	bool				is_guizmo_		 = false;				   //!< ギズモ使用
	ImGuizmo::OPERATION gizmo_operation_ = ImGuizmo::TRANSLATE;	   //!< Gizmo処理選択
	ImGuizmo::MODE		gizmo_mode_		 = ImGuizmo::LOCAL;		   //!< Gizmo Local/Global設定
	static Component*	select_component_;						   //!< どのTransformが処理されているか

private:
	//--------------------------------------------------------------------
	//! @name Cereal処理
	//--------------------------------------------------------------------
	//@{
	CEREAL_SAVELOAD(arc, ver)
	{
		arc(cereal::make_nvp("owner", owner_));
		arc(cereal::make_nvp("transform", transform_));
		arc(cereal::make_nvp("old_transform", old_transform_));
		if(ver >= 1)
		{
			arc(cereal::make_nvp("world_transform_enable", world_transform_enable_));
			arc(cereal::make_nvp("world_transform", world_transform_));
		}
	}

	//@}
};
CEREAL_CLASS_VERSION(ComponentTransform, 1);

CEREAL_REGISTER_TYPE(ComponentTransform)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentTransform)
