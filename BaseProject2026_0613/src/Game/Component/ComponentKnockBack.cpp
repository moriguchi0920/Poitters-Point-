#include <Game/Component/ComponentKnockBack.h>
#include <Game/Component/ComponentGrabbable.h>    // 当たり元が掴み可能オブジェクトか判定するため
#include <imgui.h>                                // GUI を使う場合（既存コードと合わせてください）

void ComponentKnockBack::Init()
{
    __super::Init();
    // 必要なら初期化処理をここに追加
}

void ComponentKnockBack::Update()
{
    __super::Update();

    // 不正値ガード：オーバーフロー等で inf/NaN が混入したら即クリア（Transform の NaN 化を防ぐ）
    if(!std::isfinite(knockback_velocity.x) || !std::isfinite(knockback_velocity.y) || !std::isfinite(knockback_velocity.z)) {
        knockback_velocity = float3(0.0f, 0.0f, 0.0f);
        knockback_airborne = false;
        air_frames         = 0;
        active_frames      = 0;
        return;
    }

    // ノックバック速度が充分に大きいか滞空中なら適用する
    float len2 = knockback_velocity.x * knockback_velocity.x + knockback_velocity.y * knockback_velocity.y + knockback_velocity.z * knockback_velocity.z;
    if(len2 > (velocity_epsilon * velocity_epsilon) || knockback_airborne) {
        // 速度が拡散し続ける場合も強制終了する（安全打ち切り）
        if(++active_frames > 180) {
            knockback_velocity = float3(0.0f, 0.0f, 0.0f);
            knockback_airborne = false;
            air_frames         = 0;
            active_frames      = 0;
            return;
        }

        // Component からは直接 AddTranslate を呼べないのでオーナー経由で移動を適用する
        if(auto owner = GetOwner()) {
            // false: ワールド移動, direct=false: delta を掛ける既存挙動
            owner->AddTranslate(knockback_velocity, false, false);
        }

        // XZ 減衰：damping は 0..1 に clamp する（負値だと速度が毎フレーム増幅し inf→NaN になる）
        float decay = 1.0f - std::clamp(damping, 0.0f, 1.0f);
        knockback_velocity.x *= decay;
        knockback_velocity.z *= decay;

        // 水平速度に上限をかけ、キャラコントローラ（Jolt）側のオーバーフローを防ぐ
        float hlen = sqrtf(knockback_velocity.x * knockback_velocity.x + knockback_velocity.z * knockback_velocity.z);
        if(hlen > max_knockback_speed) {
            float scale      = max_knockback_speed / hlen;
            knockback_velocity.x *= scale;
            knockback_velocity.z *= scale;
        }

        // Y は減衰させず重力を積分して放物線（上り→頂点→落下）を描かせる
        if(knockback_airborne) {
            air_frames++;
            knockback_velocity.y -= arch_gravity;
            if(knockback_velocity.y < -max_fall_speed)
                knockback_velocity.y = -max_fall_speed;

            // 落下して打ち出し高さまで戻ったら着地
            float cur_y = GetOwner() ? GetOwner()->GetTranslate().y : 0.0f;
            if(std::isfinite(cur_y) && knockback_velocity.y < 0.0f && cur_y <= launch_y + 0.5f) {
                knockback_velocity.y = 0.0f;
                knockback_airborne   = false;
            } else if(air_frames > 180) {
                knockback_velocity.y = 0.0f;
                knockback_airborne   = false;
            }
        }

        // 極小値でゼロクリア
        if(!knockback_airborne) {
            float len =
                sqrtf(knockback_velocity.x * knockback_velocity.x + knockback_velocity.y * knockback_velocity.y + knockback_velocity.z * knockback_velocity.z);
            if(len < velocity_epsilon) {
                knockback_velocity = float3(0.0f, 0.0f, 0.0f);
                active_frames      = 0;
            }
        }
    }
}

void ComponentKnockBack::ApplyKnockback(const float3& velocity)
{
    float3 v = velocity;

    // 水平の強さに応じて上向きを上乗せし、アーチ状の軌道にする（最低上向き成分も確保）
    float hlen  = sqrtf(v.x * v.x + v.z * v.z);
    float minUp = min_upward + hlen * arch_strength;
    if(v.y < minUp)
        v.y = minUp;

    // 適用直後の速度が上限を超えないようにする（外部から巨大な値が渡されても安全）
    float speed = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if(speed > max_knockback_speed)
        v = v * (max_knockback_speed / speed);

    // 滞空開始なら打ち出し高さを記録する（滞空時の再ヒットは基準を変えない）
    if(!knockback_airborne) {
        if(auto owner = GetOwner())
            launch_y = owner->GetTranslate().y;
        air_frames    = 0;
        active_frames = 0;
    }
    knockback_airborne = true;

    // 累積する（複数回ヒットや追加入力を許す）
    knockback_velocity += v;
}

