//---------------------------------------------------------------------------
//! @file   ComponentCollisionLine.h
//! @brief  コリジョンコンポーネント
//---------------------------------------------------------------------------
#pragma once

#include <System/Component/ComponentCollision.h>
#include <System/Component/ComponentTransform.h>
#include <ImGuizmo/ImGuizmo.h>

USING_PTR(ComponentCollisionLine);

//! @brief コリジョンコンポーネントクラス
class ComponentCollisionLine : public ComponentCollision, public IMatrix<ComponentCollisionLine>
{
public:
    BP_COMPONENT_DECL(ComponentCollisionLine, u8"SphereCollision機能クラス");

    ComponentCollisionLine() { collision_type_ = CollisionType::LINE; }
    ComponentCollisionLinePtr SetName(const std::string_view& name) { return Component::SetName<ComponentCollisionLine>(name); }

    virtual void Init() override;          //!< 初期化
    virtual void Update() override;        //!< 更新
    virtual void PostUpdate() override;    //!< 更新後の処理
    virtual void Draw() override;          //!< 描画
    virtual void Exit() override;          //!< 終了

    virtual void GUI() override;    //!< GUI

    //! @brief ローカル座標系でのラインの設定
    //! @return 自分のSharedPtr
    ComponentCollisionLinePtr SetLine(float3 start, float3 end);

    //! @brief ローカル座標系でのラインの取得
    //! @return {開始, 終了} (std::array<float3,2>)
    std::array<float3, 2> GetLine() const;

    //! @brief ワールド座標系でのラインの取得
    //! @return {開始, 終了} (std::array<float3,2>)
    std::array<float3, 2> GetWorldLine() const;

    //! @brief 当たっているかを調べる
    //! @param col 相手のコリジョン
    //! @return HitInfoを返す
    HitInfo IsHit(ComponentCollisionPtr col) override;

    //----------------------------------------------------------------------
    //! @name IMatrixインターフェースの利用するための定義
    //----------------------------------------------------------------------
    //@{
    //! @brief TransformのMatrix情報を取得します
    //! @return ComponentTransform の Matrix
    matrix& Matrix() override { return collision_transform_; }

    const matrix& GetMatrix() const override { return collision_transform_; }

    virtual ComponentCollisionLinePtr SharedThis() override { return std::dynamic_pointer_cast<ComponentCollisionLine>(shared_from_this()); }

    //! @brief ワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetWorldMatrix() const override;

    //! @brief 1フレーム前のワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetOldWorldMatrix() const override { return old_transform_; }

    //@}

#if 1        // CompoentCollisionからの移行
    #if 0    // 通常Componentへ移動
	inline ComponentCollisionLinePtr SetName( std::string_view name )
	{
		name_ = name;
		return std::dynamic_pointer_cast<ComponentCollisionLine>( shared_from_this() );
	}
    #endif
    inline ComponentCollisionLinePtr SetHitCollisionGroup(u32 hit_group)
    {
        collision_hit_ = hit_group;
        return std::dynamic_pointer_cast<ComponentCollisionLine>(shared_from_this());
    }

    inline ComponentCollisionLinePtr SetOverlapCollisionGroup(u32 overlap_group)
    {
        collision_overlap_ = overlap_group;
        return std::dynamic_pointer_cast<ComponentCollisionLine>(shared_from_this());
    }

    inline ComponentCollisionLinePtr SetCollisionGroup(CollisionGroup grp)
    {
        collision_group_ = grp;
        return std::dynamic_pointer_cast<ComponentCollisionLine>(shared_from_this());
    }

    inline ComponentCollisionLinePtr SetMass(float mass)
    {
        collision_mass_ = mass;
        return std::dynamic_pointer_cast<ComponentCollisionLine>(shared_from_this());
    }
#endif

protected:
#ifdef USE_JOLT_PHYSICS
    bool set_size_ = false;
#endif    //USE_JOLT_PHYSICS
    float line_scale_ = 1.0f;

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(cereal::make_nvp("owner", owner_));
        arc(cereal::make_nvp("line", line_scale_));
        arc(cereal::make_nvp("ComponentCollision", cereal::base_class<ComponentCollision>(this)));
    }

    //@}
};

CEREAL_REGISTER_TYPE(ComponentCollisionLine)
CEREAL_REGISTER_POLYMORPHIC_RELATION(ComponentCollision, ComponentCollisionLine)
