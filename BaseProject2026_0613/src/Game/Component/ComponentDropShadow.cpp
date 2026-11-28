//---------------------------------------------------------------------------
//! @file   ComponentDropShadow.cpp
//! @brief  物の真下に影を描くコンポーネント
//---------------------------------------------------------------------------
#include <Game/Component/ComponentDropShadow.h>
#include <Game/Component/ComponentGrabbable.h>
#include <Game/Component/ComponentCharacterPhysics.h>
#include <Game/Object/PoittersPoint_Ground.h>
#include <System/Component/ComponentModel.h>

#include <algorithm>

void ComponentDropShadow::Init()
{
    __super::Init();

    SetName<Component>("Drop Shadow");

    // 地面や物を描いた後に描く(先に描くと、後から描かれた地面で上書きされて見えなくなる)
    SetPriority(ProcTiming::Draw, ProcPriority::LOWEST);
}

bool ComponentDropShadow::IsAirborne() const
{
    auto owner = GetOwner();

    // キャラクター: 足が地面に着いていない(ジャンプ・吹き飛び・投げられている)
    if(auto mover = owner->GetComponent<ComponentCharacterPhysics>()) {
        if(auto grabbable = owner->GetComponent<ComponentGrabbable>()) {
            // 誰かに持たれている間は持ち主の上にいるので出さない
            if(grabbable->GetState() == ComponentGrabbable::State::Reserved || grabbable->GetState() == ComponentGrabbable::State::Held)
                return false;
        }
        return !mover->IsGrounded();
    }

    // 投げ物: 投げられて飛んでいる、または(スポナーから落とされて)何にも支えられずに落ちている
    if(auto grabbable = owner->GetComponent<ComponentGrabbable>()) {
        auto state = grabbable->GetState();
        if(state == ComponentGrabbable::State::Thrown || state == ComponentGrabbable::State::Free)
            return !grabbable->IsSupported();
        return false;    // 持ち上げ中・手に持っている間は出さない
    }
    return false;
}

void ComponentDropShadow::Update()
{
    __super::Update();

    // 出し入れを滑らかにする(空中だけ表示する設定の時に、着地の瞬間にパッと消えないように)
    float target = (!only_airborne_ || IsAirborne()) ? 1.0f : 0.0f;
    float step   = fade_speed_ * GetDeltaTime();
    if(fade_ < target)
        fade_ = std::min(fade_ + step, target);
    else
        fade_ = std::max(fade_ - step, target);
}

bool ComponentDropShadow::SetupShape()
{
    auto owner = GetOwner();

    // キャラクターは当たり判定のカプセルの半径で丸い影にする(全員同じ大きさになる)
    // (モデルの大きさはアニメーションの姿勢で変わるため使わない)
    if(auto mover = owner->GetComponent<ComponentCharacterPhysics>()) {
        is_character_ = true;
        if(radius_ <= 0.0f)
            radius_ = mover->GetRadius() * 1.5f;
        return true;
    }

    // 物はモデルの頂点の範囲(モデル自身の座標)を覚えておき、描く時にモデルの向きで回して影の形にする
    auto model = owner->GetComponent<ComponentModel>();
    if(!model)
        return false;
    int handle = model->GetModel();
    if(handle < 0)
        return false;

    MATRIX old_matrix = MV1GetMatrix(handle);
    MV1SetMatrix(handle, MGetIdent());
    MV1SetupReferenceMesh(handle, -1, TRUE);
    MV1RefreshReferenceMesh(handle, -1, TRUE);
    MV1_REF_POLYGONLIST ref = MV1GetReferenceMesh(handle, -1, TRUE);
    MV1SetMatrix(handle, old_matrix);
    if(ref.PolygonNum <= 0)
        return false;

    float3 mn     = cast(ref.MinPosition);
    float3 mx     = cast(ref.MaxPosition);
    mesh_center_  = (mn + mx) * 0.5f;
    mesh_half_    = (mx - mn) * 0.5f;
    is_character_ = false;
    return true;
}

