#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Stage.h
//! @brief  PoittersPointのステージシーンのヘッダ
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace PoittersPoint {

//! シーンクラス
class PoittersPoint_Stage : public Scene::Base
{
    static constexpr int MAX_ENEMIES = 5;

public:
    BP_CLASS_DECL(PoittersPoint_Stage, u8"(stage)ポイッターズポイント ステージシーン");

    //! @brief 初期化
    bool Init() override;

    //! @brief 更新
    void Update() override;

    //! @brief GUI表示
    void GUI() override;

    void AddDeadEnemy();

private:
    void createEnemy();

    int enemy_dead_count = 0;

    int counter  = 0;
    int counter2 = 1000;
    int counter3 = 2000;
};

}    // namespace PoittersPoint
