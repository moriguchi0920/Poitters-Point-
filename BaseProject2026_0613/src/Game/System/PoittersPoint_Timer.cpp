//---------------------------------------------------------------------------
//! @file   PoittersPoint_Title.cpp
//! @brief  自作チュートリアルシーンXのタイトル
//---------------------------------------------------------------------------
// システム
#include "PoittersPoint_Timer.h"


namespace PoittersPoint {


    // 制限時間の上限(2分)
    float PoittersPoint_Timer::max_time_ = 120.0f;

    //! @brief 初期化
    //! @return 初期化済み
    bool PoittersPoint_Timer::Init()
    {
        // 最初に1回動作する
        // ただし trueを返さなければ Initに何回も来る仕様。

        time_ = max_time_;    // 制限時間の初期化

        return true;
    }

    void PoittersPoint_Timer::Update()
    {
        time_ -= GetDeltaTime();
        if(time_ < 0.0f) {
            time_ = 0.0f;
        }
        
    }
    float PoittersPoint_Timer::GetTimer() const
    {
        return time_;
    }

    void PoittersPoint_Timer::GUI()
    {
       
    }

}    // namespace PoittersPoint
