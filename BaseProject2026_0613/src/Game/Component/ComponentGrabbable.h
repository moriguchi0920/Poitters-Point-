#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>

USING_PTR(ComponentGrabbable);

class ComponentGrabbable : public Component
{
public:
    BP_COMPONENT_DECL(ComponentGrabbable, u8"掴み可能コンポーネント");

    void Init() override;

    void Update() override;

    void GUI() override;

    void          SetLiftTime(float time);
    const float&  GetLiftTime() const;
    void          SetTranslation(float3 new_translation);
    void          AddTranslation(float3 add_translation);
    const float3& GetTranslation();
    void          SetIsGrabbed(bool grabbed);
    bool          GetIsGrabbed();
    bool          GetCanGrab();
    void          SetCanGrab(const bool can_grab);
    void          SetBounceOffset(float offset);
    void          Bounce();
    bool          IsGrounded();
    bool          IsMoving();
    //! @brief ダメージ値を設定(投げ物ごとに異なるダメージを持たせる)
    void SetDamage(float damage);
    //! @brief 投げ主か判定
    bool IsThrower(ObjectPtr target) const;

private:
    float lift_time_;
    float bounce_offset_;
    bool  is_grabbed_;
    bool  can_grab_this_;
    float  damage_;    //!< このオブジェクトのダメージ値

    float3 translation_;

    ObjectWeakPtr thrower_;    //!< 投げた本人（ダメージスキップ用）

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
};

CEREAL_CLASS_VERSION(ComponentGrabbable, 1);
