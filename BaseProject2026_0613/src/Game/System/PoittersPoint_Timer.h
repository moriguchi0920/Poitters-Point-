#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint.h
//! @brief  PoittersPointのタイトルのヘッダファイル
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace PoittersPoint {
//! システムクラス
class PoittersPoint_Timer
{
public:
    //publicでこの記述をクラスに入れておけばGUIでオブジェクト生成が可能になる
    BP_CLASS_DECL(PoittersPoint_Timer, u8"(Timer)ポイッターズポイント");
    //! @brief 初期化
    //! @return 初期化済み
    bool Init();

    void Update();

    //! @brief 時間の加算
    //! @param sec
    void AddTime(float sec);

    //! @brief 時間の減算
    //! @param sec
    void SubTime(float sec);

    //! @brief タイマーの状態設定
    void SetPause(bool is_pause);

    // 現在の残り時間をセット
    void SetTime(float time);

    // 制限時間の上限を設定
    void SetMaxTime(float max_time);

    //! @brief 停止中かどうか
    //! @return
    bool IsPaused();

    //! @brief タイムアップしたかどうか
    //! @return
    bool IsTimeUp();

    // 現在の残り時間を取得する
    float GetTimer();

    void GUI();

private:
    // 現在のタイムを保持する変数
    static float time_;
    // 制限時間の上限
    static float max_time_;

    static bool is_paused_;
};

}    // namespace PoittersPoint
