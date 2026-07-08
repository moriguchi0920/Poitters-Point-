//---------------------------------------------------------------------------
//! @file   ComponentCollision.h
//! @brief  コリジョンコンポーネント(ベースクラス)
//---------------------------------------------------------------------------
#include <System/Component/ComponentCollision.h>
#include <System/Component/ComponentTransform.h>

#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCollisionLine.h>
#include <System/Component/ComponentCollisionBox.h>
#include <System/Component/ComponentModel.h>

#include <System/Utils/HelperLib.h>

#include <System/Object.h>

#include <algorithm>

namespace {
// 大きな値を取得する
float3 merge(float3 v1, float3 v2)
{
    float3 vh{v2.x, v2.y, v2.z};

    if(fabs(v1.x) > fabs(v2.x))
        vh.x = v1.x;

    if(fabs(v1.y) > fabs(v2.y))
        vh.y = v1.y;

    if(fabs(v1.z) > fabs(v2.z))
        vh.z = v1.z;

    return vh;
}
}    // namespace

ComponentCollision::ComponentCollision()
{
    // 複数設定可能とする
    SetStatus(Component::StatusBit::SameType, true);
}

void ComponentCollision::Construct(ObjectPtr owner)
{
    // 識別子設定
    assert(owner);
    owner_    = owner;
    auto cmps = owner->GetComponents<ComponentCollision>();
    int  max  = -1;
    for(ComponentCollisionPtr cmp : cmps) {
        if((int)cmp->collision_id_ > max)
            max = (int)cmp->collision_id_;
    }

    // 割り出したIDを collision_id_ に入れる
    collision_id_ = (u32)(max + 1);
}

//! @brief 当たった情報はコールバックで送られてくる
//! @param hitInfo 当たった情報
//! @details 当たった回数分ここに来ます
void ComponentCollision::OnHit(const HitInfo& hitInfo)
{
    auto obj = GetOwner();
#if 1
    auto  contact_dir  = normalize(obj->GetTranslate() - obj->GetOldWorldMatrix().translate());
    auto  gravity_dir  = normalize(now_gravity_);
    float vertical_dot = dot(contact_dir, float3(0.0f, 1.0f, 0.0f));    // Y軸との一致度

    if(hitInfo.hit_collision_->GetMass() < 0) {
        const float contact_threshold = 0.7f;                            // Y軸方向にある程度近い接触のみ処理する
        if(vertical_dot <= contact_threshold && gravity_dir.y < 0.0f)    // 下向き時のみ
        {
            now_gravity_  = float3(0.0f, 0.0f, 0.0f);
            prev_gravity_ = float3(0.0f, 0.0f, 0.0f);
            GetOwner()->SetGravity(now_gravity_);
        }
    }
#else
    // Staticな物質にぶつかった場合、gravity_を下げる
    if(hitInfo.hit_collision_->GetMass() < 0) {
        auto   vec = obj->GetTranslate() - obj->GetOldWorldMatrix().translate();
        float3 nvc = {0.0f, -1.0f, 0.0f};
        if(length(vec).x <= 0 || length(now_gravity_).x <= 0) {
            now_gravity_  = {0.0f, 0.0f, 0.0f};
            prev_gravity_ = {0.0f, 0.0f, 0.0f};
            GetOwner()->SetGravity(now_gravity_);
        }
        else {
            nvc           = normalize(vec);
            float d       = dot(normalize(now_gravity_), nvc);
            now_gravity_ *= ((1 - (d * d)) * 0.1f);
        }
    }
#endif
    obj->OnHit(hitInfo);
    if(obj->OnHitFunc)
        obj->OnHitFunc(hitInfo);
}

#if 0
void ComponentCollision::SetName(std::string_view name)
{
    name_ = name;
}

std::string_view ComponentCollision::GetName()
{
	return name_;
}
#endif

const char* const collisionGroupName[] = {
    "WALL",
    "GROUND",
    "PLAYER",
    "ENEMY ",
    "WEAPON",
    "ITEM  ",
    "CAMERA",
    "ETC",
};

void ComponentCollision::GUICollisionData(bool use_attach)
{
    // コリジョンデータ表示
    guiCollisionData();

    // アタッチが存在必要な時
    if(use_attach)
        guiCollisionDataAttach();
}

void ComponentCollision::LateUpdate()
{
    __super::LateUpdate();

    // モデルにアタッチしている場合
    // attach_node_matrix_ にモデルのNode位置を設定する
    if(attach_node_ >= 0) {
        attach_node_matrix_ = matrix::identity();
        if(auto mdl = GetOwner()->GetComponent<ComponentModel>()) {
            attach_node_matrix_ = MV1GetFrameLocalWorldMatrix(mdl->GetModel(), attach_node_);
        }
    }
#ifdef USE_JOLT_PHYSICS
#else
    if(use_gravity_) {
        float delta = GetDeltaTime();    // 実フレーム時間

        // 速度として重力を積分
        now_gravity_ += gravity_ * delta * k_gravity_;

        // 台形積分で位置変化分を算出（中間速度 × 時間）
        float3 avg_velocity = (prev_gravity_ + now_gravity_) * 0.5f;
        float3 displacement = avg_velocity * delta;

        // Owner に重力分の移動量をセット
        GetOwner()->SetGravity(displacement);

        // 状態更新
        prev_gravity_ = now_gravity_;
    }
#endif
}

void ComponentCollision::PostUpdate()
{
    //! 古いマトリクスを更新します
    old_transform_ = collision_transform_;
}

//! @brief GUI処理
void ComponentCollision::GUI()
{
}

void ComponentCollision::AttachToModel(int node)
{
    attach_node_ = node;
#ifdef USE_JOLT_PHYSICS
    if(GetRigidBody())
        GetRigidBody()->setGravityFactor(0.0f);
#endif USE_JOLT_PHYSICS
    if(attach_node_ >= 0) {
        attach_node_matrix_ = matrix::identity();
        if(auto mdl = GetOwner()->GetComponent<ComponentModel>()) {
            attach_node_matrix_ = MV1GetFrameLocalWorldMatrix(mdl->GetModel(), attach_node_);
        }
    }
}

void ComponentCollision::AttachToModel(const std::string_view name)
{
    if(auto mdl = GetOwner()->GetComponent<ComponentModel>()) {
        attach_node_ = mdl->GetNodeIndex(name);
#ifdef USE_JOLT_PHYSICS
        if(GetRigidBody())
            GetRigidBody()->setGravityFactor(0.0f);
#endif USE_JOLT_PHYSICS
    }
    if(attach_node_ >= 0) {
        attach_node_matrix_ = matrix::identity();
        if(auto mdl = GetOwner()->GetComponent<ComponentModel>()) {
            attach_node_matrix_ = MV1GetFrameLocalWorldMatrix(mdl->GetModel(), attach_node_);
        }
    }
}

