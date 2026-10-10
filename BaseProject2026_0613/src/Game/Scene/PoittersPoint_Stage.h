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

    // ★ スライム再生成（リスポーン）用変数
    float                  slime_respawn_timer_ = 0.0f;    //!< 再生成カウントダウンタイマー
    static constexpr float SLIME_RESPAWN_TIME   = 5.0f;    //!< 消滅してから再生成されるまでの秒数
};

}    // namespace PoittersPoint
