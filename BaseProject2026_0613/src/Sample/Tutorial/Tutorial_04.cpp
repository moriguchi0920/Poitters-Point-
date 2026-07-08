#include <System/Scene.h>
#include <System/Input/InputKey.h>

namespace Tutorial {
//-------------------------------------------------------
class Tutorial_04 : public Scene::Base
{
public:
    BP_CLASS_DECL(Tutorial_04, u8"(4)Tutorial マウス入力の使用方法");

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
    struct MouseCount
    {
        int count_up   = 0;    //!< IsKeyUpが有効になった数
        int count_down = 0;    //!< IsKeyDownが有効になった数
        int count      = 0;    //!< IsKeyが有効になった数

        int count_on      = 0;    //!< IsKeyOnが有効になった数
        int count_release = 0;    //!< IsKeyReleaseが有効になった数
        int count_repeat  = 0;    //!< IsKeyRepeatが有効になった数
    };

    MouseCount mouse_[2];    //!< 左右ボタンで用意する
};

//------------------------------------------------------------------
bool Tutorial_04::Init()
{
    return true;
}

void Tutorial_04::Update()
{
    std::vector button = {MOUSE_INPUT_LEFT, MOUSE_INPUT_RIGHT};

    for(int i = 0; i < button.size(); ++i) {
        if(IsMouseDown(button[i])) {
            mouse_[i].count_down++;
        }
        if(IsMouseUp(button[i])) {
            mouse_[i].count_up++;
        }
        if(IsMouse(button[i])) {
            mouse_[i].count++;
        }

        if(IsMouseOn(button[i])) {
            mouse_[i].count_on++;
        }
        if(IsMouseRelease(button[i])) {
            mouse_[i].count_release++;
        }
        if(IsMouseRepeat(button[i])) {
            mouse_[i].count_repeat++;
        }
    }
}

void Tutorial_04::Draw()
{
    int1 line = 3;

    std::vector button = {"MOUSE_INPUT_LEFT", "MOUSE_INPUT_RIGHT"};
    const auto  col    = GetColor(255, 255, 255);
    for(int i = 0; i < button.size(); ++i) {
        int3 ofs = {10 + i * 400, 0, 0};
        line     = 3;
        DrawFormatString(ofs.x, ofs.y + ((line++) * 32), GetColor(0, 255, 255), "%s\n", button[i]);
        DrawFormatString(ofs.x, ofs.y + ((line++) * 32), col, "IsMouseDown   : %4d", mouse_[i].count_down);
        DrawFormatString(ofs.x, ofs.y + ((line++) * 32), col, "IsMouseUp     : %4d", mouse_[i].count_up);
        DrawFormatString(ofs.x, ofs.y + ((line++) * 32), col, "IsMouse       : %4d", mouse_[i].count);

        DrawFormatString(ofs.x, ofs.y + ((line++) * 32), col, "IsMouseOn     : %4d", mouse_[i].count_down);
        DrawFormatString(ofs.x, ofs.y + ((line++) * 32), col, "IsMouseRelease: %4d", mouse_[i].count_release);
        DrawFormatString(ofs.x, ofs.y + ((line++) * 32), col, "IsMouseRepeat : %4d", mouse_[i].count_repeat);
    }
}

void Tutorial_04::GUI()
{
    // HowToUseというGUIWindowを以下の内容で表示する
    ImGui::Begin("HowToUse");
    {
        ImGui::Separator();
        ImGui::Text("MouseInputの使用方法\n "    //
                    u8"\n"                       //
                    u8"IsMouse( 調べたいボタン )       :  押しているか判定できる\n"
                    u8"IsMouseDown( 調べたいボタン )   :  現在のフレームで押したか判定できる\n"
                    u8"IsMousUp( 調べたいボタン )      :  現在のフレームで離したか判定できる\n"
                    u8"\n"
                    u8"IsMouseOn( 調べたいボタン )     :  現在のフレームで押したか判定できる【IsMouseDown()と同じ】\n"
                    u8"IsMouseRelase( 調べたいボタン ) :  押してないかどうか判定できる      【!IsMouse()と同じ】\n"
                    u8"IsMouseRepeat( 調べたいボタン ) :  2フレーム以上押したか判定できる\n"
                    u8"\n"
                    u8"調べたいボタン:\n"
                    u8"   MOUSE_INPUT_LEFT   : 左ボタン\n"
                    u8"   MOUSE_INPUT_RIGHT  : 右ボタン\n"
                    u8"   MOUSE_INPUT_CENTER : 中ボタン\n"
                    u8" など (DxLib.hに書かれています)\n");
    }
    ImGui::End();
}

void Tutorial_04::Exit()
{
}

}    // namespace Tutorial
