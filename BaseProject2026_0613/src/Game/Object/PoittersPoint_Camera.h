#pragma once
//---------------------------------------------------------------------------
//! @file   PoittersPoint_Camera.h
//! @brief  PoittersPoint_Camera
//!
//! ポイッターズポイント風の自動カメラ
//!  - 注視点 : 生存者の位置と人数で作られる図形(点・線分・三角形・多角形)の中心点
//!  - 画角   : 生存者全員が必ず画面内に収まるようにカメラ距離を自動計算
//!  - 遷移   : 注視点・距離ともに指数補間で滑らかに追従
//---------------------------------------------------------------------------
#include <System/Scene.h>

namespace PoittersPoint {
// namespace PoittersPoint

USING_PTR(Camera);
class Camera : public Object
{
public:
    BP_OBJECT_DECL(Camera, "PoittersPoint::Camera");

    //! @brief 初期化
    //! @return 初期化終了
    bool Init() override;

    //! @brief 物理処理後の更新(全キャラクターの位置が確定した後にカメラを動かす)
    void PostPhysics() override;

    //! @brief 描画(デバッグ表示用)
    void Draw() override;

    //! @brief GUI表示(調整用パラメーター)
    void GUI() override;

    //! @brief 生存者が作る図形と中心点のデバッグ描画を切り替える
    void SetDebugDraw(bool enable) { debug_draw_ = enable; }
    bool IsDebugDraw() const { return debug_draw_; }

private:
    //! @brief 現在の生存者の位置を集める
    //! @return 生存者の足元座標の配列
    std::vector<float3> CollectSurvivorPositions() const;

    //! @brief 生存者が作る図形の中心点を求める
    //! 1人:その位置 / 2人:線分の中点 / 3人:三角形の重心 / 4人以上:凸包(多角形)の重心
    static float3 CalcShapeCenter(const std::vector<float3>& points);

    //! @brief 指定した注視点から、全員を画面に収めるために必要なカメラ距離を求める
    float CalcRequiredDistance(const std::vector<float3>& points, const float3& target) const;

    //! @brief カメラの前方向(注視点へ向かう向き)
    float3 GetForward() const;

    //! @brief 現在の注視点・距離をカメラコンポーネントへ反映
    void ApplyToCamera();

private:
    //---------------------------------------------------------------
    // 調整用パラメーター
    //  ※実行中はImGui(Cameraウィンドウ)から変更できる
    //  ※追従速度(～_speed_)は「1秒あたりに差をどれだけ縮めるか」の強さ。
    //    1秒後に残る差の割合は e^(-速度) になる
    //      例) 1.2 → 約30%残る(ゆっくり) / 2.5 → 約8%残る / 5.0 → 約0.7%残る(ほぼ追いつく)
    //  ※距離・高さ・半径はワールド座標の単位(キャラクターの身長が約22)
    //---------------------------------------------------------------
    float fovy_deg_            = 45.0f;           //!< 縦の画角(度)
    float aspect_ratio_        = 16.0f / 9.0f;    //!< アスペクト比
    float pitch_deg_           = 38.0f;           //!< 見下ろし角度(度) 0で真横、90で真上から見下ろす
    float yaw_deg_             = 0.0f;            //!< 水平方向の向き(度) 0で+Z側から-Z方向を見る
    float screen_margin_       = 0.80f;           //!< 画面のどれだけの範囲に収めるか(1.0で画面端ぴったり)
    float body_height_         = 22.0f;           //!< キャラクターの高さ(頭まで映すため)
    float body_radius_         = 4.0f;            //!< キャラクターの半径(体の幅まで映すため)
    float min_distance_        = 110.0f;          //!< 最も近づいたときの距離
    float max_distance_        = 520.0f;          //!< 最も離れたときの距離
    float target_follow_speed_ = 2.5f;            //!< 注視点の追従速度(大きいほど速い)
    float zoom_in_speed_       = 1.2f;            //!< 寄るときの速度(ゆっくり)
    float zoom_out_speed_      = 5.0f;            //!< 引くときの速度(映り切らないのを防ぐため速め)
    float fall_out_height_     = -30.0f;          //!< これより下に落ちたキャラクターは生存者から除外

    bool debug_draw_ = false;    //!< 生存者が作る図形と中心点を描画する

    //---------------------------------------------------------------
    // 実行時の状態
    //---------------------------------------------------------------
    bool                initialized_      = false;        //!< 初回フレームは補間せずに即座に合わせる
    float3              current_target_   = {0, 0, 0};    //!< 現在の注視点(補間後)
    float               current_distance_ = 250.0f;       //!< 現在のカメラ距離(補間後)
    float3              desired_target_   = {0, 0, 0};    //!< 目標の注視点
    float               desired_distance_ = 250.0f;       //!< 目標のカメラ距離
    std::vector<float3> survivors_;                       //!< 直近フレームの生存者位置(デバッグ描画用)
    // ※desired(目標)は毎フレーム計算し直す値、current(現在)はdesiredを滑らかに追いかける値。
    //   実際にカメラへ設定するのはcurrentの方
};

}    // namespace PoittersPoint

CEREAL_REGISTER_TYPE(PoittersPoint::Camera);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Object, PoittersPoint::Camera);
