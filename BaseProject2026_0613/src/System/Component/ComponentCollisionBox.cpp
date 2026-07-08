//---------------------------------------------------------------------------
//! @file   ComponentCollisionBox.cpp
//! @brief  Boxコリジョンコンポーネント
//---------------------------------------------------------------------------
#include <System/Component/ComponentCollisionBox.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCollisionLine.h>
#include <System/Component/ComponentTransform.h>
#include <System/Component/ComponentModel.h>
#include <System/Object.h>
#include <System/Scene.h>

int ComponentCollisionBox::shared_box_model_ = -1;

void ComponentCollisionBox::Init()
{
    __super::Init();
    // 共有モデルのロード (1x1x1の立方体)
    if(shared_box_model_ == -1) {
        shared_box_model_ = MV1LoadModel("data/System/Cube.x");
        // もしファイルがない場合は、DxLibの内部機能で作成するか、エラーとする
        // 現状はファイルがある前提

        if(shared_box_model_ != -1) {
            // 当たり判定用情報の構築
            MV1SetupCollInfo(shared_box_model_);
        }
    }
}

void ComponentCollisionBox::InitSerialize()
{
    __super::InitSerialize();    // 基底クラスの処理を呼び出し、Serializedフラグを立てる

    // 共有モデルのロード (1x1x1の立方体)
    if(shared_box_model_ == -1) {
        shared_box_model_ = MV1LoadModel("data/System/Cube.x");
        if(shared_box_model_ != -1) {
            // 当たり判定用情報の構築
            MV1SetupCollInfo(shared_box_model_);
        }
    }
}

void ComponentCollisionBox::Update()
{
    __super::Update();
}

void ComponentCollisionBox::PostUpdate()
{
    __super::PostUpdate();
    old_transform_ = GetWorldMatrix();
}

void ComponentCollisionBox::Draw()
{
    __super::Draw();

    if((!IsShowDebug() || !IsShowGrid()) && !collision_status_.is(CollisionBit::ShowInGame))
        return;

    if(shared_box_model_ == -1)
        return;

    auto trans = GetWorldMatrix();

    // デバッグ描画
    SetUseLighting(FALSE);
    SetLightEnable(FALSE);

    // 箱を描画 (ワイヤーフレーム)
    MV1SetMatrix(shared_box_model_, cast(trans));
    MV1SetWireFrameDrawFlag(shared_box_model_, TRUE);

    MV1SetDifColorScale(shared_box_model_, COLOR_F{0, 1.0f, 0, 1.0f});    // ここで色を緑に設定
    MV1DrawModel(shared_box_model_);

    MV1SetWireFrameDrawFlag(shared_box_model_, FALSE);

    SetLightEnable(TRUE);
    SetUseLighting(TRUE);
}

void ComponentCollisionBox::Exit()
{
    __super::Exit();
    // 共有モデルの解放は、アプリケーション終了時にDxLibが行うため、ここでは行わない
}

void ComponentCollisionBox::GUI()
{
    // オーナーの取得
    assert(GetOwner());
    auto obj_name = GetOwner()->GetName();

    ImGui::Begin(obj_name.data());
    {
        ImGui::Separator();
        auto ui_name = std::string("Collision Box:") + std::to_string(collision_id_);
        if(ImGui::TreeNode(ui_name.c_str())) {
            // 有効/無効
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable))
                SetStatus(StatusBit::Enable, enable);

            if(ImGui::Button(u8"削除")) {
                GetOwner()->RemoveComponent(shared_from_this());
            }

            // コリジョン情報を出す
            GUICollisionData();

            std::string colname = u8"COL:" + std::to_string(collision_id_) + "/ ";

            float* mat = GetColMatrixFloat();
            float  matrixTranslation[3], matrixRotation[3], matrixScale[3];
            DecomposeMatrixToComponents(mat, matrixTranslation, matrixRotation, matrixScale);
            ImGui::DragFloat3((colname + u8"座標(T)").c_str(), matrixTranslation, 0.01f);
            RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, mat);

            ImGui::DragFloat3((colname + u8"サイズ(S)").c_str(), (float*)&size_, 0.01f, 0.01f, 1000.0f, "%.2f");

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

ComponentCollisionBoxPtr ComponentCollisionBox::SetSize(float3 size)
{
    size_ = size;
    return std::dynamic_pointer_cast<ComponentCollisionBox>(shared_from_this());
}

float3 ComponentCollisionBox::GetSize() const
{
    return size_;
}

ComponentCollisionBox::HitInfo ComponentCollisionBox::IsHit(ComponentCollisionPtr col)
{
    HitInfo info;

    switch(col->GetCollisionType()) {
    case ComponentCollision::CollisionType::LINE:
        return isHit(std::dynamic_pointer_cast<ComponentCollisionBox>(shared_from_this()), std::dynamic_pointer_cast<ComponentCollisionLine>(col));
    case ComponentCollision::CollisionType::SPHERE:
        return isHit(std::dynamic_pointer_cast<ComponentCollisionBox>(shared_from_this()), std::dynamic_pointer_cast<ComponentCollisionSphere>(col));
    case ComponentCollision::CollisionType::CAPSULE:
        return isHit(std::dynamic_pointer_cast<ComponentCollisionBox>(shared_from_this()), std::dynamic_pointer_cast<ComponentCollisionCapsule>(col));
    case ComponentCollision::CollisionType::MODEL:
        return isHit(std::dynamic_pointer_cast<ComponentCollisionBox>(shared_from_this()), std::dynamic_pointer_cast<ComponentCollisionModel>(col));
    case ComponentCollision::CollisionType::BOX:
        // @todo Box VS Box
        break;
    }

    return info;
}

const matrix ComponentCollisionBox::GetWorldMatrix() const
{
    auto transform = collision_transform_;

    // Boxのサイズをスケールとして適用
    transform = mul(matrix::scale(size_), transform);

    if(IsCollisionStatus(ComponentCollision::CollisionBit::DisableTransform)) {
        return transform;
    }

    auto obj = GetOwner();
    if(attach_node_ >= 0) {
        auto mdl = obj->GetComponent<ComponentModel>();
        if(mdl) {
            transform = mul(transform, attach_node_matrix_);
        }
    }
    else {
        auto cmp = obj->GetComponent<ComponentTransform>();
        if(cmp) {
            transform = mul(transform, cmp->GetWorldMatrix());
        }
    }

    return transform;
}
