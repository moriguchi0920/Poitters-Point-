#pragma once
//---------------------------------------------------------------------------
//! @file   ComponentCharacterPhysics.h
//! @brief  キャラクター共通の物理移動コンポーネント
//! @details ComponentPhysics(カプセル)を作成し、移動・ジャンプ・吹き飛びをすべて
//!          剛体の速度として表現する。重力・地面・壁・他キャラ/投げ物との押し合いは Jolt が解く。
//!          ステート側は AddTranslate ではなく Move() / Jump() / Launch() を呼ぶだけでよい。
//---------------------------------------------------------------------------
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <System/Component/ComponentPhysics.h>

USING_PTR(ComponentCharacterPhysics);

class ComponentCharacterPhysics : public Component
{
public:
    BP_COMPONENT_DECL(ComponentCharacterPhysics, u8"キャラクター物理移動コンポーネント");

    void PrePhysics() override;
    void GUI() override;

    using Component::OnHitComponent;
    //! @brief 物理の接触通知(接地判定に使用)
    void OnHitComponent(const HitInfoPhysics& hit_info) override;

    //! @brief カプセルの物理ボディを作成する (Object::Init から呼ぶ)
    //! @param radius 半径
    //! @param height 全高(足元～頭)。足元がオブジェクトの原点になる
    //! @param type 衝突タイプ
    void Setup(float radius, float height, ComponentPhysics::CollisionType type = ComponentPhysics::CollisionType::PLAYER);

    //! @brief このフレームの移動要求 (ワールド方向)
    //! @param velocity60 1/60秒あたりの移動量(従来の move_speed と同じ単位)
    void Move(const float3& velocity60);

    //! @brief ジャンプ(上向きの速度を与える)
    //! @param speed 上向き初速(単位/秒)
    void Jump(float speed);

    //! @brief 吹き飛ばす(速度を直接与え、操作不能にする)
    //! @details 復帰は SetControlEnabled(true) で行う(通常は ComponentStateDown が行う)
    void Launch(const float3& velocity);

    //! @brief 操作(Move)を受け付けるか
    void SetControlEnabled(bool enable) { control_enabled_ = enable; }
    bool IsControlEnabled() const { return control_enabled_; }

    //! @brief 接地しているか(足元付近に接触がある)
    bool IsGrounded() const { return time_since_ground_ <= ground_grace_time_; }

    //! @brief 現在の速度(単位/秒)
    float3 GetVelocity() const;

    //! @brief 体が触れている相手(物理の接触)
    struct Touch
    {
        ObjectWeakPtr object_;            //!< 触れている相手
        float3        point_;             //!< 接触点(ワールド)
        float         elapsed_ = 0.0f;    //!< 最後に接触してからの時間
    };

    //! @brief 今触れている相手の一覧(物理ステップが無いフレームでも消えないよう少しの間保持する)
    const std::vector<Touch>& GetTouches() const { return touches_; }

    float GetRadius() const { return radius_; }
    float GetHeight() const { return height_; }

private:
    ComponentPhysicsPtr GetPhysics() const { return GetOwner()->GetComponent<ComponentPhysics>(); }

    float radius_ = 3.8f;
    float height_ = 22.0f;

    float3 move_velocity_   = {0.0f, 0.0f, 0.0f};    //!< 今フレームの移動要求(単位/秒)
    bool   control_enabled_ = true;                  //!< 操作可能か(吹き飛び・ダウン中は false)

    std::vector<Touch> touches_;                //!< 触れている相手
    float              touch_keep_time_ = 0.25f;    //!< 接触が途切れても触れているとみなす時間

    float time_since_ground_ = 0.0f;     //!< 最後に足元が接触してからの時間
    float ground_grace_time_ = 0.1f;     //!< 接地とみなす猶予(物理ステップが無いフレーム対策)
    float ground_damping_    = 6.0f;     //!< 操作不能時の接地中の水平減速(大きいほど早く止まる)
    float max_fall_speed_    = 400.0f;    //!< 落下速度の上限

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
    //@}
};

CEREAL_CLASS_VERSION(ComponentCharacterPhysics, 1);
