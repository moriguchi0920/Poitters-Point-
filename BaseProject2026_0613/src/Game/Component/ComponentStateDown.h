#pragma once
//---------------------------------------------------------------------------
//! @file   ComponentStateDown.h
//! @brief  ダウン状態コンポーネント
//! @details 投げ物に当たって吹き飛んだ時・他人に掴まれている時の状態。
//!          着地してから一定時間倒れたままになり、起き上がると終了(GetIsFinished)。
//!          操作不能(ComponentCharacterPhysics::SetControlEnabled(false))の管理もここで行う。
//---------------------------------------------------------------------------
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/ComponentState.h>

USING_PTR(ComponentStateDown);

class ComponentStateDown : public ComponentState
{
public:
    BP_COMPONENT_DECL(ComponentStateDown, u8"ダウン状態コンポーネント");

    void Init() override;
    void Update() override;
    void Exit() override;
    void GUI() override;

    //! @brief 掴まれている間は倒れたまま時間を進めない
    void SetHeld(bool held) { is_held_ = held; }

    //! @brief もう一度吹き飛んだ(ダウン時間をやり直す)
    void Restart();

    //! @brief すぐに起き上がる(持ち上げが取り消された時など)
    void RecoverSoon();

    bool GetIsFinished() const { return finished_; }

    //! @brief 脱落(HP 0)：倒れたまま起き上がらない
    void SetEliminated() { is_eliminated_ = true; }

private:
    //! @brief モデルを倒す角度(度)を設定する
    void ApplyTilt(float degree);

    bool  is_held_       = false;
    bool  is_eliminated_ = false;    //!< 脱落して起き上がらない
    bool  finished_      = false;
    float air_time_      = 0.0f;    //!< 吹き飛んでからの時間
    float lying_time_    = 0.0f;    //!< 着地して倒れている時間
    float tilt_          = 0.0f;    //!< 現在のモデルの傾き(度)
    bool  has_base_      = false;
    matrix base_matrix_  = matrix::identity();    //!< 倒す前のモデル行列

    float down_time_     = 1.2f;      //!< 着地後に倒れている時間
    float tilt_speed_    = 450.0f;    //!< 倒れる/起き上がる速さ(度/秒)
    float max_air_time_  = 3.0f;      //!< 着地しなくてもこの時間で倒れ扱いにする(安全策)

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
    //@}
};

CEREAL_CLASS_VERSION(ComponentStateDown, 1);