bool ComponentDropShadow::CalcShadowShape(float3& center, float2& axis1, float2& axis2) const
{
    auto owner = GetOwner();

    //-----------------------------------------------------------
    // キャラクター: 足元を中心にした円
    //-----------------------------------------------------------
    if(is_character_) {
        center = owner->GetTranslate();
        axis1  = {radius_, 0.0f};
        axis2  = {0.0f, radius_};
        return true;
    }

    //-----------------------------------------------------------
    // 物: モデルの箱(中心・各軸の半分の長さ)を今の向きで回し、真上から見た形に近い楕円にする
    //  箱の3つの軸(長さ付き)を地面へ投影した2Dベクトル v を足し合わせた行列 M = Σ v vᵀ の
    //  固有ベクトルが楕円の向き、固有値の平方根が楕円の半径になる
    //  (横倒しの丸太は細長く、立てた丸太は丸くなる)
    //-----------------------------------------------------------
    auto model = owner->GetComponent<ComponentModel>();
    if(!model)
        return false;

    const matrix world = model->GetWorldMatrix();    // モデルの向き・スケール込み
    const float3 ax    = world.axisX();
    const float3 ay    = world.axisY();
    const float3 az    = world.axisZ();

    center = world.translate() + ax * (float)mesh_center_.x + ay * (float)mesh_center_.y + az * (float)mesh_center_.z;

    const float2 v[3] = {
        {(float)ax.x * (float)mesh_half_.x, (float)ax.z * (float)mesh_half_.x},
        {(float)ay.x * (float)mesh_half_.y, (float)ay.z * (float)mesh_half_.y},
        {(float)az.x * (float)mesh_half_.z, (float)az.z * (float)mesh_half_.z},
    };
    float a = 0.0f, b = 0.0f, d = 0.0f;
    for(auto& p : v) {
        a += (float)p.x * (float)p.x;
        b += (float)p.x * (float)p.y;
        d += (float)p.y * (float)p.y;
    }

    // 2x2 対称行列の固有値・固有ベクトル
    float half_trace = (a + d) * 0.5f;
    float diff       = sqrtf(((a - d) * 0.5f) * ((a - d) * 0.5f) + b * b);
    float l1         = half_trace + diff;
    float l2         = std::max(half_trace - diff, 0.0f);

    float2 e1 = (fabsf(b) > 1e-6f) ? float2(l1 - d, b) : (a >= d ? float2(1.0f, 0.0f) : float2(0.0f, 1.0f));
    float  e1_len = sqrtf((float)e1.x * (float)e1.x + (float)e1.y * (float)e1.y);
    e1            = (e1_len > 1e-6f) ? e1 / e1_len : float2(1.0f, 0.0f);
    float2 e2     = {-(float)e1.y, (float)e1.x};

    // 物の真下に隠れてしまわないよう、少し大きめにする
    float r1 = std::max(sqrtf(l1) * size_scale_, 1.5f);
    float r2 = std::max(sqrtf(l2) * size_scale_, 1.5f);
    axis1    = e1 * r1;
    axis2    = e2 * r2;
    return true;
}

void ComponentDropShadow::Draw()
{
    __super::Draw();

    if(fade_ <= 0.0f)
        return;

    // 形の準備(モデルの読み込みが終わるまでは描かない)
    if(!has_shape_) {
        has_shape_ = SetupShape();
        if(!has_shape_)
            return;
    }

    auto ground = Scene::Object::Get<PoittersPoint::Ground>();
    if(!ground)
        return;

    float3 center;
    float2 axis1, axis2;
    if(!CalcShadowShape(center, axis1, axis2))
        return;

    // 高さは物の位置(キャラクターは足元)から測る
    float floor_y = ground->GetFieldFloorHeight();
    float height  = (float)GetOwner()->GetTranslate().y - floor_y;
    if(height < -5.0f)
        return;    // 床より下(場外へ落ちた等)には描かない

    // 高いほど小さく薄くする
    float t     = std::clamp(height / shrink_height_, 0.0f, 1.0f);
    float scale = 1.0f - (1.0f - min_scale_) * t;
    int   alpha = static_cast<int>(max_alpha_ * fade_ * (1.0f - 0.5f * t));
    if(alpha <= 0)
        return;
    axis1 *= scale;
    axis2 *= scale;

    // 半透明で、Zバッファは参照するが書き込まない(キャラクターや物に隠れるように、かつ後ろの物を消さないように)
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    SetWriteZBuffer3D(FALSE);
    SetUseLighting(FALSE);
    const int cull = GetUseBackCulling();
    SetUseBackCulling(DX_CULLING_NONE);    // 三角形の向きに関係なく描く

    const int    segments = 24;
    const float  y        = floor_y + floor_offset_;
    const float  cx       = (float)center.x;
    const float  cz       = (float)center.z;
    const VECTOR mid      = VGet(cx, y, cz);
    const u32    color    = GetColor(0, 0, 0);
    auto         point    = [&](float angle) {
        float c = cosf(angle), s = sinf(angle);
        return VGet(cx + (float)axis1.x * c + (float)axis2.x * s, y, cz + (float)axis1.y * c + (float)axis2.y * s);
    };
    for(int i = 0; i < segments; ++i) {
        float a0 = TAU * static_cast<float>(i) / segments;
        float a1 = TAU * static_cast<float>(i + 1) / segments;
        DrawTriangle3D(mid, point(a1), point(a0), color, TRUE);
    }

    // 描画設定を元に戻す
    SetUseBackCulling(cull);
    SetUseLighting(TRUE);
    SetWriteZBuffer3D(TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void ComponentDropShadow::GUI()
{
    __super::GUI();

    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode("Drop Shadow")) {
            ImGui::Text(u8"空中: %s / 濃さ: %.2f / 形: %s", IsAirborne() ? u8"〇" : u8"×", fade_, is_character_ ? u8"円(キャラクター)" : u8"楕円(モデルの形)");
            ImGui::Checkbox(u8"空中にいる時だけ表示", &only_airborne_);
            if(is_character_)
                ImGui::DragFloat(u8"影の半径", &radius_, 0.1f, 0.5f, 50.0f);
            else
                ImGui::DragFloat(u8"影の大きさの倍率", &size_scale_, 0.01f, 0.1f, 3.0f);
            ImGui::DragFloat(u8"一番濃い時の不透明度", &max_alpha_, 1.0f, 0.0f, 255.0f);
            ImGui::DragFloat(u8"小さくなりきる高さ", &shrink_height_, 1.0f, 1.0f, 500.0f);
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentDropShadow)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentDropShadow)
