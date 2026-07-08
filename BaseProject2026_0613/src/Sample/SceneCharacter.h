//---------------------------------------------------------------------------
//! @file   SceneCharacter.h
//! @brief  キャラクター操作サンプルシーン
//---------------------------------------------------------------------------
#pragma once

#include <System/Scene.h>

namespace physics {
class Character;
class RigidBody;
}    // namespace physics

//===========================================================================
//! キャラクター操作サンプルシーン
//===========================================================================
class SceneCharacter final : public Scene::Base
{
public:
    BP_CLASS_DECL(SceneCharacter, u8"キャラクター操作サンプル");

    virtual bool Init() override;      // 初期化
    virtual void Update() override;    // 更新
    virtual void Draw() override;      // 描画
    virtual void Exit() override;      // 終了
    virtual void GUI() override;       // GUI表示

private:
    std::shared_ptr<Model>              model_env_;    //!< 背景モデル
    std::shared_ptr<physics::RigidBody> body_mesh_;    //!< 床

    std::shared_ptr<physics::Character> character_;    //!< キャラクターコントローラー
    std::shared_ptr<physics::RigidBody> body_env_;     //!< 地形衝突情報
};
