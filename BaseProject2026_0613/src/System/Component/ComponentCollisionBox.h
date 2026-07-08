//---------------------------------------------------------------------------
//! @file   ComponentCollisionBox.h
//! @brief  コリジョンコンポーネント(ボックス)
//---------------------------------------------------------------------------
#pragma once
#include <System/Component/ComponentCollision.h>
#include <System/Component/ComponentTransform.h>
#include <ImGuizmo/ImGuizmo.h>

USING_PTR(ComponentCollisionBox);

//! @brief コリジョンコンポーネントクラス
class ComponentCollisionBox : public ComponentCollision, public IMatrix<ComponentCollisionBox>
{
public:
    BP_COMPONENT_DECL(ComponentCollisionBox, u8"BoxCollision機能クラス");

    ComponentCollisionBox() { collision_type_ = CollisionType::BOX; }
    ComponentCollisionBoxPtr SetName(const std::string_view& name) { return Component::SetName<ComponentCollisionBox>(name); }

    virtual void Init() override;
    virtual void InitSerialize() override;
    virtual void Update() override;
    virtual void PostUpdate() override;
    virtual void Draw() override;
    virtual void Exit() override;

    virtual void GUI() override;    //!< GUI

    //! @brief サイズの設定
    //! @param size サイズ
    ComponentCollisionBoxPtr SetSize(float3 size);

    //! @brief サイズの取得
    //! @return サイズ
    float3 GetSize() const;

    HitInfo IsHit(ComponentCollisionPtr col) override;

    //----------------------------------------------------------------------
    //! @name IMatrixインターフェースの利用するための定義
    //----------------------------------------------------------------------
    //@{
    //! @brief TransformのMatrix情報を取得します
    //! @return ComponentTransform の Matrix
    matrix& Matrix() override { return collision_transform_; }

    const matrix& GetMatrix() const override { return collision_transform_; }

    virtual ComponentCollisionBoxPtr SharedThis() override { return std::dynamic_pointer_cast<ComponentCollisionBox>(shared_from_this()); }

    //! @brief ワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetWorldMatrix() const override;

    //! @brief 1フレーム前のワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetOldWorldMatrix() const override { return old_transform_; }

    //@}

    inline ComponentCollisionBoxPtr SetHitCollisionGroup(u32 hit_group)
    {
        collision_hit_ = hit_group;
        return std::dynamic_pointer_cast<ComponentCollisionBox>(shared_from_this());
    }

    inline ComponentCollisionBoxPtr SetOverlapCollisionGroup(u32 overlap_group)
    {
        collision_overlap_ = overlap_group;
        return std::dynamic_pointer_cast<ComponentCollisionBox>(shared_from_this());
    }

    inline ComponentCollisionBoxPtr SetCollisionGroup(CollisionGroup grp)
    {
        collision_group_ = grp;
        return std::dynamic_pointer_cast<ComponentCollisionBox>(shared_from_this());
    }

    inline ComponentCollisionBoxPtr SetMass(float mass)
    {
        collision_mass_ = mass;
        return std::dynamic_pointer_cast<ComponentCollisionBox>(shared_from_this());
    }

    static int GetSharedBoxModel() { return shared_box_model_; }

protected:
    float3 size_ = {1.0f, 1.0f, 1.0f};    //!< サイズ

    static int shared_box_model_;    //!< 全インスタンスで共有する1x1x1のBoxモデル

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(cereal::make_nvp("owner", owner_));
        arc(cereal::make_nvp("size", size_));
        arc(cereal::make_nvp("ComponentCollision", cereal::base_class<ComponentCollision>(this)));
    }

    //@}
};

CEREAL_REGISTER_TYPE(ComponentCollisionBox)
CEREAL_REGISTER_POLYMORPHIC_RELATION(ComponentCollision, ComponentCollisionBox)
