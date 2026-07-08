#include <System/Scene.h>
#include <System/Input/InputKey.h>

namespace Tutorial {

//-------------------------------------------------------
//! シーンクラス
class Tutorial_03 : public Scene::Base
{
public:
    BP_CLASS_DECL(Tutorial_03, u8"(3)Tutorial キー入力の使用方法");

    //! @brief 初期化
    //! @return 初期化終了
    bool Init() override;

    //! @brief 更新
    //! @param delta 更新時間
    void Update() override;

    //! @brief 描画
    void Draw() override;

    //! @brief デバッグ用UI
    void GUI() override;

private:
    int count_up_   = 0;
    int count_down_ = 0;
    int count_      = 0;

    int count_on_      = 0;
    int count_release_ = 0;
    int count_repeat_  = 0;
};

//------------------------------------------------------------------

bool Tutorial_03::Init()
{
    return true;
}

void Tutorial_03::Update()
{
    //---------------------------------------------
    // SPACEキーを押したときの挙動のチェック
    //---------------------------------------------
    if(IsKeyDown(KEY_INPUT_SPACE)) {
        count_down_++;
    }
    if(IsKeyUp(KEY_INPUT_SPACE)) {
        count_up_++;
    }
    if(IsKey(KEY_INPUT_SPACE)) {
        count_++;
    }

    //---------------------------------------------
    // 初期にあったキー取得バージョン
    //---------------------------------------------
    if(IsKeyOn(KEY_INPUT_SPACE)) {
        count_on_++;
    }
    if(IsKeyRelease(KEY_INPUT_SPACE)) {
        count_release_++;
    }
    if(IsKeyRepeat(KEY_INPUT_SPACE)) {
        count_repeat_++;
    }
}

void Tutorial_03::Draw()
{
    // キー情報の表示
    const int3 ofs  = {0, 0, 0};
    int1       line = 3;
    DrawFormatString(ofs.x, ofs.y + ((line++) * 32), GetColor(0, 255, 0), "KEY_INPUT_SPACE", count_down_);
    DrawFormatString(ofs.x, ofs.y + ((line++) * 32), GetColor(255, 255, 255), "IsKey        : %4d", count_);
    DrawFormatString(ofs.x, ofs.y + ((line++) * 32), GetColor(255, 255, 255), "IsKeyDown    : %4d", count_down_);
    DrawFormatString(ofs.x, ofs.y + ((line++) * 32), GetColor(255, 255, 255), "IsKeyUp      : %4d", count_up_);

    DrawFormatString(ofs.x, ofs.y + ((line++) * 32), GetColor(255, 255, 255), "IsKeyOn      : %4d", count_on_);
    DrawFormatString(ofs.x, ofs.y + ((line++) * 32), GetColor(255, 255, 255), "IsKeyRelease : %4d", count_release_);
    DrawFormatString(ofs.x, ofs.y + ((line++) * 32), GetColor(255, 255, 255), "IsKeyRepeat  : %4d", count_repeat_);
}

void Tutorial_03::GUI()
{
    // HowToUseというGUIWindowを以下の内容で表示する
    ImGui::Begin("HowToUse");
    {
        // GUIでの使用方法表示
        ImGui::Separator();
        ImGui::Text("キーInputの使用方法\n"    //
                    u8"\n"                     //
                    u8"IsKey( 調べたいキー )       :  押しているか判定できる\n"
                    u8"IsKeyDown( 調べたいキー )   :  現在のフレームで押したか判定できる\n"
                    u8"IsKeyUp( 調べたいキー )     :  現在のフレームで離したか判定できる\n"
                    u8"\n"
                    u8"IsKeyOn( 調べたいキー )     :  現在のフレームで押したか判定できる【IsKeyDown()と同じ】\n"
                    u8"IsKeyRelase( 調べたいキー ) :  押してないかどうか判定できる      【!IsKey()と同じ】\n"
                    u8"IsKeyRepeat( 調べたいキー ) :  2フレーム以上押したか判定できる\n"
                    u8"\n"
                    u8"調べたいキー:\n"
                    u8"   KEY_INPUT_SPACE  : スペースキー\n"
                    u8"   KEY_INPUT_RETURN : エンターキー\n"
                    u8"   KEY_INPUT_UP     : Upキー\n"
                    u8" など (DxLib.hに書かれています)\n");
    }
    ImGui::End();
}

}    // namespace Tutorial
