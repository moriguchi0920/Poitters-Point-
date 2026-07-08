//---------------------------------------------------------------------------
//!	@file	SystemMain.cpp
//! @brief	システムメイン
//---------------------------------------------------------------------------
#include <System/Debug/DebugCamera.h>
#include <System/Physics/PhysicsEngine.h>

//----------------------------------------------------------------
// シーンオブジェクト
//----------------------------------------------------------------
#include <System/Scene.h>
#include <System/Utils/IniFileLib.h>
#include "LightManager.h"
#include "SystemMain.h"
#include <System/GuiMenu.h>

namespace {
// iniファイルで上書きされます
bool show_gui   = true;    //!< GUIの表示 (ini "GUIEditor")
bool show_debug = true;    //!< デバッグ表示 (ini "GUIEditor")
bool show_fps   = true;    //!< FPSの表示 (ini "ShowFPS")
bool show_grid  = true;    //!< グリッドの表示 (ini "ShowGrid")
bool debug_keys = true;    //<! デバッグキー

u64 current_time_ = 0;       //!< 現在の時間 (単位:μsec)
f32 delta_time_   = 0.0f;    //!< 1フレームの経過時間（CPUとGPU, ScreenFlip()更新待ちすべて含む）

bool menu_active = false;
bool menu_select = false;

bool hide = false;

float3 clear_color = {0.5f, 0.5f, 0.5f};
//--------------------------------------------------------------
//! @name   CPU負荷計測
//--------------------------------------------------------------
//@{

u64 cpu_start_counter_    = 0;    //!< 1フレームの開始タイミングカウンター
u64 cpu_profile_duration_ = 0;    //!< 1フレームのCPU処理時間(単位:μsec)

//@}

//! 物理シミュレーションの実体
std::unique_ptr<physics::Engine> physics_engine_;

//! デバッグカメラの移動方法
DebugCamera::Control control = DebugCamera::UnrealEngine;

//! 光源管理
LightManager light_manager_;

std::shared_ptr<Texture> texture_hdr_;    //!< HDRバッファ

std::shared_ptr<ShaderPs> shader_ps_tonemapping_;    // ピクセルシェーダー

}    // namespace

//---------------------------------------------------------------------------
//! 1フレームのCPU処理時間を取得(単位:μsec)
//---------------------------------------------------------------------------
u64 GetCpuProfile()
{
    return cpu_profile_duration_;
}

//---------------------------------------------------------------------------
//! パフォーマンスカウンターを取得 (単位:μsec)
//---------------------------------------------------------------------------
u64 GetPerformanceCounterMicroSec()
{
    u64 counter = DxLib::GetNowSysPerformanceCount();
    return DxLib::ConvSysPerformanceCountToMicroSeconds(counter);
}

//---------------------------------------------------------------------------
//! 経過時間計測をリセット
//---------------------------------------------------------------------------
void ResetDeltaTime()
{
    current_time_ = GetPerformanceCounterMicroSec();
}

//---------------------------------------------------------------------------
//! 1フレームの間に経過した時間を計算
//---------------------------------------------------------------------------
void UpdateDeltaTime()
{
    u64 last_time  = current_time_;
    current_time_  = GetPerformanceCounterMicroSec();
    f32 delta_time = static_cast<f32>(current_time_ - last_time) * (1.0f / 1000.0f / 1000.0f);
    // デバッグ中である可能性やロードなどでヒッチがかかっている場合を考慮
    if(delta_time < 1.0f)
        delta_time_ = delta_time;
}

//---------------------------------------------------------------------------
//! 1フレームの間に経過した時間を取得
//---------------------------------------------------------------------------
f32 GetDeltaTime()
{
    return delta_time_;
}

//---------------------------------------------------------------------------
// これを掛けると60FPSのときと基本的に同じ速さになる
//---------------------------------------------------------------------------
f32 GetDeltaTime60()
{
    f32 ps = delta_time_ * 60.0f;
    if(ps > 5.0f)    // デバッグで止めたときの対処
        ps = 1.0f;
    return ps;
}

//---------------------------------------------------------------------------
// ツールなどで使用する際強制的にFPSを変更する
//---------------------------------------------------------------------------
void SetDeltaTime(f32 fps)
{
    delta_time_ = fps;
}