void ComponentCollision::guiCollisionData()
{
    //----------------------------------------------------------------------------
    // GUIコリジョン基本情報
    //----------------------------------------------------------------------------
    auto str = u8"コリジョンタイプ : " + CollisionTypeName[(u32)collision_type_];
    ImGui::Text(str.c_str());
    ImGui::Separator();
    ImGui::Text(u8"コリジョングループ: ");
    ImGui::SameLine();
    int index = GetCollisionGroupIndex();
    if(ImGui::Combo(u8"##コリジョングループ", &index, collisionGroupName, (sizeof(collisionGroupName) / sizeof(collisionGroupName[0])))) {
        collision_group_ = static_cast<CollisionGroup>(1 << index);
    }
    ImGui::Separator();

    //----------------------------------------------------------------------------
    // ゲーム中のヒット表示
    //----------------------------------------------------------------------------
    //  ImGui::CheckboxFlags(u8"初期化済み", &collision_status_.get(), 1 << (u32)CollisionBit::Initialized);
    ImGui::CheckboxFlags(u8"ヒットしない", &collision_status_.get(), 1 << (u32)CollisionBit::DisableHit);
    ImGui::CheckboxFlags(u8"ゲーム中表示", &collision_status_.get(), 1 << (u32)CollisionBit::ShowInGame);
    ImGui::Separator();

    //----------------------------------------------------------------------------
    // Hitするグループの設定
    //----------------------------------------------------------------------------
    if(ImGui::TreeNode(u8"Hitするグループ【SetHitCollisionGroup()】")) {
        ImGui::CheckboxFlags("WALL", (u32*)&collision_hit_, (u32)CollisionGroup::WALL);
        ImGui::CheckboxFlags("GROUND", (u32*)&collision_hit_, (u32)CollisionGroup::GROUND);
        ImGui::CheckboxFlags("PLAYER", (u32*)&collision_hit_, (u32)CollisionGroup::PLAYER);
        ImGui::CheckboxFlags("ENEMY", (u32*)&collision_hit_, (u32)CollisionGroup::ENEMY);
        ImGui::CheckboxFlags("WEAPON", (u32*)&collision_hit_, (u32)CollisionGroup::WEAPON);
        ImGui::CheckboxFlags("ITEM", (u32*)&collision_hit_, (u32)CollisionGroup::ITEM);
        ImGui::CheckboxFlags("CAMERA", (u32*)&collision_hit_, (u32)CollisionGroup::CAMERA);
        ImGui::CheckboxFlags("ETC", (u32*)&collision_hit_, (u32)CollisionGroup::ETC);
        ImGui::TreePop();
    }

    //----------------------------------------------------------------------------
    // Hitするがオーバーラップするグループ
    //----------------------------------------------------------------------------
    if(ImGui::TreeNode(u8"オーバーラップするグループ【SetOverlapCollisionGroup()】")) {
        ImGui::CheckboxFlags("WALL", (u32*)&collision_overlap_, (u32)CollisionGroup::WALL);
        ImGui::CheckboxFlags("GROUND", (u32*)&collision_overlap_, (u32)CollisionGroup::GROUND);
        ImGui::CheckboxFlags("PLAYER", (u32*)&collision_overlap_, (u32)CollisionGroup::PLAYER);
        ImGui::CheckboxFlags("ENEMY", (u32*)&collision_overlap_, (u32)CollisionGroup::ENEMY);
        ImGui::CheckboxFlags("WEAPON", (u32*)&collision_overlap_, (u32)CollisionGroup::WEAPON);
        ImGui::CheckboxFlags("ITEM", (u32*)&collision_overlap_, (u32)CollisionGroup::ITEM);
        ImGui::CheckboxFlags("CAMERA", (u32*)&collision_overlap_, (u32)CollisionGroup::CAMERA);
        ImGui::CheckboxFlags("ETC", (u32*)&collision_overlap_, (u32)CollisionGroup::ETC);
        ImGui::TreePop();
    }
    ImGui::Separator();

    //----------------------------------------------------------------------------
    // 重力設定
    //----------------------------------------------------------------------------
    ImGui::Checkbox(u8"重力を使用【UseGravity()】", &use_gravity_);
    ImGui::DragFloat3(u8"重力加速度【Get/SetGravity()】", (float*)&gravity_);

    bool no_move = collision_mass_ < 0.001f;
    if(ImGui::Checkbox(u8"質量無限【mass=-1】", &no_move)) {
        if(no_move) {
            collision_mass_ = -1.0f;
        }
        else {
            collision_mass_ = 1.0f;    // デフォルトの質量に戻す
        }
    }
    if(!no_move) {
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
        if(ImGui::DragFloat(u8"質量【Get/SetMass()】", (float*)&collision_mass_, 0.001f, 0, 1000000.0f)) {
            collision_mass_ = std::min(1000000.0f, std::max(collision_mass_, 0.001f));
        }
        ImGui::PopItemWidth();
    }

    ImGui::DragFloat(u8"上に登れる量【Get/SetClimbLimit()】", &difficult_to_climb_);
}

void ComponentCollision::guiCollisionDataAttach()
{
    // モデルにアタッチしているかを調べる
    if(auto cmp = GetOwner()->GetComponent<ComponentModel>()) {
        // GUIでの AttachNodeの切り替えに対応させる
        bool attach = false;
        if(attach_node_ >= 0) {
            // 既にアタッチ済み
            attach = true;
            if(ImGui::Checkbox("AttachNode", &attach)) {
                if(!attach)
                    attach_node_ = -1;
            }
        }
        else {
            // アタッチしていない
            if(ImGui::Checkbox("AttachNode", &attach)) {
                if(attach)
                    attach_node_ = 0;
            }
        }

        // GUIノードを列挙します
        auto items = cmp->GetNodesNamePChar();
        if(ImGui::Combo("Node", &attach_node_, items.data(), (int)items.size())) {
            // 切り替えたとき
            collision_transform_ = matrix::identity();
        }
    }
}

//! @brief Capsule VS Sphere
//! @param col1 Capsuleコリジョン
//! @param col2 Sphere コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionCapsulePtr col1, ComponentCollisionSpherePtr col2)
{
    ComponentCollision::HitInfo info{};

    // 自分のコリジョン
    float3 cpos1 = col1->GetTranslate();
    float3 cpos2 = col1->GetVectorAxisY() * col1->GetHeight() + cpos1;
    float  cs    = 1.0f;    //< スケール

    float3 epos1;
    float  es = 1.0f;

    // モデルアタッチ
    if(col1->attach_node_ >= 0) {
        if(auto mdl = col1->GetOwner()->GetComponent<ComponentModel>()) {
            cpos1 = mul(float4(cpos1, 1), col1->attach_node_matrix_).xyz;
            cpos2 = mul(float4(cpos2, 1), col1->attach_node_matrix_).xyz;
            cpos2 = normalize(cpos2 - cpos1) * col1->GetHeight() + cpos1;
        }
    }
    else {
        // ComponentTransform(オブジェクト姿勢)
        if(auto cmp = col1->GetOwner()->GetComponent<ComponentTransform>()) {
            // 高さに回転とスケールを掛け合わせる
            cpos1 = mul(float4(cpos1, 1), cmp->GetMatrix()).xyz;
            cpos2 = mul(float4(cpos2, 1), cmp->GetMatrix()).xyz;
            // 半径はXZで平均としておく
            cs = (length(cmp->GetMatrix().axisX()) + length(cmp->GetMatrix().axisZ())) / 2;
        }
    }

    // モデルアタッチ
    if(col2->attach_node_ >= 0) {
        if(auto mdl = col1->GetOwner()->GetComponent<ComponentModel>()) {
            epos1 = col2->GetTranslate();
            epos1 = mul(float4(epos1, 1), col2->attach_node_matrix_).xyz;
        }
    }
    else {
        // ComponentTransform(オブジェクト姿勢)
        if(auto cmp = col2->GetOwner()->GetComponent<ComponentTransform>()) {
            epos1 = mul(col2->GetMatrix(), cmp->GetMatrix())._41_42_43;
            // pos1 = mul( float4( pos1, 0 ) , cmp->GetMatrix() ).xyz;
            // pos1 += cmp->GetTranslate().xyz;
            float sx = length(cmp->GetVectorAxisX());
            float sy = length(cmp->GetVectorAxisY());
            float sz = length(cmp->GetVectorAxisZ());
            es       = (sx + sy + sz) / 3.0f;
        }
    }

    float  cr = col1->GetRadius() * cs;
    float3 cv = normalize(cpos1 - cpos2);
    VECTOR c1 = cast(cpos1 - cv * cr);
    VECTOR c2 = cast(cpos2 + cv * cr);

    float  er = col2->GetRadius() * es;
    VECTOR e1 = cast(epos1);

    // 跳ね返り点が欲しいため、HitCheck_Capsule_Capsuleは使わない
    SEGMENT_POINT_RESULT result;
    Segment_Point_Analyse(&c1, &c2, &e1, &result);

    if(result.Seg_Point_MinDist_Square < (cr + er) * (cr + er)) {
        // 線と線で一番近くなる点を求め、ベクトル化する
        // 最も近い点
        float3 c0  = cast(result.Seg_MinDist_Pos);
        float3 e0  = cast(e1);
        float3 vec = e0 - c0;    // 調べたほうの跳ね返りの方向(100%)
        float  len = length(vec);
        if(abs(len) <= abs(len) * FLT_EPSILON) {
            // 全く同じ位置にいる場合はz移動する形にしておく
            vec = {0, 0, 1};
        }

        float3 vs  = normalize(vec) * (cr + er);
        vec       -= vs;

        // このpush_は、調べたほうの押し戻し方向100%で作成する
        info.push_         = vec;
        info.hit_          = true;
        info.hit_position_ = (e0 + c0) * 0.5f;
    }

    return info;
}

