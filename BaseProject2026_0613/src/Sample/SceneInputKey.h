//---------------------------------------------------------------------------
//! @file   SceneInputKey.h
//! @brief  サンプルシーン
//---------------------------------------------------------------------------
#pragma once

#include <System/Scene.h>

//===========================================================================
//! サンプルシーン
//===========================================================================
class SceneInputKey final : public Scene::Base
{
public:
    BP_CLASS_DECL(SceneInputKey, u8"キー入力とEffekseer");

    bool Init() override;      //!< 初期化
    void Update() override;    //!< 更新
    void Draw() override;      //!< 描画
    void Exit() override;      //!< 終了
    void GUI() override;       //!< GUI表示

private:
};
