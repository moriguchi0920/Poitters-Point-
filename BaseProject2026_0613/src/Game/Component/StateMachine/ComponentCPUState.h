#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>
#include <Game/Component/StateMachine/ComponentStateMachine.h>

USING_PTR(ComponentCPUState);

class ComponentCPUState : public ComponentStateMachine
{
public:
    BP_COMPONENT_DECL(ComponentCPUState, u8"CPU状態制御コンポーネント");

    void Init() override;

    void Update() override;

    void GUI() override;

private:
    // タイマー完成後に置き換える仮タイマー
    float       time_count_;
    const float escape_offset_ = 20.0f;

    // CPUの隙である思考時間であるかどうか
    bool is_thinking_;

    // CPUの行動定数
    enum CPU_ACTION
    {
        ACTION_DEFAULT,
        // ものを掴みに行く動き
        ACTION_GRAB,
        // ものを持っている相手を避ける動き
        ACTION_AVOID_ATTACKER,
        // ものを持っている場合、相手に投げに行く動き
        ACTION_ATTACK,
        ACTION_NUM
    };

    int cur_action_;

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
};

CEREAL_CLASS_VERSION(ComponentCPUState, 1);