//! @brief Sphere VS Capsule
//! @param col1 Sphereコリジョン
//! @param col2 Capsule コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionSpherePtr col1, ComponentCollisionCapsulePtr col2)
{
    auto hit  = isHit(col2, col1);
    hit.push_ = -hit.push_;    // push方向を反対にする
    return hit;
}

//! @brief Capsule VS Capsule
//! @param col1 Capsuleコリジョン
//! @param col2 Capsule コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionCapsulePtr col1, ComponentCollisionCapsulePtr col2)
{
    ComponentCollision::HitInfo info{};

    // 自分のコリジョン
    float3 cpos1 = col1->GetTranslate();
    float3 cpos2 = normalize(col1->GetVectorAxisY()) * col1->GetHeight() + cpos1;
    float  cs    = 1.0f;    //< スケール

    // モデルアタッチ
    if(col1->attach_node_ >= 0) {
        if(auto mdl = col1->GetOwner()->GetComponent<ComponentModel>()) {
            cpos1 = mul(float4(cpos1, 1), col1->attach_node_matrix_).xyz;
            cpos2 = mul(float4(cpos2, 1), col1->attach_node_matrix_).xyz;
            cpos2 = normalize(cpos2 - cpos1) * col1->GetHeight() + cpos1;
        }
    }
    else {
        // ComponentTransform(オブジェクト姿勢)
        if(auto cmp = col1->GetOwner()->GetComponent<ComponentTransform>()) {
            auto& mtx = cmp->GetWorldMatrix();
            // 高さに回転とスケールを掛け合わせる
            cpos1 = mul(float4(cpos1, 1), mtx).xyz;
            cpos2 = mul(float4(cpos2, 1), mtx).xyz;
            // 半径はXZで平均としておく
            cs = (length(mtx.axisX()) + length(mtx.axisZ())) / 2;
        }
    }

    // 相手のコリジョン
    float3 epos1 = col2->GetTranslate();
    float3 epos2 = normalize(col2->GetVectorAxisY()) * col2->GetHeight() + epos1;
    float  es    = 1.0f;    //< スケール

    // モデルアタッチ
    if(col2->attach_node_ >= 0) {
        if(auto mdl = col2->GetOwner()->GetComponent<ComponentModel>()) {
            epos1 = mul(float4(epos1, 1), col2->attach_node_matrix_).xyz;
            epos2 = mul(float4(epos2, 1), col2->attach_node_matrix_).xyz;
            epos2 = normalize(epos2 - epos1) * col2->GetHeight() + epos1;
        }
    }
    else {
        // ComponentTransform(オブジェクト姿勢)
        if(auto cmp = col2->GetOwner()->GetComponent<ComponentTransform>()) {
            auto& mtx = cmp->GetWorldMatrix();
            // 高さに回転とスケールを掛け合わせる
            epos1 = mul(float4(epos1, 1), mtx).xyz;
            epos2 = mul(float4(epos2, 1), mtx).xyz;
            // 半径はXZで平均としておく
            es = (length(mtx.axisX()) + length(mtx.axisZ())) / 2;
        }
    }

    float  cr = col1->GetRadius() * cs;
    float3 cv = normalize(cpos1 - cpos2);
    VECTOR c1 = cast(cpos1 - (cv * cr));
    VECTOR c2 = cast(cpos2 + (cv * cr));
    float  er = col2->GetRadius() * es;
    float3 ev = normalize(epos1 - epos2);
    VECTOR e1 = cast(epos1 - (ev * er));
    VECTOR e2 = cast(epos2 + (ev * er));

    // 跳ね返り点が欲しいため、HitCheck_Capsule_Capsuleは使わない
    SEGMENT_SEGMENT_RESULT result;
    Segment_Segment_Analyse(&c1, &c2, &e1, &e2, &result);

#if 0    // 当たり不具合チェック用
	if ( col1->attach_node_ )
		DrawCapsule3D( c1, c2, cr, 10, GetColor( 0, 0, 255 ), GetColor( 0, 0, 255 ), FALSE );

	if ( col2->attach_node_ )
		DrawCapsule3D( e1, e2, er, 10, GetColor( 0, 0, 255 ), GetColor( 0, 0, 255 ), FALSE );
#endif

    if(result.SegA_SegB_MinDist_Square < (cr + er) * (cr + er)) {
        // 線と線で一番近くなる点を求め、ベクトル化する
        // 最も近い点
        float3 c0  = cast(result.SegA_MinDist_Pos);
        float3 e0  = cast(result.SegB_MinDist_Pos);
        float3 vec = e0 - c0;    // 調べたほうの跳ね返りの方向(100%)

        float len = length(vec);
        if(abs(len) <= abs(len) * FLT_EPSILON) {
            // 全く同じ位置にいる場合はz移動する形にしておく
            vec = {0, 0, 1};
        }
        float3 vs  = normalize(vec) * (cr + er);
        vec       -= vs;

        // このpush_は、調べたほうの押し戻し方向100%で作成する
        info.push_         = vec;
        info.hit_          = true;
        info.hit_position_ = (e0 + c0) * 0.5f;
    }

    return info;
}

//! @brief Sphere VS Sphere
//! @param col1 Sphereコリジョン
//! @param col2 Sphere コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionSpherePtr col1, ComponentCollisionSpherePtr col2)
{
    ComponentCollision::HitInfo info{};

    float3 pos1;
    float  scale1 = 1.0f;

    // モデルアタッチ
    if(col1->attach_node_ >= 0) {
        if(auto mdl = col1->GetOwner()->GetComponent<ComponentModel>()) {
            pos1 = col1->GetTranslate();
            pos1 = mul(float4(pos1, 1), col1->attach_node_matrix_).xyz;
        }
    }
    else {
        if(auto cmp = col1->GetOwner()->GetComponent<ComponentTransform>()) {
            pos1 = mul(col1->GetMatrix(), cmp->GetWorldMatrix())._41_42_43;
            // pos1 = mul( float4( pos1, 0 ) , cmp->GetMatrix() ).xyz;
            // pos1 += cmp->GetTranslate().xyz;
            float sx = length(cmp->GetVectorAxisX());
            float sy = length(cmp->GetVectorAxisY());
            float sz = length(cmp->GetVectorAxisZ());
            scale1   = (sx + sy + sz) / 3.0f;
        }
    }

    float3 pos2;
    float  scale2 = 1.0f;

    // モデルアタッチ
    if(col2->attach_node_ >= 0) {
        if(auto mdl = col2->GetOwner()->GetComponent<ComponentModel>()) {
            pos2 = col2->GetTranslate();
            pos2 = mul(float4(pos2, 1), col2->attach_node_matrix_).xyz;
        }
    }
    else {
        if(auto cmp = col2->GetOwner()->GetComponent<ComponentTransform>()) {
            pos2 = mul(col2->GetMatrix(), cmp->GetWorldMatrix())._41_42_43;
            // pos2 = mul( float4( pos2, 0 ), cmp->GetMatrix() ).xyz;
            // pos2 += cmp->GetTranslate().xyz;
            float sx = length(cmp->GetVectorAxisX());
            float sy = length(cmp->GetVectorAxisY());
            float sz = length(cmp->GetVectorAxisZ());
            scale2   = (sx + sy + sz) / 3.0f;
        }
    }

    if(HitCheck_Sphere_Sphere(cast(pos1), col1->GetRadius() * scale1, cast(pos2), col2->GetRadius() * scale2)) {
        // 中間地点を当たった場所にする
        info.hit_          = true;
        info.hit_position_ = (pos1 - pos2) * 0.5f + pos2;

        // 押し出し方向
        float3 distance = pos1 - pos2;
        float  len      = length(distance);
        if(abs(len) <= abs(len) * FLT_EPSILON) {
            // 全く同じ位置にいる場合はz移動する形にしておく
            distance = {0, 0, 1};
        }

        float3 vec     = normalize(distance);
        float  reallen = col1->GetRadius() * scale1 + col2->GetRadius() * scale2;
        vec            = vec * (reallen - len);

        // このpush_は、調べたほうの押し戻し方向100%で作成する
        info.push_ = vec;
    }

    return info;
}

