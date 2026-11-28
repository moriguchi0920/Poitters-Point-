#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/ComponentState.h>

USING_PTR(ComponentStateDead);

//! @brief 死亡状態コンポーネント
//!
//! 死亡演出が終わったら以下の死亡処理を行い、自身を削除する
//!  - 物理ボディ(ComponentCharacterPhysics / ComponentPhysics)を外し、当たり判定を消す
//!
//! 死亡演出は2種類
//!  - モデルに"dead"アニメーションがある場合(Enemy) : アニメーションを再生し、終了を待つ
//!  - "dead"アニメーションが無い場合(Player)        : SetUseAnimation(false)を指定すると、
//!                                                     モデルをその場で後ろへ倒して演出する
class ComponentStateDead : public ComponentState
{
public:
    BP_COMPONENT_DECL(ComponentStateDead, u8"死亡状態コンポーネント");

    void Init() override;

    void Update() override;

    void GUI() override;

    //! @brief "dead"アニメーションを使うか(AddComponent直後に設定する)
    //! @param use false: アニメーションを使わずにモデルを倒して演出する
    ComponentStateDead* SetUseAnimation(bool use)
    {
        use_animation_ = use;
        return this;
    }

private:
    //! @brief 死亡処理(演出終了時に1回だけ呼ぶ)
    void FinishDead();

    bool  use_animation_ = true;     //!< "dead"アニメーションで演出するか
    float timer_         = 0.0f;     //!< 倒れ演出の経過時間(秒)
    float fall_time_     = 0.6f;     //!< 倒れきるまでの時間(秒)
    float wait_time_     = 0.4f;     //!< 倒れた後、死亡処理までの待ち時間(秒)
    float fall_angle_    = 90.0f;    //!< 倒れる角度(度)
    float3 start_rot_    = {0, 0, 0};    //!< 倒れ始めのモデルの回転(度)

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
};

CEREAL_CLASS_VERSION(ComponentStateDead, 1);
