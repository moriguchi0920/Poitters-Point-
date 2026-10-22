//---------------------------------------------------------------------------
//! @file   PoittersPoint_Log.cpp
//! @brief  PoittersPoint_Log
//---------------------------------------------------------------------------
#include "PoittersPoint_Log.h"
#include "Game/Scene/PoittersPoint_Stage.h"

#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include "Game/Component/ComponentGrabbable.h"
#include <System/Component/ComponentCollisionCapsule.h>

namespace PoittersPoint {
// namespace PoittersPoint

bool Log::Init()
{
    // 親(継承元の基底クラス)のInit関数を呼ぶ
    // これがなければabort()が呼ばれる
    Super::Init();

    SetTranslate({10.0f, 10.0f, -20.0f});

    SetName("Log");
    auto model = AddComponent<ComponentModel>("data/Game/Models/Log/Log.mv1");
    if(model) {
        // モデルの読み込み成功時の追加設定はここに
    }

    // 丸太の形状に合致する単一カプセル衝突
    // カプセルは初期で縦向きだが、丸太は横向きなので回転させる
    auto col = AddComponent<ComponentCollisionCapsule>();
    if(col) {
        col->SetRadius(3.0f);
        col->SetHeight(46.33f);
        col->SetTranslate({23.36f, 3.3f, 0.0f});

        // カプセルを横向き（丸太の長軸に沿わせる）に回転
        // 回転: {-90, 90, 0} (度)
        float matrixT[3], matrixR[3], matrixS[3];
        DecomposeMatrixToComponents(col->GetMatrixFloat(), matrixT, matrixR, matrixS);
        matrixR[0] = -90.0f;
        matrixR[1] = 90.0f;
        matrixR[2] = 0.0f;
        RecomposeMatrixFromComponents(matrixT, matrixR, matrixS, col->GetMatrixFloat());

        col->UseGravity(true);
        col->SetGravity(-0.2f);
        col->SetCollisionGroup(ComponentCollision::CollisionGroup::LOG);
        col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY | (u32)ComponentCollision::CollisionGroup::GROUND |
                                  (u32)ComponentCollision::CollisionGroup::ITEM | (u32)ComponentCollision::CollisionGroup::PLAYER);
        col->SetMass(200.0f);
    }

    auto grabbable = AddComponent<ComponentGrabbable>();
    if(grabbable) {
        grabbable->SetBounceOffset(0.0f);    // 初期は跳ねを抑える
        grabbable->SetLiftTime(1.0f);
    }

    //SE
    se_handle_       = LoadSoundMem("data/Game/LogSE/Log1.mp3");
    throw_cool_down_ = 0.0f;

    // 転がり計算用に前フレームの位置を初期化
    prev_pos_ = GetTranslate();

    SetScaleAxisXYZ({1.0f});

    return true;
}

void Log::Update()
{
    __super::Update();

    // 物理演算前の位置を記録（PostPhysicsで転がり回転に使用）
    prev_pos_ = GetTranslate();

    auto grabbable = GetComponent<ComponentGrabbable>();
    if(!grabbable)
        return;

    bool is_grabbed = grabbable->GetIsGrabbed();

    // 投擲検出：掴まれた→離された の遷移
    if(was_grabbed_prev_ && !is_grabbed) {
        // ComponentStateThrow が設定した grabbable translation_ を水平速度として取得
        // ComponentGrabbable::Update() は既に __super::Update() で移動を適用済みのため、
        // just_thrown_ を立てて PrePhysics での速度適用をスキップする
        velocity_       = grabbable->GetTranslation();
        throw_velocity_ = velocity_;
        // grabbable の translation_ をゼロにして二重移動を防ぐ
        grabbable->SetTranslation({0, 0, 0});
        // 上方向の衝動を一度だけ与える（重力は ComponentCollision が処理）
        AddTranslate({0.0f, k_throw_up_, 0.0f});
        just_thrown_ = true;
    }
    was_grabbed_prev_ = is_grabbed;

    // 掴まれている時は速度をリセット
    if(is_grabbed) {
        velocity_       = {0, 0, 0};
        throw_velocity_ = {0, 0, 0};
        just_thrown_    = false;
        is_on_ground_   = false;
        was_on_ground_  = false;
    }

    // 接地中かつ静止中は跳ねを抑制
    if(grabbable->IsGrounded() && !grabbable->IsMoving()) {
        grabbable->SetBounceOffset(0.0f);
    }
}

//! @brief 物理シミュレーション前処理
void Log::PrePhysics()
{
    // gravity_ は LateUpdate で ComponentCollision が設定し、
    // __super::PrePhysics で消費される。
    // 接地中は重力変位が極め小さい（衝突でリセットされる）。
    // 落下中は蓄積して大きくなる。
    float gravity_mag = (float)length(gravity_);
    is_on_ground_     = (gravity_mag < 0.05f);

    __super::PrePhysics();

    auto grabbable = GetComponent<ComponentGrabbable>();
    if(!grabbable)
        return;

    // 掴まれている場合は速度を適用しない
    if(grabbable->GetIsGrabbed())
        return;

    // 投擲直後のフレームは速度適用をスキップ（投擲衝動は既に ComponentStateThrow が適用済み）
    if(just_thrown_) {
        just_thrown_ = false;
        return;
    }

    // 速度を適用（水平成分のみ、鉛直は重力で処理）
    float3 horiz_v = {velocity_.x, 0.0f, velocity_.z};
    if((float)length(horiz_v) > 0.001f) {
        AddTranslate(horiz_v);
    }
}