void ComponentKnockBack::OnHitComponent(const HitInfo& hit_info)
{
    // Scene からの当たり通知を受け、当たり元が掴める投げ物ならノックバックを計算して適用する
    if(!hit_info.hit_)
        return;
    if(!hit_info.hit_collision_)
        return;

    auto hitter = hit_info.hit_collision_->GetOwner();
    if(!hitter)
        return;

    // 掴み可能（投げ物）の場合のみノックバック計算（弾など別ロジックならここで拡張）
    if(auto grabbable = hitter->GetComponent<ComponentGrabbable>()) {
        // 自分を投げたものだったらスキップ
        if(grabbable->IsThrower(GetOwnerPtr()))
            return;

        // 地面で停止している投げ物は無視
        if(!grabbable->IsMoving())
            return;

        // 連続ヒット防止（既に当たっているなら無視）
        if(grabbable->IsAlreadyHit(GetOwnerPtr()))
            return;

        // ヒット記録
        grabbable->AddHitTarget(GetOwnerPtr());

        // 投げ物の移動量（ComponentGrabbable::GetTranslation() を速度相当として使用）
        float3 throwVel = grabbable->GetTranslation();
        float  velMag   = sqrtf(throwVel.x * throwVel.x + throwVel.y * throwVel.y + throwVel.z * throwVel.z);

        // 副方向：投げ物の進行方向（散開感を出すブレンド用）
        float3 travel_dir = {throwVel.x, 0.0f, throwVel.z};
        float  travel_len = sqrtf(travel_dir.x * travel_dir.x + travel_dir.z * travel_dir.z);
        if(travel_len > 0.0001f)
            travel_dir = travel_dir / travel_len;
        else
            travel_dir = float3{0.0f, 0.0f, 1.0f};

        // 主方向：ヒッターから自分への外側方向（当たり元＝プレイヤーから見て後ろへ飛ぶ）
        float3 push_dir = GetOwner()->GetTranslate() - hitter->GetTranslate();
        push_dir.y      = 0.0f;
        float push_len  = sqrtf(push_dir.x * push_dir.x + push_dir.z * push_dir.z);
        if(push_len > 0.0001f)
            push_dir = push_dir / push_len;
        else
            push_dir = travel_dir;

        // 当たり元の進行方向を少量ブレンドしつつ、基本は当たり元から遠ざかる向き（後ろへ吹き飛ぶ）
        float3 hit_dir = push_dir * away_weight + travel_dir * (1.0f - away_weight);
        float  hit_len = sqrtf(hit_dir.x * hit_dir.x + hit_dir.z * hit_dir.z);
        if(hit_len > 0.0001f)
            hit_dir = hit_dir / hit_len;
        else
            hit_dir = travel_dir;

        // 調整係数（投げ物の移動速度は frame あたり 25 程度なので小さめに調整）
        float baseForce   = 0.4f;
        float damageBonus = grabbable->GetDamage() * 0.5f;
        float force       = velMag * baseForce + damageBonus;

        // XZ = 当たり方向へ水平に吹き飛ぶ / Y = 投げの鉛直成分を下敷きにする
        // （ApplyKnockback 側でアーチ強度分を上乗せし、Update 側で重力積分して弧状に飛んで落下する）
        float3 knock = hit_dir * force;
        knock.y      = throwVel.y * 0.1f;

        ApplyKnockback(knock);
    }
}

void ComponentKnockBack::GUI()
{
    __super::GUI();

    // オーナー名を使ったシンプルなウィンドウ（既存スタイルに合わせてください）
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();

        // 有効/無効
        bool enable = GetStatus(StatusBit::Enable);
        if(ImGui::Checkbox(u8"有効", &enable))
            SetStatus(StatusBit::Enable, enable);

        // ノックバック表示と調整
        ImGui::Text(u8"現在のノックバック: (%.2f, %.2f, %.2f)", knockback_velocity.x, knockback_velocity.y, knockback_velocity.z);
        ImGui::Text(u8"滞空: %s / 打ち出しY: %.2f / 滞空F: %d", knockback_airborne ? u8"中" : u8"-", launch_y, air_frames);
        ImGui::SliderFloat(u8"減衰 (damping)", &damping, 0.0f, 1.0f);
        ImGui::SliderFloat(u8"最低上向き (min_upward)", &min_upward, 0.0f, 20.0f);
        ImGui::SliderFloat(u8"アーチ強度 (arch_strength)", &arch_strength, 0.0f, 1.0f);
        ImGui::SliderFloat(u8"アーチ重力 (arch_gravity)", &arch_gravity, 0.0f, 2.0f);
        ImGui::SliderFloat(u8"落下速度上限 (max_fall_speed)", &max_fall_speed, 1.0f, 50.0f);
        ImGui::SliderFloat(u8"速度上限 (max_knockback_speed)", &max_knockback_speed, 1.0f, 50.0f);
        ImGui::SliderFloat(u8"後ろへ飛ぶ度 (away_weight)", &away_weight, 0.0f, 1.0f);
        ImGui::SliderFloat(u8"閾値 (velocity_epsilon)", &velocity_epsilon, 0.0f, 1.0f);

        if(ImGui::Button(u8"ノックバッククリア")) {
            knockback_velocity = float3{0.0f, 0.0f, 0.0f};
            knockback_airborne = false;
            air_frames         = 0;
            active_frames      = 0;
        }

        if(ImGui::Button(u8"削除")) {
            GetOwner()->RemoveComponent(shared_from_this());
        }
    }
    ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentKnockBack)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentKnockBack)