//! @brief Model VS Sphere
//! @param col1 Model コリジョン
//! @param col2 Sphereコリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionSpherePtr col1, ComponentCollisionModelPtr col2)
{
    ComponentCollision::HitInfo info{};

    // モデルが存在していない
    auto mdl = col2->GetOwner()->GetComponent<ComponentModel>();
    if(mdl == nullptr)
        return info;

    float3 opos{};
    float3 cpos{};
    if(col1->attach_node_ >= 0) {
        cpos = mul(float4(col1->GetTranslate(), 1), col1->attach_node_matrix_).xyz;
        opos = cpos;
    }
    else {
        // オブジェクト位置に対するコリジョン(1フレーム前)
        opos = mul(float4(col1->GetTranslate(), 1), col1->GetOwner()->GetOldWorldMatrix()).xyz;

        // オブジェクト位置に対するコリジョン
        cpos = mul(float4(col1->GetTranslate(), 1), col1->GetOwner()->GetWorldMatrix()).xyz;

        // 実際の移動できる量にする
        auto  move  = cpos - opos;
        float movey = move.y;
        move.y      = 0;

        if(col1->IsUseGravity()) {
            move   = col2->checkMovement(opos, move, 1.0f);
            move.y = movey;
            cpos   = opos + move;

            // 当たりからキャラの位置を求める
            auto rot                      = col1->GetOwner()->GetWorldMatrix();
            rot._41_42_43                 = float3{0.0f, 0.0f, 0.0f};
            auto col1r                    = mul(float4(col1->GetTranslate(), 1), rot).xyz;
            col1->GetOwner()->Translate() = cpos - col1r;
        }
    }

    float scale = 1.0f;
    if(col1->attach_node_ < 0) {
        auto  mat = col1->GetWorldMatrix();
        float sx  = length(mat.axisVectorX());
        float sy  = length(mat.axisVectorY());
        float sz  = length(mat.axisVectorZ());
        scale     = (sx + sy + sz) / 3.0f;
    }

    float radius = col1->GetRadius() * scale;
#if 0
	MV1_COLL_RESULT_POLY hit_poly{};
	float3				 bottom = cpos - float3{ 0, col1->GetRadius() * scale, 0 };
	float3				 top	= opos + float3{ 0, col1->GetRadius() * scale, 0 };
	hit_poly					= MV1CollCheck_Line( mdl->GetModel(), -1, cast( top ), cast( bottom ) );
	DrawLine3D( cast( top ), cast( bottom ), 0xff00ffff );

	if( hit_poly.HitFlag != 0 )
	{
		float3 pos = cast( hit_poly.HitPosition );

		// 半径分押し戻し
		float3 vec = pos - bottom;

		// 一旦1つ目の当たりだけで返してみる
		// このpush_は、調べたほうの押し戻し方向100%で作成する
		info.push_		   = vec;
		info.hit_		   = true;
		info.hit_position_ = pos;
	}
#endif

#if 0
	// 球当たりの確認
	{
		// 戻し量
		float3 vh = 0;

		MV1_COLL_RESULT_POLY_DIM hit_poly_dim{};

		float3 oc = opos;
		float3 cc = cpos - now_gravity_;

		//DrawCapsule3D( cast( oc ), cast( cc ), radius, 10, GetColor( 0, 0, 255 ), GetColor( 0, 0, 255 ), FALSE );

		hit_poly_dim = MV1CollCheck_Capsule( mdl->GetModel(), -1, cast( oc ), cast( cc ), radius );
		for( int i = 0; i < hit_poly_dim.HitNum; i++ )
		{
			SEGMENT_TRIANGLE_RESULT result{};

			VECTOR v1 = cast( oc );
			VECTOR v2 = cast( cc );
			DxLib::Segment_Triangle_Analyse( &v1,
											 &v2,
											 &hit_poly_dim.Dim[ i ].Position[ 0 ],
											 &hit_poly_dim.Dim[ i ].Position[ 1 ],
											 &hit_poly_dim.Dim[ i ].Position[ 2 ],
											 &result );

			float3 line_pos = cast( result.Seg_MinDist_Pos );
			float3 tri_pos	= cast( result.Tri_MinDist_Pos );

			// カプセルへの戻し方向
			if( HelperLib::Math::NearlyEqual( length( line_pos - tri_pos ), 0 ) )
			{
				float3 v = ( tri_pos - cpos );
				vh		 = merge( vh, v );
			}
			else
			{
				float3 vec = normalize( line_pos - tri_pos );
				// 戻し量
				float  len = radius - length( line_pos - tri_pos );
				float3 v   = vec * len;
				vh		   = merge( vh, v );
			}
		}

		if( hit_poly_dim.HitNum > 0 )
		{
			info.push_		   = vh;
			info.hit_		   = true;
			info.hit_position_ = cpos - vh;
		}
		MV1CollResultPolyDimTerminate( hit_poly_dim );
	}
#endif
#if 1
    // 当たりの確認(通常)
    {
        // 戻し量
        float3 vh = 0;

        MV1_COLL_RESULT_POLY_DIM hit_poly_dim{};

        float3 ocenter = opos + info.push_;
        float3 ncenter = cpos + info.push_;

        // DrawCapsule3D( cast( ocenter ), cast( ncenter ), radius, 10, GetColor( 0, 0, 255 ), GetColor( 0, 0, 255 ), FALSE );

        // スピードは一旦無視。面倒なのでカプセルをそのまま利用
        hit_poly_dim = MV1CollCheck_Capsule(mdl->GetModel(), -1, cast(ocenter), cast(ncenter), radius);
        for(int i = 0; i < hit_poly_dim.HitNum; i++) {
            SEGMENT_TRIANGLE_RESULT result{};

            VECTOR v1 = cast(ocenter);
            VECTOR v2 = cast(ncenter);
            DxLib::Segment_Triangle_Analyse(
                &v1, &v2, &hit_poly_dim.Dim[i].Position[0], &hit_poly_dim.Dim[i].Position[1], &hit_poly_dim.Dim[i].Position[2], &result);

            float3 line_pos = cast(result.Seg_MinDist_Pos);
            float3 tri_pos  = cast(result.Tri_MinDist_Pos);
            float3 tri_nml  = cast(hit_poly_dim.Dim[i].Normal);

            // カプセルへの戻し方向
            if(HelperLib::Math::NearlyEqual(length(line_pos - tri_pos), 0)) {
                float3 v = (tri_pos - ncenter);
                v        = v + (tri_nml * radius);

                vh = merge(vh, v);
            }
            else {
                float3 vec = normalize(line_pos - tri_pos);
                // 戻し量
                float  len = radius - length(line_pos - tri_pos);
                float3 v   = vec * len;
                vh         = merge(vh, v);
            }
        }
        if(hit_poly_dim.HitNum > 0) {
            info.push_         += vh;
            info.hit_           = true;
            info.hit_position_ += (cpos - vh);
        }
        MV1CollResultPolyDimTerminate(hit_poly_dim);
    }
#endif
    return info;
}

