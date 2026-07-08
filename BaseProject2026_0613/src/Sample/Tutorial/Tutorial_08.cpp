#include <System/Scene.h>
#include <System/Utils/IniFileLib.h>

namespace Tutorial {
//-------------------------------------------------------
class Tutorial_08 : public Scene::Base
{
public:
    BP_CLASS_DECL(Tutorial_08, u8"(8)Tutorial iniファイルの使い方");

    //! @brief 初期化
    //! @return 初期化が終わったか
    bool Init() override;

    //! @brief 更新
    //! @param delta 更新周期
    void Update() override;

    //! @brief 描画
    void Draw() override;

    //! @brief デバッグ表示用GUI
    void GUI() override;

    //! @brief 終了
    void Exit() override;

private:
};

//------------------------------------------------------------------
class Object08 : public Object
{
public:
    bool Init() override
    {
        __super::Init();
        Load();

        return true;
    }

    void Update() override { __super::Update(); }

    void GUI() override
    {
        __super::GUI();

        ImGui::Begin("Object");
        {
            if(ImGui::Button("Reload"))
                Load();

            ImGui::DragInt("HP", &hit_point_);
            ImGui::DragFloat("value", &value_, 0.1f);
            ImGui::DragFloat3("point", (float*)&point_, 0.1f);
        }
        ImGui::End();
    }

    void Load()
    {
        IniFileLib ini("Tutorial.ini");
        hit_point_ = ini.GetInt("Player", "HP");
        point_     = ini.GetFloat3("Player", "point");
    }

private:
    int    hit_point_ = 0;
    float  value_     = 0.0f;
    float3 point_     = {0, 0, 0};
};

bool Tutorial_08::Init()
{
    Scene::Object::Create<Object08>()->SetName("Obj");

    return true;
}

void Tutorial_08::Update()
{
}

void Tutorial_08::Draw()
{
}

void Tutorial_08::GUI()
{
    // HowToUseというGUIWindowを以下の内容で表示する
    ImGui::Begin("HowToUse");
    {
    }
    ImGui::End();
}

void Tutorial_08::Exit()
{
}

}    // namespace Tutorial
