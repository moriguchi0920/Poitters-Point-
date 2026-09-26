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

    // ノックバック速度が充分に大きければ適用し、減衰させる
    float len2 = knockback_velocity.x * knockback_velocity.x + knockback_velocity.y * knockback_velocity.y + knockback_velocity.z * knockback_velocity.z;
    if(len2 > (velocity_epsilon * velocity_epsilon)) {
        // Component からは直接 AddTranslate を呼べないのでオーナー経由で移動を適用する
        if(auto owner = GetOwner()) {
            // false: ワールド移動, direct=false: delta を掛ける既存挙動
            owner->AddTranslate(knockback_velocity, false, false);
        }

        // 減衰：スカラー乗算で減衰を適用
        knockback_velocity = knockback_velocity * (1.0f - damping);

        // 極小値でゼロクリア
        float len =
            sqrtf(knockback_velocity.x * knockback_velocity.x + knockback_velocity.y * knockback_velocity.y + knockback_velocity.z * knockback_velocity.z);
        if(len < velocity_epsilon) {
            knockback_velocity = float3(0.0f, 0.0f, 0.0f);
        }
    }
}

void ComponentKnockBack::ApplyKnockback(const float3& velocity)
{
    float3 v = velocity;
    // 最低上向き成分を確保しておく
    if(v.y < min_upward)
        v.y = min_upward;

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

        // ノックバック方向：自分(被ヒット側)からヒッターへのベクトルで押し出す
        float3 dir = GetOwner()->GetTranslate() - hitter->GetTranslate();
        float  len = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
        if(len > 0.0001f)
            dir = dir / len;
        else
            dir = float3{0.0f, 0.0f, 1.0f};

        // 投げ物の移動量（ComponentGrabbable::GetTranslation() を速度相当として使用）
        float3 throwVel = grabbable->GetTranslation();
        float  velMag   = sqrtf(throwVel.x * throwVel.x + throwVel.y * throwVel.y + throwVel.z * throwVel.z);

        // 調整係数（必要ならチューニング）
        float baseForce   = 5.0f;
        float damageBonus = grabbable->GetDamage() * 0.5f;
        float force       = velMag * baseForce + damageBonus;

        float3 knock = dir * force;
        // コンポーネントの min_upward を尊重
        knock.y = fmaxf(knock.y, min_upward);

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
        ImGui::SliderFloat(u8"減衰 (damping)", &damping, 0.0f, 1.0f);
        ImGui::SliderFloat(u8"最低上向き (min_upward)", &min_upward, 0.0f, 20.0f);
        ImGui::SliderFloat(u8"閾値 (velocity_epsilon)", &velocity_epsilon, 0.0f, 1.0f);

        if(ImGui::Button(u8"ノックバッククリア")) {
            knockback_velocity = float3{0.0f, 0.0f, 0.0f};
        }

        if(ImGui::Button(u8"削除")) {
            GetOwner()->RemoveComponent(shared_from_this());
        }
    }
    ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentKnockBack)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentKnockBack)