//! @brief Model VS Capsule
//! @param col1 Modelコリジョン
//! @param col2 Capsule コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionModelPtr col1, ComponentCollisionLinePtr col2)
{
    auto hit  = isHit(col2, col1);
    hit.push_ = -hit.push_;    // push方向を反対にする
    return hit;
}

//! @brief Sphere VS Capsule
//! @param col1 Sphereコリジョン
//! @param col2 Capsule コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionModelPtr col1, ComponentCollisionSpherePtr col2)
{
    auto hit  = isHit(col2, col1);
    hit.push_ = -hit.push_;    // push方向を反対にする
    return hit;
}

//! @brief Model VS Capsule
//! @param col1 Modelコリジョン
//! @param col2 Capsule コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionCapsulePtr col1, ComponentCollisionModelPtr col2)
{
    ComponentCollision::HitInfo info{};

    // モデルが存在していない
    auto mdl = col2->GetOwner()->GetComponent<ComponentModel>();
    if(mdl == nullptr)
        return info;

    float3 opos{};
    float3 cpos{};
    float3 opos1{};
    float3 cpos1{};
    auto   trans = col1->GetWorldMatrix();

    if(col1->attach_node_ >= 0) {
        cpos  = trans.translate();
        cpos1 = normalize((float3&)trans.axisVectorY()) * col1->GetHeight() + cpos;

        opos  = cpos;
        opos1 = cpos1;
    }
    else {
        // オブジェクト位置に対するコリジョン(1フレーム前)
        opos = mul(float4(col1->GetTranslate(), 1), col1->GetOwner()->GetOldWorldMatrix()).xyz;

        // オブジェクト位置に対するコリジョン
        cpos = mul(float4(col1->GetTranslate(), 1), col1->GetOwner()->GetWorldMatrix()).xyz;

        // 実際の移動できる量にする
        auto  move  = cpos - opos;
        float movey = move.y;
        move.y      = 0;

        if(col1->IsUseGravity()) {
            move   = col2->checkMovement(opos, move, 1.0f);
            move.y = movey;
            cpos   = opos + move;

            // 当たりからキャラの位置を求める
            auto rot                      = col1->GetOwner()->GetWorldMatrix();
            rot._41_42_43                 = float3{0.0f, 0.0f, 0.0f};
            auto col1r                    = mul(float4(col1->GetTranslate(), 1), rot).xyz;
            col1->GetOwner()->Translate() = cpos - col1r;
        }

        // その頭の位置
        opos1 = mul(float4(col1->GetTranslate() + float3(0, col1->GetHeight(), 0), 1), col1->GetOwner()->GetOldWorldMatrix()).xyz;
        cpos1 = mul(float4(col1->GetTranslate() + float3(0, col1->GetHeight(), 0), 1), col1->GetOwner()->GetWorldMatrix()).xyz;
    }

    float scale = 1.0f;
    if(col1->attach_node_ < 0) {
        auto  mat = col1->GetWorldMatrix();
        float sx  = length(mat.axisVectorX());
        float sy  = length(mat.axisVectorY());
        float sz  = length(mat.axisVectorZ());
        scale     = (sx + sy + sz) / 3.0f;
    }

    float radius = col1->GetRadius() * scale;

#if 1
    {
        MV1_COLL_RESULT_POLY hit_poly{};
        float3               bottom = cpos;     //-float3{ 0, col1->GetRadius() * scale, 0 };
        float3               top    = opos1;    // bottom + float3{ 0, col1->GetRadius() * scale * 2, 0 };
        hit_poly                    = MV1CollCheck_Line(mdl->GetModel(), -1, cast(top), cast(bottom));

        if(hit_poly.HitFlag != 0) {
            float3 pos = cast(hit_poly.HitPosition);

            // 半径分押し戻し
            //float3 vec = pos - cpos;
            float3 vec = cast(hit_poly.Normal) * length(pos - cpos);

            // 一旦1つ目の当たりだけで返してみる
            // このpush_は、調べたほうの押し戻し方向100%で作成する
            info.push_         = vec;
            info.hit_          = true;
            info.hit_position_ = pos;
        }
    }
    // 上の球当たりの確認
    {
        // 戻し量
        float3 vh = 0;

        MV1_COLL_RESULT_POLY_DIM hit_poly_dim{};

        float3 oc = opos1 - float3(0, radius, 0) + info.push_;
        float3 cc = cpos1 - float3(0, radius, 0) + info.push_;

        // DrawCapsule3D( cast( oc ), cast( cc ), radius, 10, GetColor( 0, 0, 255 ), GetColor( 0, 0, 255 ), FALSE );

        hit_poly_dim = MV1CollCheck_Capsule(mdl->GetModel(), -1, cast(oc), cast(cc), radius);
        for(int i = 0; i < hit_poly_dim.HitNum; i++) {
            SEGMENT_TRIANGLE_RESULT result{};

            VECTOR v1 = cast(oc);
            VECTOR v2 = cast(cc);
            DxLib::Segment_Triangle_Analyse(
                &v1, &v2, &hit_poly_dim.Dim[i].Position[0], &hit_poly_dim.Dim[i].Position[1], &hit_poly_dim.Dim[i].Position[2], &result);

            float3 line_pos = cast(result.Seg_MinDist_Pos);
            float3 tri_pos  = cast(result.Tri_MinDist_Pos);

            // カプセルへの戻し方向
            if(HelperLib::Math::NearlyEqual(length(line_pos - tri_pos), 0)) {
                float3 v = (tri_pos - cpos1);
                vh       = merge(vh, v);
            }
            else {
                float3 vec = normalize(line_pos - tri_pos);
                // 戻し量
                float  len = radius - length(line_pos - tri_pos);
                float3 v   = vec * len;
                vh         = merge(vh, v);
            }
        }
        vh.y = 0;
        if(hit_poly_dim.HitNum > 0) {
            info.push_         += vh;
            info.hit_           = true;
            info.hit_position_ += (cpos - vh);
        }
        MV1CollResultPolyDimTerminate(hit_poly_dim);
    }
#endif

#if 1
    // 当たりの確認(通常カプセル)
    float3 nyopos = opos;
    float3 nycpos = cpos;
    nyopos.y      = 0;
    nycpos.y      = 0;
    //if( !HelperLib::Math::NearlyEqual( length( nyopos - nycpos ), 0 ) )
    {
        // 戻し量
        float3 vh = 0;

        MV1_COLL_RESULT_POLY_DIM hit_poly_dim{};

        float3 bottom_e = cpos + info.push_ + float3(0, radius, 0);
        float3 top_e    = cpos1 + info.push_ - float3(0, radius, 0);

        float3 bottom_s = opos + info.push_ + float3(0, radius, 0);
        float3 top_s    = opos1 + info.push_ - float3(0, radius, 0);

        float3 bottom_len = (bottom_e - bottom_s);
        float3 top_len    = (top_e - top_s);
        float3 front      = normalize(bottom_len);

        //printfDx( "LEN: %2.2f\n", (float)length( bottom_len ) );

        //for( int j = 0; j < count; j++ )
        {
            float3 bottomx = bottom_e + vh;    //+bottom_len*( (float)( j + 1 ) / (float)count ) + vh;
            float3 topx    = top_e + vh;       // + top_len * ( (float)( j + 1 ) / (float)count ) + vh;

            //DrawCapsule3D( cast( topx ), cast( bottomx ), radius, 10, GetColor( 0, 0, 255 ), GetColor( 0, 0, 255 ), TRUE );

            hit_poly_dim = MV1CollCheck_Capsule(mdl->GetModel(), -1, cast(topx), cast(bottomx), radius);
            for(int i = 0; i < hit_poly_dim.HitNum; i++) {
                SEGMENT_TRIANGLE_RESULT result{};

                VECTOR v1 = cast(topx);
                VECTOR v2 = cast(bottomx);
                DxLib::Segment_Triangle_Analyse(
                    &v1, &v2, &hit_poly_dim.Dim[i].Position[0], &hit_poly_dim.Dim[i].Position[1], &hit_poly_dim.Dim[i].Position[2], &result);

                float3 line_pos = cast(result.Seg_MinDist_Pos);
                float3 tri_pos  = cast(result.Tri_MinDist_Pos);

                float  per       = length(line_pos - topx) / length(bottomx - topx);
                float3 oline_pos = (bottom_s - top_s) * per + top_s;
                float  len       = radius - length(line_pos - tri_pos);
                bool   over      = dot(line_pos - tri_pos, oline_pos - tri_pos) < (float1)0.0f;
    #if 0
				float3 l1 = cast( hit_poly_dim.Dim[ i ].Position[ 0 ] ) - cast( hit_poly_dim.Dim[ i ].Position[ 2 ] );
				float3 l2 = cast( hit_poly_dim.Dim[ i ].Position[ 1 ] ) - cast( hit_poly_dim.Dim[ i ].Position[ 2 ] );
				float3 n = cross( normalize(l1), normalize(l2) );
    #else
                float3 n = {0, 0, 0};
                if(!HelperLib::Math::NearlyEqual(length(line_pos - tri_pos), 0))
                    n = normalize(line_pos - tri_pos);
    #endif
                if(over) {
                    len = radius + length(line_pos - tri_pos);
                    n   = -n;
                }
                //if( dot( front, n ) > (float1)0.3f )
                //	continue;

                // カプセルへの戻し方向
                //if( HelperLib::Math::NearlyEqual( length( line_pos - tri_pos ), 0 ) )
                float3 v = n * len;
                vh       = merge(vh, v);
                if(vh.y > difficult_to_climb_ || vh.y < 0) {
                    vh.y  = 0;
                    vh.x *= 1.25f;
                    vh.z *= 1.25f;
                }
            }

            int hit = hit_poly_dim.HitNum;
            if(hit > 0) {
                info.push_         += vh;
                info.hit_           = true;
                info.hit_position_ += (cpos - vh);
            }
            MV1CollResultPolyDimTerminate(hit_poly_dim);
        }
    }
#endif

#if 1
    // 下あたり
    {
        MV1_COLL_RESULT_POLY hit_poly{};
        float3               bottom = cpos + info.push_;    //-float3{ 0, col1->GetRadius() * scale, 0 };
        float3               top    = opos1;                //+ info.push_;	 // bottom + float3{ 0, col1->GetRadius() * scale * 2, 0 };
        hit_poly                    = MV1CollCheck_Line(mdl->GetModel(), -1, cast(top), cast(bottom));

        if(hit_poly.HitFlag != 0) {
            float3 pos = cast(hit_poly.HitPosition);

            // 半径分押し戻し
            float3 vec = pos - bottom;

            // 一旦1つ目の当たりだけで返してみる
            // このpush_は、調べたほうの押し戻し方向100%で作成する
            info.push_         += vec;
            info.hit_           = true;
            info.hit_position_ += pos;
        }
    }
#endif

#if 0
	// 上あたり
	{
		MV1_COLL_RESULT_POLY hit_poly{};
		float3				 bottom = opos + info.push_;	 //-float3{ 0, col1->GetRadius() * scale, 0 };
		float3				 top	= cpos1 + info.push_;	 // bottom + float3{ 0, col1->GetRadius() * scale * 2, 0 };
		hit_poly					= MV1CollCheck_Line( mdl->GetModel(), -1, cast( bottom ), cast( top ) );

		if( hit_poly.HitFlag != 0 )
		{
			float3 pos = cast( hit_poly.HitPosition );

			// 半径分押し戻し
			float3 vec = pos - top;

			// 一旦1つ目の当たりだけで返してみる
			// このpush_は、調べたほうの押し戻し方向100%で作成する
			info.push_		   += vec;
			info.hit_			= true;
			info.hit_position_ += pos;
		}
	}
#endif
    return info;
}