void Log::PostPhysics()
{
    __super::PostPhysics();

    auto grabbable = GetComponent<ComponentGrabbable>();
    if(!grabbable)
        return;

    // 物理演算による移動量を取得
    float3 delta = GetTranslate() - prev_pos_;

    // 落下中は接地状態をクリア
    if(delta.y < -0.5f) {
        is_on_ground_ = false;
    }
    // 衝突システムの接地判定を併用（1フレーム遅れの gravity_ 検出を補正）
    if(grabbable->IsGrounded()) {
        is_on_ground_ = true;
    }

    // 空中→接地の遷移を検出
    bool landed_this_frame = (!was_on_ground_ && is_on_ground_);
    was_on_ground_         = is_on_ground_;

    // 未掴みの場合、空中・接地で異なる減衰を適用
    if(!grabbable->GetIsGrabbed()) {
        if(is_on_ground_) {
            // 接地中：転がり摩擦
            velocity_ *= k_friction_;
            // 着地の瞬間: 水平速度の一部を失う
            if(landed_this_frame) {
                float3 horiz  = {velocity_.x, 0.0f, velocity_.z};
                horiz        *= k_landing_loss_;
                velocity_.x   = horiz.x;
                velocity_.z   = horiz.z;
            }
            velocity_.y = 0.0f;
        }
        else {
            // 空中：空気抵抗（軽減）
            velocity_ *= k_air_drag_;
        }

        float3 horiz_v = {velocity_.x, 0.0f, velocity_.z};
        if((float)length(horiz_v) < 0.01f) {
            velocity_.x = 0.0f;
            velocity_.z = 0.0f;
        }
    }

    // 空中中は視覚的な転がり回転をしない
    if(!is_on_ground_)
        return;

    // 水平面（XZ）の移動量
    float3 horiz_delta = {delta.x, 0.0f, delta.z};
    float  horiz_dist  = length(horiz_delta);

    if(horiz_dist > 0.001f) {
        // 丸太（X軸を中心とするシリンダー）の転がり：
        // 水平移動量 / 半径 = 回転角（ラジアン）
        // ラジアンを度に変換: * (180 / PI)
        float radius    = k_roll_radius_;
        float angle_deg = (horiz_dist / radius) * 57.2957795f;

        // Z方向の移動が進行方向の時は負の回転（右手座標系）
        if(delta.z < 0.0f)
            angle_deg = -angle_deg;

        // X軸周りに累積回転
        float3 rot  = GetRotationAxisXYZ();
        rot.x      += angle_deg;
        SetRotationAxisXYZ(rot);
    }
}

void Log::GUI()
{
    __super::GUI();

    ImGui::Begin("Log Physics");
    ImGui::Text(u8"水平速度 : %.3f", (float)length(float3(velocity_.x, 0, velocity_.z)));
    ImGui::Text(u8"鉛直速度 : %.3f", velocity_.y);
    ImGui::Text(u8"接地中 : %s", is_on_ground_ ? u8"はい" : u8"いいえ");
    ImGui::Separator();
    ImGui::DragFloat(u8"空気抵抗", &k_air_drag_, 0.001f, 0.5f, 1.0f, "%.3f");
    ImGui::DragFloat(u8"転がり摩擦", &k_friction_, 0.001f, 0.5f, 1.0f, "%.3f");
    ImGui::DragFloat(u8"着地速度損失", &k_landing_loss_, 0.01f, 0.0f, 1.0f, "%.2f");
    ImGui::DragFloat(u8"投擲上方向速度", &k_throw_up_, 0.1f, 0.0f, 100.0f, "%.2f");
    ImGui::DragFloat(u8"転がり半径", &k_roll_radius_, 0.1f, 0.1f, 100.0f, "%.2f");
    ImGui::End();
}

void Log::OnHit(const ComponentCollision::HitInfo& hit_info)
{
    if(hit_info.hit_collision_->GetCollisionGroup() == ComponentCollision::CollisionGroup::GROUND) {
        // SEクールダウンタイマーの更新（DeltaTimeで減算）
        if(throw_cool_down_ > 0.0f) {
            throw_cool_down_ -= GetDeltaTime();
            if(throw_cool_down_ < 0.0f) {
                throw_cool_down_ = 0.0f;
            }
        }

        if(auto grabbable = GetComponent<ComponentGrabbable>()) {
            grabbable->SetCanGrab(true);

            // クールダウンタイマーが終了している時のみ判定
            if(throw_cool_down_ <= 0.0f) {
                // 【着地した瞬間の判定】
                // 落下・移動中で、かつまだ接地状態になっていない瞬間にSEを鳴らしバウンド
                if(!grabbable->IsGrounded() && grabbable->IsMoving()) {
                    if(se_handle_ != -1) {
                        PlaySoundMem(se_handle_, DX_PLAYTYPE_BACK);
                        throw_cool_down_ = 1.0f;    // 連打防止
                    }
                    grabbable->Bounce();
                }
            }

            // 着地後の跳ね抑制処理
            if(grabbable->IsGrounded()) {
                grabbable->SetBounceOffset(0.0f);
            }
        }
    }

    // 最後にこれを入れてください。ここでめりこみの解消などの処理を行っています。
    Super::OnHit(hit_info);
}

}    // namespace PoittersPoint
