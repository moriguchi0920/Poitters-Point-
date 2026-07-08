//---------------------------------------------------------------------------
//! @file   SceneSandbox.h
//! @brief  サンプルシーン
//---------------------------------------------------------------------------
#pragma once

#include <System/Scene.h>

//===========================================================================
//! サンプルシーン
//===========================================================================
class SceneSandbox final : public Scene::Base
{
public:
    BP_CLASS_DECL(SceneSandbox, u8"自作サンプル");

    bool Init() override;      //!< 初期化
    void Update() override;    //!< 更新
    void Draw() override;      //!< 描画
    void Exit() override;      //!< 終了
    void GUI() override;       //!< GUI表示

private:
    int model_ = -1;    //!< MV1モデルハンドル
};