//! @brief Sphere VS Capsule
//! @param col1 Sphereコリジョン
//! @param col2 Capsule コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionSpherePtr col1, ComponentCollisionLinePtr col2)
{
    auto hit  = isHit(col2, col1);
    hit.push_ = -hit.push_;    // push方向を反対にする
    return hit;
}

ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionCapsulePtr col1, ComponentCollisionLinePtr col2)
{
    auto hit  = isHit(col2, col1);
    hit.push_ = -hit.push_;    // push方向を反対にする
    return hit;
}

// Line(col1)とSphere(col2)の当たりをチェックします
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionLinePtr col1, ComponentCollisionSpherePtr col2)
{
    // 当たり情報
    ComponentCollision::HitInfo info{};

    // ラインの情報
    auto line_owner = col1->GetOwner();
    auto line_comp  = line_owner->GetComponent<ComponentCollisionLine>();
    auto line       = line_comp->GetWorldLine();

    // 球の情報
    auto sphere_owner = col2->GetOwner();
    if(sphere_owner == nullptr)
        return info;

    auto sphere = sphere_owner->GetComponent<ComponentCollisionSphere>();
    if(sphere == nullptr)
        return info;

    auto  pos    = sphere->GetWorldMatrix().translate();
    float radius = sphere->GetRadius();

    SEGMENT_POINT_RESULT result;

    // ライン情報と点情報を用意する
    VECTOR start  = cast(line[0]);
    VECTOR end    = cast(line[1]);
    VECTOR center = cast(pos);

    // ラインと点により分析する
    Segment_Point_Analyse(&start, &end, &center, &result);

    // 点にライン上で一番近い位置を割り出す
    auto point = cast(result.Seg_MinDist_Pos);
    // その位置から点までの距離を割り出す
    float len = length(point - pos);

    // その距離が半径よりも小さければ当たっている
    if(len <= radius) {
        auto vec           = normalize(line[0] - line[1]);
        info.hit_          = true;
        info.hit_position_ = point + vec * acos(len / radius) / 0.5f * DX_PI_F;
    }

    // 判定結果を返す
    return info;
}

// Line(col1)とCapsule(col2)の当たりをチェックします
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionLinePtr col1, ComponentCollisionCapsulePtr col2)
{
    ComponentCollision::HitInfo info{};

    // ラインの情報
    auto line_owner = col1->GetOwner();
    auto line_comp  = line_owner->GetComponent<ComponentCollisionLine>();
    auto line       = line_comp->GetWorldLine();

    // カプセルの情報
    auto capsule_owner = col2->GetOwner();
    if(capsule_owner == nullptr)
        return info;

    auto capsule = capsule_owner->GetComponent<ComponentCollisionCapsule>();
    if(capsule == nullptr)
        return info;

    // カプセルの状態を分析
    auto  mat    = capsule->GetWorldMatrix();
    auto  pos    = mat.translate();
    float height = capsule->GetHeight();
    float radius = capsule->GetRadius();
    auto  pos1   = pos + height * mat.axisY();

    SEGMENT_SEGMENT_RESULT result;

    // ライン情報とカプセルの縦ラインを互いにチェックする
    VECTOR l1_start = cast(line[0]);
    VECTOR l1_end   = cast(line[1]);
    VECTOR l2_start = cast(pos);
    VECTOR l2_end   = cast(pos1);

    // ラインどうしの状態を分析
    Segment_Segment_Analyse(&l1_start, &l1_end, &l2_start, &l2_end, &result);

    // ライン上の一番近い点を割り出し、その距離を確認する
    auto  point     = cast(result.SegA_MinDist_Pos);
    auto  cap_point = cast(result.SegB_MinDist_Pos);
    float len       = length(point - cap_point);

    // 片方はラインなのであくまで半径よりもこれが小さいかで
    // 当たったかどうかが判定できる
    if(len <= radius) {
        // 当たったら情報を入れておく
        auto vec           = normalize(line[0] - line[1]);
        info.hit_          = true;
        info.hit_position_ = point + vec * acos(len / radius) / 0.5f * DX_PI_F;
    }

    // 判定結果を返す
    return info;
}

