#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>

USING_PTR(ComponentPlayerState);

class ComponentKnockBack : public Component
{
public:
    BP_COMPONENT_DECL(ComponentKnockBack, u8"ノックバックコンポーネント");

    void Init() override;
    void Update() override;
    //! @brief GUI
    //! GUIでのノックバックの確認や変更に使用
    void GUI() override;

    // ノックバック用：OnHitでここに速度を入れ、Updateで減衰しながら適用する
    float3 knockback_velocity = {0.0f, 0.0f, 0.0f};

    // 外部からノックバックを与える
    void ApplyKnockback(const float3& velocity);

    // 当たり通知を受け取って内部でノックバックを計算・適用する
    void OnHitComponent(const HitInfo& hit_info) override;

    // チューニング用パラメータ（GUIで変更可能）
    float damping          = 0.12f;    // 0..1 に近いほど早く減衰（lerp 係数）
    float velocity_epsilon = 0.01f;    // これ以下ならゼロとみなす
    float min_upward       = 5.0f;     // Apply 時に最低限保証する上向き成分

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
};

CEREAL_CLASS_VERSION(ComponentKnockBack, 1);
