#pragma once
#include <System/Scene.h>
#include "ComponentState.h"

USING_PTR(ComponentStateThink);

class ComponentStateThink : public ComponentState
{
public:
    BP_COMPONENT_DECL(ComponentStateThink, u8"思考状態");

    void Init() override;

    void Update() override;

    void GUI() override
    {
        __super::GUI();

        // GUI内に出現させる
        ImGui::Begin(GetOwner()->GetName().data());
        {
            ImGui::Separator();
            if(ImGui::TreeNode(GetName().data())) {
                // 有効/無効
                bool enable = GetStatus(StatusBit::Enable);
                if(ImGui::Checkbox(u8"有効", &enable))
                    SetStatus(StatusBit::Enable, enable);

                // GUI上でオーナーから自分を削除します
                if(ImGui::Button(u8"削除"))
                    GetOwner()->RemoveComponent(shared_from_this());

                ImGui::TreePop();
            }
        }
        ImGui::End();
    }

    void SetThinkingFinishTime(float time);

    float GetCurThinkingTime();

    bool GetFinished();

private:
    float thinking_time;
    float thinking_finish_time;

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
};
