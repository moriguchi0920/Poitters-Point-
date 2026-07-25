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

       void GUI();

       // 現在の残り時間を取得する
       float GetTimer() const;

    private:
       float time_ = 0.0f;
       static float max_time_;

    };

}    // namespace PoittersPoint