//---------------------------------------------------------------------------------
//! グリッドの表示をON/OFFします
//---------------------------------------------------------------------------------
void ShowGrid(bool active)
{
    show_grid = active;
}

//===========================================================================
// 数値スクロール表示用のリングバッファ
//===========================================================================
struct ScrollingBuffer
{
    u32                 offset_ = 0;    //!< データー先頭の位置
    std::vector<ImVec2> data_;          //!< プロットデーター配列

    ScrollingBuffer(size_t max_size = 4096) { data_.reserve(max_size); }
    void AddPoint(f32 t, f32 value)
    {
        // プロットする値
        auto x = ImVec2(t, value);

        if(data_.size() < data_.capacity()) {
            // 新規追加
            data_.emplace_back(std::move(x));
        }
        else {
            // リングバッファとして再利用
            data_[offset_] = x;
            offset_        = (offset_ + 1) % static_cast<u32>(data_.capacity());
        }
    }
    void clear()
    {
        data_.clear();
        offset_ = 0;
    }
};

//---------------------------------------------------------------------------
//! FPSを表示します
//! @param  [in]    delta   1フレームの経過時間(単位:秒)
//---------------------------------------------------------------------------
void ShowFps(f32 delta)
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |          // タイトルバー/スクロールバーなし
                                    ImGuiWindowFlags_NoDocking |             // ドッキングなし
                                    ImGuiWindowFlags_AlwaysAutoResize |      // 自動リサイズなし
                                    ImGuiWindowFlags_NoSavedSettings |       // 保存しない
                                    ImGuiWindowFlags_NoFocusOnAppearing |    // フォーカスしない
                                    ImGuiWindowFlags_NoNav |                 // キーやゲームパッドで操作対象にしない
                                    ImGuiWindowFlags_NoMove;                 // 移動させない

    //----------------------------------------------------------
    // 表示位置設定
    //----------------------------------------------------------
    constexpr u32 corner = 2;    // 0:左上 1:右上 2:左下 3: 右下
    {
        constexpr f32        PADDING   = 10.0f;    // 余白部分
        const ImGuiViewport* viewport  = ImGui::GetMainViewport();
        ImVec2               work_pos  = viewport->WorkPos;    // WorkPosを使用するとメニューバーなどの位置を考慮した位置が取得できる
        ImVec2               work_size = viewport->WorkSize;
        ImVec2               window_pos, window_pos_pivot;
        window_pos.x       = (corner & 1) ? (work_pos.x + work_size.x - PADDING) : (work_pos.x + PADDING);
        window_pos.y       = (corner & 2) ? (work_pos.y + work_size.y - PADDING) : (work_pos.y + PADDING);
        window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowViewport(viewport->ID);
    }

    //----------------------------------------------------------
    // 半透明の背景
    //----------------------------------------------------------
    ImGui::SetNextWindowBgAlpha(0.35f);

    //----------------------------------------------------------
    // オーバーレイウィンドウを表示
    //----------------------------------------------------------

    // モニターのリフレッシュレートを取得 (Hz)
    s32 refresh_rate = 60;
    {
        HDC hdc      = GetDC(GetMainWindowHandle());    // デバイスコンテキストの取得
        refresh_rate = GetDeviceCaps(hdc, VREFRESH);    // リフレッシュレートの取得
        ReleaseDC(GetMainWindowHandle(), hdc);          // デバイスコンテキストの解放
    }

    // 現在のフレームレート
    auto frame_rate = ImGui::GetIO().Framerate;

    if(static_cast<f32>(refresh_rate) < ImGui::GetIO().Framerate) {
        frame_rate = static_cast<f32>(refresh_rate);
    }

    // CPU負荷(μsec)
    u64 cpu_profile_time = GetCpuProfile();

    // 角を丸める
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

    if(ImGui::Begin(u8"FPS計測", nullptr, window_flags)) {
        //----------------------------------------------------------
        // フレームレートグラフを表示
        //----------------------------------------------------------
        static ScrollingBuffer cpu_data, fps_data;

        // 経過時間
        static float t  = 0;
        t              += delta;

        // 数値を0.0f～1.0fの範囲で設定する
        f32  cpu_micro_sec = static_cast<f32>(cpu_profile_time);
        auto ratio         = cpu_micro_sec / (1000.0f * 1000.0f / static_cast<f32>(refresh_rate));

        cpu_data.AddPoint(t, ratio);                                          // CPU負荷
        fps_data.AddPoint(t, frame_rate / static_cast<f32>(refresh_rate));    // フレームレート

        static float history = 10.0f;

        constexpr ImPlotFlags flags = ImPlotFlags_NoInputs | ImPlotFlags_NoFrame;

        if(ImPlot::BeginPlot(u8"フレームレート & CPU負荷", ImVec2(-1.0f, 96.0f), flags)) {
            constexpr ImPlotAxisFlags axis_flags = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_Lock;

            ImPlot::SetupAxes(NULL, NULL, flags, axis_flags);
            ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);    // 表示範囲
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0f, 1.01f);                         // 上下数値の範囲(最大値目盛りを出すため1.01f)
            ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);

            ImPlot::PlotShaded(u8"CPU負荷",                                // 名前
                               &cpu_data.data_[0].x,                       // 時間軸t
                               &cpu_data.data_[0].y,                       // 値
                               static_cast<s32>(cpu_data.data_.size()),    // 配列数
                               -INFINITY,                                  // 塗りつぶし範囲
                               ImPlotShadedFlags_None,                     // ImPlotShadedFlags
                               cpu_data.offset_,                           // 先頭オフセット
                               sizeof(ImVec2));                            // 構造体あたりのサイズ

            ImPlot::PlotLine("fps",                                      // 名前
                             &fps_data.data_[0].x,                       // 時間軸t
                             &fps_data.data_[0].y,                       // 値
                             static_cast<s32>(fps_data.data_.size()),    // 配列数
                             ImPlotShadedFlags_None,                     // ImPlotShadedFlags
                             fps_data.offset_,                           // 先頭オフセット
                             sizeof(ImVec2));                            // 構造体あたりのサイズ
            ImPlot::EndPlot();
        }
        ImGui::SliderFloat(u8"履歴範囲", &history, 1, 30, "%.1f s");
    }

    //----------------------------------------------------------
    // 現在の数値を表示
    //----------------------------------------------------------
    ImGui::Separator();
    ImGui::Text(u8"FPS    : %3.2f fps (max:%3d fps)", frame_rate, refresh_rate);
    ImGui::Text(u8"CPU負荷 : %3.2f ms", static_cast<f32>(cpu_profile_time) / 1000.0f);

    // オーバーレイウィンドウ終了
    ImGui::End();
    ImGui::PopStyleVar();    // 角を丸める設定を元に戻す
}