// ライン(col1)とモデル(col2)での当たりを判定する
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionLinePtr col1, ComponentCollisionModelPtr col2)
{
    ComponentCollision::HitInfo info{};

    // ライン情報
    auto line_owner = col1->GetOwner();
    auto line_comp  = line_owner->GetComponent<ComponentCollisionLine>();
    auto line       = line_comp->GetWorldLine();

    // モデル情報
    auto model_owner = col2->GetOwner();
    if(model_owner == nullptr)
        return info;

    auto model = model_owner->GetComponent<ComponentModel>();
    if(model == nullptr)
        return info;

    MV1_COLL_RESULT_POLY hit_poly{};

    // MV1とラインのチェック関数を呼び出す
    hit_poly = MV1CollCheck_Line(model->GetModel(), -1, cast(line[0]), cast(line[1]));

    // 当たったかどうかのフラグを設定し、当たった位置も入れておく
    info.hit_          = hit_poly.HitFlag;
    info.hit_position_ = float3(hit_poly.HitPosition.x, hit_poly.HitPosition.y, hit_poly.HitPosition.z);

    return info;
}

//! @brief Capsule VS Model
//! @param col1 Capsuleコリジョン
//! @param col2 Model コリジョン
//! @return 当たり情報
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionModelPtr col1, ComponentCollisionCapsulePtr col2)
{
    auto hit  = isHit(col2, col1);
    hit.push_ = -hit.push_;    // push方向を反対にする
    return hit;
}

//! @brief Box VS Sphere
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionBoxPtr col1, ComponentCollisionSpherePtr col2)
{
    ComponentCollision::HitInfo info{};

    // Boxのワールド行列 (スケールなし)
    matrix box_world    = mul(col1->GetMatrix(), col1->GetOwner()->GetWorldMatrix());
    box_world._11_12_13 = normalize(box_world._11_12_13);
    box_world._21_22_23 = normalize(box_world._21_22_23);
    box_world._31_32_33 = normalize(box_world._31_32_33);
    matrix box_inv      = inverse(box_world);

    float3 half_size = col1->GetSize() * col1->GetOwner()->GetScaleAxisXYZ();
    ;

    // 1フレーム前の状態を取得
    matrix old_box_world = mul(col1->GetMatrix(), col1->GetOwner()->GetOldWorldMatrix());
    matrix old_box_inv   = inverse(old_box_world);

    // 球の中心をBoxローカル空間で取得
    float3 sphere_pos_world = col2->GetWorldMatrix().translate();
    float3 sphere_pos_local = mul(float4(sphere_pos_world, 1), box_inv).xyz;

    // 半径を計算
    float3 mat_scale = col2->GetOwner()->GetScaleAxisXYZ();    // 簡易的にスケールを取得
    float  radius    = col2->GetRadius() * ((mat_scale.x + mat_scale.y + mat_scale.z) / 3.0f);

    // Boxの範囲内での最近接点を計算
    float3 closest = clamp(sphere_pos_local, -half_size, half_size);
    float3 diff    = sphere_pos_local - closest;
    float  dist_sq = dot(diff, diff);

    // 距離が半径以内なら衝突
    if(dist_sq < radius * radius) {
        info.hit_ = true;

        float  dist = sqrt(dist_sq);
        float3 normal;
        float  penetration;

        if(dist > FLT_EPSILON) {
            // 球がBox外部にある場合
            normal      = diff / dist;
            penetration = radius - dist;
        }
        else {
            // 球の中心がBox内部にある場合：最短の面法線を探す
            float3 abs_sphere   = abs(sphere_pos_local);
            float3 dist_to_face = half_size - abs_sphere;

            if(dist_to_face.x < dist_to_face.y && dist_to_face.x < dist_to_face.z) {
                normal      = float3(sphere_pos_local.x > 0 ? 1.0f : -1.0f, 0, 0);
                penetration = dist_to_face.x + radius;
            }
            else if(dist_to_face.y < dist_to_face.z) {
                normal      = float3(0, sphere_pos_local.y > 0 ? 1.0f : -1.0f, 0);
                penetration = dist_to_face.y + radius;
            }
            else {
                normal      = float3(0, 0, sphere_pos_local.z > 0 ? 1.0f : -1.0f);
                penetration = dist_to_face.z + radius;
            }
        }

        // 押し出しベクトルを適用
        float3 push_local  = normal * penetration;
        info.push_         = mul(float4(-push_local, 0), box_world).xyz;
        info.hit_position_ = col2->GetWorldMatrix().translate() - (normal * radius);
    }
    return info;
}

//! @brief Sphere VS Box
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionSpherePtr col1, ComponentCollisionBoxPtr col2)
{
    auto hit  = isHit(col2, col1);
    hit.push_ = -hit.push_;
    return hit;
}

