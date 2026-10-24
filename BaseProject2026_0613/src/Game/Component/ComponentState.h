#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>

USING_PTR(ComponentState);

class ComponentState : public Component
{
public:
    BP_COMPONENT_DECL(ComponentState, u8"オブジェクト状態表現基底");

    void Init() override { __super::Init(); }

    void Update() override { __super::Update(); }

    void GUI() override
    {
        __super::GUI();
        /*
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
        */
    }

    //! @brief 現在のステートを取得する
    const std::string GetStateName() const { return GetName().data(); }
    // 関数の後置constはGet~関数やIs~のような数値を取得するだけの関数には必ずつける

    //! @brief 次のステートにする
    template <class T>
    T* ChangeState()
    {
        static_assert(std::derived_from<T, ComponentState>, "ステートクラス以外のクラスは指定できません");
        RemoveThisComponent();
        return GetOwner()->AddComponent<T>().get();
    }

    //! @brief アニメーションが終わったら次のステートにする
    //! @return
    template <class T>
    void ChangeStateByAnimEnd()
    {
        static_assert(std::derived_from<T, ComponentState>, "ステートクラス以外のクラスは指定できません");
        if(auto model = GetOwner()->GetComponent<ComponentModel>()) {
            if(!model->IsPlaying()) {
                ChangeState<T>();
            }
        }
    }

    const float GetAnimationTime() const
    {
        if(auto model = GetOwner()->GetComponent<ComponentModel>()) {
            return model->GetAnimationTime();
        }
        return 0.0f;
    }

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
};
