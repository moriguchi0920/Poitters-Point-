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

    // ノックバック用：OnHitでここに速度を入れ、UpdateでXZは減衰・Yは重力積分しながら適用する
    float3 knockback_velocity = {0.0f, 0.0f, 0.0f};

    // 外部からノックバックを与える
    void ApplyKnockback(const float3& velocity);

    // 当たり通知を受け取って内部でノックバックを計算・適用する
    void OnHitComponent(const HitInfo& hit_info) override;

    // チューニング用パラメータ（GUIで変更可能）
    float damping          = 0.12f;    // XZ の地面減衰（0..1 有効範囲。外は clamp され、負値は速度を増幅させ NaN の原因になる）
    float velocity_epsilon = 0.01f;    // これ以下ならゼロとみなす
    float min_upward       = 5.0f;     // Apply 時に最低限保証する上向き成分
    float arch_strength    = 0.25f;    // 水平速度に対する上向きの割合（アーチの大きさ）
    float arch_gravity     = 0.25f;    // 滞空中に Y 速度へ毎フレーム加算する下向き加速度
    float max_fall_speed   = 5.0f;    // 落下速度の上限
    float max_knockback_speed = 15.0f; // 速度（水平）の上限。オーバーフローで Transform が NaN になるのを防ぐ
    float away_weight      = 0.7f;     // 吹き飛ぶ方向の重み（1=当たり元から真後ろへ、0=投げ物の進行方向）

    // 滞空状態（アーチを描く間は true、打ち出し高さまで落ちたら着地）
    bool  knockback_airborne = false;
    float launch_y           = 0.0f;    // 打ち出し位置の Y（着地判定の基準）
    int   air_frames         = 0;       // 滞空フレーム数（安全打ち切り用）
    int   active_frames      = 0;       // ノックバック適用中フレーム数（安全打ち切り用）

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