//---------------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------------
void SystemInit()
{
    // 光源管理を初期化
    light_manager_.initialize();

    // Editor.iniから読み取り
    Scene::LoadEditor();

    // Game.iniから読み込む
    IniFileLib ini("Game.ini");

    auto scene_name = ini.GetString("Scene", "Start");
    // 作成
    auto* scene = CreateInstanceFromName<Scene::Base>(scene_name);
    if(scene) {
        Scene::Change(std::shared_ptr<Scene::Base>(scene));
    }
    else {
        // iniファイルの設定のクラスが見つからない場合はサンプルシーンを起動
        scene = CreateInstanceFromName<Scene::Base>("SceneSample");
        if(scene) {
            Scene::Change(std::shared_ptr<Scene::Base>(scene));
        }
    }

    show_debug = ini.GetBool("System", "GUIEditor");
    show_gui   = show_debug;
    show_grid  = ini.GetBool("System", "ShowGrid");
    show_fps   = ini.GetBool("System", "ShowFPS");
    if(!ini.GetBool("System", "ShowMouse", true)) {
        HideMouse();
        hide = true;
    }
    clear_color = ini.GetFloat3("System", "ClearColor", clear_color);

    debug_keys = ini.GetBool("System", "DebugKeys", true);

    //----------------------------------------------------------
    // 物理シミュレーションを初期化
    //----------------------------------------------------------
    physics_engine_ = physics::createPhysics();

    // 現在の時間を初期化
    ResetDeltaTime();

    //----------------------------------------------------------
    // 描画先テクスチャを作成
    //----------------------------------------------------------
    texture_hdr_ = std::make_shared<Texture>(WINDOW_W, WINDOW_H, DXGI_FORMAT_R16G16B16A16_FLOAT);

    // シェーダー読込
    shader_ps_tonemapping_ = std::make_shared<ShaderPs>("data/Shader/ps_tonemapping");
}

