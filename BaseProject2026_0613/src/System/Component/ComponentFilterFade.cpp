//---------------------------------------------------------------------------
//! @file   ComponentFilterFade.cpp
//! @brief  フェードフィルターコンポーネント
//---------------------------------------------------------------------------
#include <System/Component/ComponentFilterFade.h>
#include <System/Object.h>
#include <System/Scene.h>
#include <System/Graphics/Render.h>

namespace {

//--------------------------------------------------------------
//! 定数バッファパラメーター構造体
//--------------------------------------------------------------
struct Parameter
{
    float4 resolution_;      //!< 解像度 [幅, 高さ, 1.0f/幅, 1.0f/高さ]
    float  alpha_;           //!< フェードアルファ値(0.0f-1.0f)
    u32    mosaic_width_;    //!< モザイクのピクセル幅
};

}    // namespace

//---------------------------------------------------------------------------
//! コンストラクタ
//---------------------------------------------------------------------------
ComponentFilterFade::ComponentFilterFade()
{
    // ピクセルシェーダー
    shader_ps_ = std::make_shared<ShaderPs>("data/Shader/ps_filter_fade");

    // ワークテクスチャ作成
    //! @todo ここで作成するワークテクスチャは一時バッファのため、全体で共有利用するほうが良い。
    texture_work_ = std::make_shared<Texture>(WINDOW_W, WINDOW_H, DXGI_FORMAT_R8G8B8A8_UNORM);

    //-----------------------------------------------
    // 定数バッファを作成
    //-----------------------------------------------
    parameter_cb_ = CreateShaderConstantBuffer(sizeof(Parameter));
}

void ComponentFilterFade::Init()
{
    __super::Init();

    auto draw = [&]() {
        // 現在のRenderTargetを保存
        auto target_desc   = GetRenderTarget();
        auto render_target = target_desc.color_targets_[0];
        //------------------------------------------------------
        // ワークテクスチャにコピー
        //------------------------------------------------------
        CopyToRenderTarget(texture_work_.get(), render_target);

        //------------------------------------------------------
        // シェーダー適用コピーして描き戻す
        //------------------------------------------------------
        SetWriteZBufferFlag(false);    // フィルター用にデプスバッファ更新を無効化

        // 定数バッファ更新
        auto* p = reinterpret_cast<Parameter*>(GetBufferShaderConstantBuffer(parameter_cb_));
        {
            p->resolution_ = float4(WINDOW_W, WINDOW_H, 1.0f / WINDOW_W, 1.0f / WINDOW_H);
            p->alpha_      = fade_alpha_;

            // モザイク幅
            f32 mosaic_factor = 1.0f - fade_alpha_;    // フェードアウトすると粗くなっていく
            f32 mosaic_width  = mosaic_width_ * mosaic_factor;

            p->mosaic_width_ = std::max(1u, static_cast<u32>(mosaic_width));

            UpdateShaderConstantBuffer(parameter_cb_);    // シェーダー用定数バッファハンドルの定数バッファへの変更を適用する
        }

        // 定数バッファをb4に設定
        SetShaderConstantBuffer(parameter_cb_, DX_SHADERTYPE_PIXEL, 4);

        // 全画面描画
        CopyToRenderTarget(render_target, texture_work_.get(), *shader_ps_);

        SetShaderConstantBuffer(-1, DX_SHADERTYPE_PIXEL, 4);    // b4
        SetWriteZBufferFlag(true);                              // 元に戻す

        //------------------------------------------------------
        // 元に戻す
        //------------------------------------------------------
        SetRenderTarget(target_desc);
    };

    // 描画タイミングを設定して描画
    SetProc("FilterDraw", draw, ProcTiming::Filter, ProcPriority::NORMAL);
}

//---------------------------------------------------------------------------
//! 更新
//---------------------------------------------------------------------------
void ComponentFilterFade::Update()
{
    f32 delta = GetDeltaTime();

    if(finish_fade_)
        return;

    fade_alpha_ += fade_add_alpha_ * delta;
    if(fade_alpha_ >= 1.0f) {
        finish_fade_ = true;
        fade_alpha_  = 1.0f;
    }
    if(fade_alpha_ < 0.0f) {
        finish_fade_ = true;
        fade_alpha_  = 0.0f;
    }
}

//---------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------
void ComponentFilterFade::Exit()
{
    __super::Exit();

    // シェーダー用定数バッファハンドルを削除する
    DeleteShaderConstantBuffer(parameter_cb_);
}

//---------------------------------------------------------------------------
//! GUI描画
//---------------------------------------------------------------------------
void ComponentFilterFade::GUI()
{
    assert(GetOwner());
    auto objName = GetOwner()->GetName();

    ImGui::Begin(objName.data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode(u8"フェード処理")) {
            if(ImGui::Button(u8"削除")) {
                GetOwner()->RemoveComponent(shared_from_this());
            }

            //--------------------------------------------------
            // 調整項目を表示
            //--------------------------------------------------
            ImGui::SliderFloat(u8"フェード係数", &fade_alpha_, 0.0f, 1.0f, "%1.2f");
            ImGui::DragInt(u8"モザイクの最大幅", &mosaic_width_, 1.0f, 1, 256, "%3.1f");

            ImGui::TreePop();
        }
    }

    ImGui::End();
}