//! @brief Box VS Capsule
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionBoxPtr col1, ComponentCollisionCapsulePtr col2)
{
    ComponentCollision::HitInfo info{};

    matrix box_world    = mul(col1->GetMatrix(), col1->GetOwner()->GetWorldMatrix());
    box_world._11_12_13 = normalize(box_world._11_12_13);
    box_world._21_22_23 = normalize(box_world._21_22_23);
    box_world._31_32_33 = normalize(box_world._31_32_33);
    matrix box_inv      = inverse(box_world);

    float3 half_size             = col1->GetSize() * col1->GetOwner()->GetScaleAxisXYZ();
    float  c2_difficult_to_climb = col2->GetClimbLimit();

    // カプセルの幾何学的定義の算出
    auto   trans2      = col2->GetWorldMatrix();
    float3 axis_y      = normalize((float3&)trans2.axisVectorY());
    float  half_height = (col2->GetHeight() * 0.5f);
    float  radius      = col2->GetRadius() * ((length(trans2.axisVectorX()) + length(trans2.axisVectorZ())) * 0.5f);

    // 底面位置(trans2.translate())から中心位置を算出
    float3 center_world = trans2.translate() + axis_y * half_height;

    // 中心を基準にBoxローカルでの軸始点・終点を算出
    float3 p1_local = mul(float4(center_world - (axis_y * half_height) + (axis_y * radius), 1), box_inv).xyz;
    float3 p2_local = mul(float4(center_world + (axis_y * half_height) - (axis_y * radius), 1), box_inv).xyz;

    // 1フレーム前の状態を現在のBoxのローカル座標系で取得
    auto   old_world        = col2->GetOwner()->GetOldWorldMatrix();
    float3 old_axis_y       = normalize((float3&)old_world.axisVectorY());
    float3 old_center_world = old_world.translate() + old_axis_y * ((col2->GetHeight() * 0.5f) + col2->GetRadius());
    float3 old_p1_local     = mul(float4(old_center_world, 1), box_inv).xyz;

    auto old_col2 = col2->GetOldWorldMatrix();

    // 相対移動ベクトル
    float3 movement = p1_local - old_p1_local;

    // サンプリング軸
    const int SAMPLES     = 64;
    float     min_dist_sq = FLT_MAX;
    float3    best_p, best_c;
    for(int i = 0; i <= SAMPLES; ++i) {
        float3 p    = p1_local + (p2_local - p1_local) * ((float)i / SAMPLES);
        float3 c    = clamp(p, -half_size, half_size);
        float  d_sq = dot(p - c, p - c);
        if(d_sq < min_dist_sq) {
            min_dist_sq = d_sq;
            best_p      = p;
            best_c      = c;
        }
    }

    // マージンを設定し、極端に微小な接触での押し戻しを防ぐ
    const float margin = 0.01f;
    if(min_dist_sq < (radius - margin) * (radius - margin)) {
        info.hit_         = true;
        float  d          = sqrt(min_dist_sq);
        float3 push_local = 0;

        if(d < FLT_EPSILON * 1000.0f) {
            // 内部: 接地優先
            if(col2->IsUseGravity()) {
                now_gravity_  = float3(0.0f, 0.0f, 0.0f);
                prev_gravity_ = float3(0.0f, 0.0f, 0.0f);
                GetOwner()->SetGravity(now_gravity_);
            }

            float3 diff = best_c - best_p;
            float  len  = dot(diff, diff);
            if(len < FLT_EPSILON * 1000.0f) {
                push_local = {0, -3.0f, 0};
            }
            else
                push_local = normalize(diff) * radius;
        }
        else {
            push_local = (best_c - best_p) * ((radius - d) / d);
            float len  = (float)push_local.x + (float)push_local.z;
            if(powf(len, 2.0f) <= 0.0f) {
                // 内部: 接地優先
                if(col2->IsUseGravity()) {
                    now_gravity_  = float3(0.0f, 0.0f, 0.0f);
                    prev_gravity_ = float3(0.0f, 0.0f, 0.0f);
                    GetOwner()->SetGravity(now_gravity_);
                }
            }
        }
        if(push_local.y > c2_difficult_to_climb && push_local.y > 0) {
            // 現状のMatrixをOldMatrixへ戻すpush_localを作成して、そこから現在のMatrixへの変換をかけることで、押し戻し方向を「上から押し戻す」方向に補正する
            float3 old_p = mul(float4(center_world, 1), old_col2).xyz;
            float3 old_c = clamp(old_p, -half_size, half_size);
            push_local   = old_p - old_c;
            push_local.y = 0;
        }

        info.push_         = mul(float4(push_local, 0), box_world).xyz;
        float3 bp_world    = mul(float4(best_p, 1), box_world).xyz;
        info.hit_position_ = bp_world - (normalize(info.push_) * radius);
    }
    return info;
}

//! @brief Capsule VS Box
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionCapsulePtr col1, ComponentCollisionBoxPtr col2)
{
    auto hit  = isHit(col2, col1);
    hit.push_ = -hit.push_;
    return hit;
}

//! @brief Box VS Model
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionBoxPtr col1, ComponentCollisionModelPtr col2)
{
    ComponentCollision::HitInfo info{};

    auto model = col2->GetOwner()->GetComponent<ComponentModel>();
    if(model == nullptr)
        return info;

    matrix box_world = mul(col1->GetMatrix(), col1->GetOwner()->GetWorldMatrix());
    float3 size      = col1->GetSize();

    // Boxをカプセルとして判定
    float3 pos    = {box_world._41, box_world._42, box_world._43};
    float3 p1     = pos + mul(float4(0, -size.y, 0, 0), box_world).xyz;
    float3 p2     = pos + mul(float4(0, size.y, 0, 0), box_world).xyz;
    float  radius = min(size.x, size.z);

    MV1_COLL_RESULT_POLY_DIM res = MV1CollCheck_Capsule(model->GetModel(), -1, cast(p1), cast(p2), radius);
    if(res.HitNum > 0) {
        info.hit_ = true;
        for(int i = 0; i < res.HitNum; i++) {
            float3 normal = cast(res.Dim[i].Normal);
            // 簡易的なめり込み深さの算出 (法線方向の投影)
            // 注意: res.Dim[i].Position は三角形の頂点(3点)
            float3 hitPos = cast(res.Dim[i].Position[0]);
            float  depth  = radius - dot(pos - hitPos, normal);

            if(depth > 0.0f) {
                float3 push = normal * depth;

                // 法線が上向き（床・斜面）であれば、真上のみに押し出して左右滑りを抑制
                if(normal.y >= 0.5f) {
                    push.x = 0.0f;
                    push.z = 0.0f;
                    // push.y はそのまま（depthを維持）
                }
                else {
                    // 壁の場合はY軸の押し出しを抑制
                    push.y = 0.0f;
                }

                if(abs(push.x) > abs(info.push_.x))
                    info.push_.x = push.x;
                if(abs(push.y) > abs(info.push_.y))
                    info.push_.y = push.y;
                if(abs(push.z) > abs(info.push_.z))
                    info.push_.z = push.z;
            }
            info.hit_position_ = hitPos;
        }
    }
    MV1CollResultPolyDimTerminate(res);
    return info;
}

//! @brief Model VS Box
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionModelPtr col1, ComponentCollisionBoxPtr col2)
{
    auto hit  = isHit(col2, col1);
    hit.push_ = -hit.push_;
    return hit;
}

//! @brief Box VS Line
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionBoxPtr col1, ComponentCollisionLinePtr col2)
{
    ComponentCollision::HitInfo info{};
    matrix                      box_world = mul(col1->GetMatrix(), col1->GetOwner()->GetWorldMatrix());
    box_world._11_12_13                   = normalize(box_world._11_12_13);
    box_world._21_22_23                   = normalize(box_world._21_22_23);
    box_world._31_32_33                   = normalize(box_world._31_32_33);
    matrix box_inv                        = inverse(box_world);
    float3 half_size                      = col1->GetSize();
    auto   line                           = col2->GetWorldLine();
    float3 p1_local                       = mul(float4(line[0], 1), box_inv).xyz;
    float3 p2_local                       = mul(float4(line[1], 1), box_inv).xyz;
    float3 dir_local                      = p2_local - p1_local;
    float  t_min = 0.0f, t_max = 1.0f;
    for(int i = 0; i < 3; ++i) {
        if(fabsf(dir_local[i]) < FLT_EPSILON) {
            if(p1_local[i] < -half_size[i] || p1_local[i] > half_size[i])
                return info;
        }
        else {
            float inv_dir = 1.0f / dir_local[i];
            float t0      = (-half_size[i] - p1_local[i]) * inv_dir;
            float t1      = (half_size[i] - p1_local[i]) * inv_dir;
            if(inv_dir < 0.0f) {
                float tmp = t0;
                t0        = t1;
                t1        = tmp;
            }
            if(t0 > t_min)
                t_min = t0;
            if(t1 < t_max)
                t_max = t1;
            if(t_min > t_max)
                return info;
        }
    }
    info.hit_            = true;
    float3 hit_pos_local = p1_local + dir_local * t_min;
    info.hit_position_   = mul(float4(hit_pos_local, 1), box_world).xyz;
    float3 normal_local  = 0;
    float  max_val       = -1.0f;
    for(int i = 0; i < 3; ++i) {
        float val = fabsf(hit_pos_local[i] / half_size[i]);
        if(val > max_val) {
            max_val         = val;
            normal_local    = 0;
            normal_local[i] = (hit_pos_local[i] > 0 ? 1.0f : -1.0f);
        }
    }
    // info.push_ は押し出しを行わないため 0 のままにします
    return info;
}

//! @brief Line VS Box
ComponentCollision::HitInfo ComponentCollision::isHit(ComponentCollisionLinePtr col1, ComponentCollisionBoxPtr col2)
{
    return isHit(col2, col1);
}
