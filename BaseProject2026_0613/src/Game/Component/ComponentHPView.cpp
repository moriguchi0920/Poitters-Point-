#pragma once
#include <Game/Component/ComponentHPView.h>

void ComponentHPView::Init()
{
    __super::Init();

    HP_image_    = LoadGraph("data/Game/Image/HP.png");       //　背景画像
    Heart_image_ = LoadGraph("data/Game/Image/Heart.png");    // ハートの画像
}

void ComponentHPView::Update()
{
}

void ComponentHPView::PostDraw()
{
    //DrawString(100, 50, "HPView::Draw() called!", GetColor(255, 255, 255));

    // 元の画像サイズを取得
    int img_w, img_h;
    GetGraphSize(HP_image_, &img_w, &img_h);

    // 2. 縮小する
    float scale  = 0.7f;
    int   draw_w = static_cast<int>(img_w * scale);
    int   draw_h = static_cast<int>(img_h * scale);

    if(HP_image_ != -1) {
        int start_x = 50;     // X座標
        int start_y = 550;    // Y座標
        int spacing = draw_w + 50;

        int draw_x = start_x + (spacing * player_id_);
        int draw_y = start_y;

        for(int i = 0; i < 4; i++) {
            int draw_x = start_x + (spacing * i);
            int draw_y = start_y;

            // UI画像を描画
            DrawExtendGraph(draw_x, draw_y, draw_x + draw_w, draw_y + draw_h, HP_image_, TRUE);

            if(Heart_image_ != -1) {
                int h_w = static_cast<int>(draw_w * 0.35f);    // 枠の幅に対する割合
                int h_h = static_cast<int>(draw_h * 0.70f);    // 枠の高さに対する割合

                // X座標：枠の左端(draw_x) ＋ 「枠の幅の半分」 ＋ 少しの余白
                int heart_x = draw_x + (draw_w / 2) + 10;

                // Y座標：枠の縦方向の中央に配置する計算
                int heart_y = draw_y + (draw_h / 2) - (h_h / 2) + 5;

                DrawExtendGraph(heart_x, heart_y, heart_x + h_w, heart_y + h_h, Heart_image_, TRUE);
            }
        }
    }
}

void ComponentHPView::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("Hit Points")) {
            //-------------------------------------------------------
            // 共通部分(共通化したい)

            // 有効/無効
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable))
                SetStatus(StatusBit::Enable, enable);

            // GUI上でオーナーから自分(SampleObjectController)を削除します
            if(ImGui::Button(u8"削除"))
                GetOwner()->RemoveComponent(shared_from_this());
            //-------------------------------------------------------

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentHPView)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentHPView)