//---------------------------------------------------------------------------------
//! 更新
//---------------------------------------------------------------------------------
void SystemUpdate()
{
    // 1フレームの間に経過した時間を計算 ⊿t
    UpdateDeltaTime();

    if(debug_keys) {
        //----------------------------------------------------------
        // メインメニューバー
        //----------------------------------------------------------
        if constexpr(false) {
            // Altキーでメニューを開閉
            if(IsKeyDown(KEY_INPUT_LALT) || IsKeyDown(KEY_INPUT_RALT)) {
                menu_active = !menu_active;
            }
        }
        else {
            // 画面上部にマウスカーソルがあるときにメニューを開閉
            s32 x, y;
            DxLib::GetMousePoint(&x, &y);
            menu_active = y < 16;

            // ウィンドウがアクティブなときは常に表示
            if(ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow)) {
                menu_active = true;
            }
        }

        // F5キーでデバッグ表示変更
        if(IsKeyDown(KEY_INPUT_F5)) {
            show_debug = !show_debug;
            // show_debugにすべて合わせる(表示/非表示が反対状態になることを避けます)
            show_gui  = show_debug;
            show_grid = show_debug;
            show_fps  = show_debug;
        }
        // F4キーでカメラ変更
        if(IsKeyDown(KEY_INPUT_F4)) {
            auto debug_camera = !DebugCamera::IsUse();
            DebugCamera::Use(debug_camera);
        }

        if(IsKeyDown(KEY_INPUT_F6)) {
            hide = !hide;
            HideMouse(hide);
        }

        menu_select = false;
        if(menu_active) {
            if(ImGui::BeginMainMenuBar()) {
                if(ImGui::BeginMenu("File")) {
                    ImGui::EndMenu();
                }

                if(ImGui::BeginMenu("Edit")) {
                    ImGui::EndMenu();
                }

                // シーン選択
                if(ImGui::BeginMenu("Scene")) {
                    menu_select = true;

                    gui::GuiMenuScene::GUI();

                    ImGui::EndMenu();
                }

                if(ImGui::BeginMenu("Debug")) {
                    menu_select = true;
                    ImGui::Checkbox(u8"グリッド表示", &show_grid);
                    ImGui::Checkbox(u8"FPS表示", &show_fps);
                    ImGui::Separator();
                    ImGui::Checkbox(u8"GUI表示", &show_gui);
                    ImGui::Separator();

                    static bool debug_camera;
                    debug_camera = DebugCamera::IsUse();

                    ImGui::Checkbox(u8"デバッグカメラ(F4)", &debug_camera);
                    if(debug_camera) {
                        if(ImGui::BeginMenu(u8"移動方式")) {
                            ImGui::RadioButton("Unity", (int*)(&control), DebugCamera::Unity);
                            ImGui::RadioButton("UnrealEngine", (int*)(&control), DebugCamera::UnrealEngine);
                            ImGui::RadioButton("Maya", (int*)(&control), DebugCamera::Maya);
                            ImGui::EndMenu();
                        }
                    }

                    DebugCamera::Use(debug_camera);
                    DebugCamera::SetControl(control);
                    ImGui::Separator();
                    ImGui::CheckboxFlags(u8"エディター配置", (int*)&Scene::SceneStatus(), 1 << (int)Scene::EditorStatusBit::EditorPlacement);
                    if(Scene::GetEditorStatus(Scene::EditorStatusBit::EditorPlacement)) {
                        ImGui::CheckboxFlags(u8"Always", (int*)&Scene::SceneStatus(), 1 << (int)Scene::EditorStatusBit::EditorPlacement_Always);
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
        }
    }

    //----------------------------------------------------------
    // 光源情報を更新
    //----------------------------------------------------------
    light_manager_.update();

    //----------------------------------------------------------
    // シーンの更新前処理
    //----------------------------------------------------------
    Scene::PreUpdate();

    //----------------------------------------------------------
    // シーンの更新
    //----------------------------------------------------------
    Scene::Update();

    //----------------------------------------------------------
    // GUI処理&描画
    //----------------------------------------------------------
    if(show_gui)
        Scene::GUI();

    //----------------------------------------------------------
    // 物理シミュレーション前の処理
    //----------------------------------------------------------
    Scene::PrePhysics();

    //----------------------------------------------------------
    // 物理シミュレーションを更新
    //----------------------------------------------------------
    float physics_time = delta_time_;
    if(Scene::IsPause())
        physics_time = 0.0f;

    physics_engine_->update(physics_time);

    //----------------------------------------------------------
    // 物理シミュレーション後の処理
    //----------------------------------------------------------
    Scene::PostPhysics();

    //----------------------------------------------------------
    // シーンの更新後処理
    //----------------------------------------------------------
    Scene::PostUpdate();
}

//---------------------------------------------------------------------------------
//! 描画
//---------------------------------------------------------------------------------
void SystemDraw()
{
    //----------------------------------------------------------
    // HDRの描画先を指定
    //----------------------------------------------------------
    SetRenderTarget(texture_hdr_.get(), GetDepthStencil());
    ClearColor(texture_hdr_.get(), float4(clear_color, 0.0f));

    //----------------------------------------------------------
    // グリッドを描画
    //----------------------------------------------------------
    if(show_grid) {
        constexpr f32 size = 64.0f;    // グリッドの範囲

        for(f32 x = -size; x <= size; x += 1.0f) {
            DxLib::DrawLine3D(VGet(x, 0.0f, -size), VGet(x, 0.0f, +size), GetColor(224, 224, 224));
        }
        for(f32 z = -size; z <= size; z += 1.0f) {
            DxLib::DrawLine3D(VGet(-size, 0.0f, z), VGet(+size, 0.0f, z), GetColor(224, 224, 224));
        }

        // X軸
        DxLib::DrawLine3D(VGet(-size, 0.0f, 0.0f), VGet(+size, 0.0f, 0.0f), GetColor(255, 64, 64));
        // Y軸
        DxLib::DrawLine3D(VGet(0.0f, -size, 0.0f), VGet(0.0f, +size, 0.0f), GetColor(64, 255, 64));
        // Z軸
        DxLib::DrawLine3D(VGet(0.0f, 0.0f, -size), VGet(0.0f, 0.0f, +size), GetColor(64, 64, 255));
    }

    // シーンの描画
    Scene::Draw();

    //----------------------------------------------------------
    // トーンマッピング
    //----------------------------------------------------------

    // 元の描画先に戻す
    SetRenderTarget(GetBackBuffer(), GetDepthStencil());

    // トーンマッピング適用
    CopyToRenderTarget(GetBackBuffer(), texture_hdr_.get(), *shader_ps_tonemapping_);

    // FPSの表示
    if(show_fps) {
        ShowFps(delta_time_);
    }
}

//---------------------------------------------------------------------------------
//! 終了
//---------------------------------------------------------------------------------
void SystemExit()
{
    Scene::SaveEditor();

    Scene::Exit();

    // HDRバッファの解放
    texture_hdr_.reset();

    //----------------------------------------------------------
    // 物理シミュレーションを解放
    //----------------------------------------------------------
    physics_engine_.reset();

    //----------------------------------------------------------
    // 光源管理を解放
    //----------------------------------------------------------
    light_manager_.finalize();
}

//---------------------------------------------------------------------------------
//	フレームの開始
//---------------------------------------------------------------------------------
void SystemBeginFrame()
{
    // CPU計測開始
    cpu_start_counter_ = GetPerformanceCounterMicroSec();
}

//---------------------------------------------------------------------------------
//	フレームの終了
//---------------------------------------------------------------------------------
void SystemEndFrame()
{
    // CPU計測終了
    u64 cpu_end_counter   = GetPerformanceCounterMicroSec();
    cpu_profile_duration_ = cpu_end_counter - cpu_start_counter_;
}

bool IsShowMenu()
{
    return menu_active && menu_select;
}

bool IsShowDebug()
{
    return show_debug;
}

bool IsShowGUI()
{
    return show_gui;
}

bool IsShowFPS()
{
    return show_fps;
}

bool IsShowGrid()
{
    return show_grid;
}

//---------------------------------------------------------------------------
//! RenderTarget HDRバッファを取得
//---------------------------------------------------------------------------
Texture* GetHdrBuffer()
{
    return texture_hdr_.get();
}
