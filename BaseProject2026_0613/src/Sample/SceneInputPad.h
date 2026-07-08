//---------------------------------------------------------------------------
//! @file   SceneInputPad.h
//! @brief  パッド入力サンプルシーン
//---------------------------------------------------------------------------
#pragma once

#include <System/Scene.h>

//===========================================================================
//! パッド入力サンプルシーン
//===========================================================================
class SceneInputPad : public Scene::Base
{
public:
    BP_CLASS_DECL(SceneInputPad, u8"パッド入力");

    bool Init() override;      //!< 初期化
    void Update() override;    //!< 更新
    void Draw() override;      //!< 描画
    void Exit() override;      //!< 終了
    void GUI() override;       //!< GUI表示

private:
};
