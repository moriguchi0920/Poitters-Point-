//---------------------------------------------------------------------------
//! @file   ComponentTransform.cpp
//! @brief  トランスフォームコンポーネント(座標と姿勢と大きさ)
//---------------------------------------------------------------------------
#include <System/Component/ComponentTransform.h>
#include <System/Object.h>
#include <System/Scene.h>
#include <System/Component/ComponentPhysics.h>

#include <ImGuizmo/ImGuizmo.h>

Component* ComponentTransform::select_component_ = nullptr;

//! @brief 三軸ギズモの表示
//! @param matrix マトリクス(位置・回転・スケール)
//! @param ope オペレーション
//! @param mode モード
//! @param id 個別ID
//! @return 表示できたか
bool ShowGizmo(float* matrix, ImGuizmo::OPERATION ope, ImGuizmo::MODE mode, uint64_t id)
{
    // Gizmoを表示するためのMatrixをDxLibから取得
    auto camera_view = GetCameraViewMatrix();
    auto camera_proj = GetCameraProjectionMatrix();

    // Gizmoの表示を設定する
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());

    // 画面サイズを取得する
    RECT rect;
    RECT crect;
    GetWindowRect(GetMainWindowHandle(), &rect);
    GetClientRect(GetMainWindowHandle(), &crect);

    // Gizmoを画面に合わせて処理する
    float windowWidth  = (float)rect.right - rect.left;
    float windowHeight = (float)rect.bottom - rect.top;
    float windowBar    = windowHeight - crect.bottom;
    ImGuizmo::SetRect((float)rect.left, (float)rect.top + windowBar / 2, windowWidth, windowHeight - windowBar / 2);

    // Manipulateを表示する
    static bool  useSnap         = false;
    static float snap[3]         = {1.0f, 1.0f, 1.0f};
    static float bounds[]        = {-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f};
    static float boundsSnap[]    = {0.1f, 0.1f, 0.1f};
    static bool  boundSizing     = false;
    static bool  boundSizingSnap = false;
    ImGuizmo::SetID((int)id);
    ImGuizmo::AllowAxisFlip(false);    //< これがないとGizmoが反転してしまう

    return ImGuizmo::Manipulate((const float*)&camera_view,
                                (const float*)&camera_proj,
                                ope,
                                mode,
                                (float*)matrix,
                                NULL,
                                false ? &snap[0] : NULL,
                                boundSizing ? bounds : NULL,
                                boundSizingSnap ? boundsSnap : NULL);
}

//! @brief Matrixから位置・回転・スケールに変換(ZXYで変換)
//! @param matx 入力Matrix
//! @param translation 位置
//! @param rotation 回転
//! @param scale スケール
void DecomposeMatrixToComponents(const float* matx, float* translation, float* rotation, float* scale)
{
    matrix mat = *(matrix*)matx;

    scale[0] = length(mat.axisX());
    scale[1] = length(mat.axisY());
    scale[2] = length(mat.axisZ());

    MATRIX dxmat = cast(mat);

    // ZXYで回転を作成 (DxLib命令を利用)
    GetMatrixZXYRotation(&dxmat, &rotation[0], &rotation[1], &rotation[2]);

    constexpr float RAD2DEG = (180.f / DX_PI_F);

    rotation[0] *= RAD2DEG;
    rotation[1] *= RAD2DEG;
    rotation[2] *= RAD2DEG;

    translation[0] = mat.translate().x;
    translation[1] = mat.translate().y;
    translation[2] = mat.translate().z;
}

//! @brief 位置・回転・スケールからMatrixに変換(ZXYで変換)
//! @param translation 位置
//! @param rotation 回転
//! @param scale スケール
//! @param matx マトリクス
void RecomposeMatrixFromComponents(const float* translation, const float* rotation, const float* scale, float* matx)
{
    constexpr float DEG2RAD = (DX_PI_F / 180.0f);
    MATRIX          dxmat;

    // ZXYでマトリクスを作成 (DxLib命令を利用)
    CreateRotationZXYMatrix(&dxmat, rotation[0] * DEG2RAD, rotation[1] * DEG2RAD, rotation[2] * DEG2RAD);

    matrix mat = cast(dxmat);

    mat._11_12_13    = mat._11_12_13 * scale[0];
    mat._21_22_23    = mat._21_22_23 * scale[1];
    mat._31_32_33    = mat._31_32_33 * scale[2];
    mat._41_42_43_44 = {translation[0], translation[1], translation[2], 1};

    memcpy(matx, &mat, sizeof(mat));
}

ComponentTransform::ComponentTransform()
{
    Matrix() = matrix::identity();
}

void ComponentTransform::Init()
{
    __super::Init();

    // 初期段階で前回設定のワールドマトリクス使用しないようにする
    Scene::GetCurrentScene()->SetPriority(shared_from_this(), ProcTiming::PreUpdate, ProcPriority::HIGHEST);
}

void ComponentTransform::PrePhysics()
{
    // 移動可能な物体
    if(auto cmp_physics = GetOwner()->GetComponent<ComponentPhysics>()) {
        if(!cmp_physics->GetPhysicsStatus(ComponentPhysics::PhysicsBit::Static)) {
            auto       mx = cmp_physics->GetWorldMatrix();
            quaternion q((float3x3)mx);
            auto       trans = mx.translate();
            auto       t     = GetDeltaTime();
            auto       body  = cmp_physics->GetRigidBody();
            if(std::isnan(trans.x) || std::isnan(trans.y) || std::isnan(trans.z))
                return;
#if 0
			auto mz = body->worldMatrix();
			float len = length( mz.translate() - trans );
			if( len <= 0.01f )
				return;
#endif
            if(body) {
                float power = cmp_physics->GetPhysicsReal();
                if(power >= 1.0f)
                    return;

                float3 v = {body->linearVelocity().x * power, body->linearVelocity().y, body->linearVelocity().z * power};
                if(v.y > 0)
                    v.y = body->linearVelocity().y * power;

                body->moveKinematic(trans, q, t);
                body->addLinearVelocity(v);
            }
        }
    }
}

//! @brief 更新後の処理
void ComponentTransform::PostUpdate()
{
    __super::PostUpdate();

    old_transform_ = GetWorldMatrix();
}

//! @brief GUI処理
void ComponentTransform::GUI()
{
    // オーナーの取得
    assert(GetOwner());
    auto obj_name = GetOwner()->GetName();

    // GUIによる場所の移動をおこなったか?
    bool update = false;

    if(is_guizmo_) {
        // Gizmo表示
        float* ptr = GetMatrixFloat();
        if(world_transform_enable_)
            ptr = world_transform_.f32_128_0;

        if(ShowGizmo(ptr, gizmo_operation_, gizmo_mode_, reinterpret_cast<uint64_t>(this)))
            update = true;

        // キーにより、Manipulateの処理を変更する
        // TODO : 一旦UE4に合わせておくが、のちにEditor.iniで設定できるようにする
        // W = Translate / E = Rotate / R = Scale (Same UE5)
        if(!ImGui::IsAnyItemActive()) {
            if(ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_W))
                gizmo_operation_ = ImGuizmo::TRANSLATE;
            if(ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_E))
                gizmo_operation_ = ImGuizmo::ROTATE;
            if(ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_R))
                gizmo_operation_ = ImGuizmo::SCALE;
        }
    }

    // GUI描画
    ImGui::Begin(obj_name.data());
    {
        ImGui::Separator();
        is_guizmo_ = false;
        if(ImGui::TreeNode("Transform")) {
            ImGui::DragFloat4(u8"Ｘ軸", VectorAxisXFloat(), 0.01f, -10000.0f, 10000.0f, "%.2f");
            ImGui::DragFloat4(u8"Ｙ軸", VectorAxisYFloat(), 0.01f, -10000.0f, 10000.0f, "%.2f");
            ImGui::DragFloat4(u8"Ｚ軸", VectorAxisZFloat(), 0.01f, -10000.0f, 10000.0f, "%.2f");
            ImGui::DragFloat4(u8"座標", TranslateFloat(), 0.01f, -10000.0f, 10000.0f, "%.2f");

            ImGui::Separator();
            ImGui::TreePop();
        }

        if(ImGui::IsWindowFocused()) {
            select_component_ = this;
        }
        if(select_component_ == this)
            is_guizmo_ = true;

        // ギズモモードの選択
        // ギズモの座標
        if(ImGui::RadioButton(u8"座標", gizmo_operation_ == ImGuizmo::TRANSLATE))
            gizmo_operation_ = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        // ギズモの回転
        if(ImGui::RadioButton(u8"回転", gizmo_operation_ == ImGuizmo::ROTATE))
            gizmo_operation_ = ImGuizmo::ROTATE;
        ImGui::SameLine();
        // ギズモのスケール
        if(ImGui::RadioButton(u8"スケール", gizmo_operation_ == ImGuizmo::SCALE))
            gizmo_operation_ = ImGuizmo::SCALE;
        ImGui::SameLine();
        // ギズモの全部
        if(ImGui::RadioButton(u8"全部", gizmo_operation_ == ImGuizmo::UNIVERSAL))
            gizmo_operation_ = ImGuizmo::UNIVERSAL;

        // ギズモスケール以外はLocal/Worldの選択ができるようにしておく
        if(gizmo_operation_ != ImGuizmo::SCALE) {
            if(ImGui::RadioButton("Local", gizmo_mode_ == ImGuizmo::LOCAL))
                gizmo_mode_ = ImGuizmo::LOCAL;
            ImGui::SameLine();
            if(ImGui::RadioButton("World", gizmo_mode_ == ImGuizmo::WORLD))
                gizmo_mode_ = ImGuizmo::WORLD;
        }

        // TRSにてマトリクスを再度作成する
        float* mat = GetMatrixFloat();
        float  matrixTranslation[3], matrixRotation[3], matrixScale[3];
        DecomposeMatrixToComponents(mat, matrixTranslation, matrixRotation, matrixScale);

        if(ImGui::DragFloat3(u8"座標(T)", matrixTranslation, 0.1f))
            update = true;

        ImGui::DragFloat3(u8"回転(R)", matrixRotation, 0.1f);
        ImGui::DragFloat3(u8"スケール(S)", matrixScale, 0.01f);
        RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, mat);

        if(update) {
            // 親が ComponentPhysics を持っている場合はそちらを変更する必要がある
            if(auto cmp_physics = GetOwner()->GetComponent<ComponentPhysics>())
                cmp_physics->SetPhysicsMatrix(cmp_physics->GetWorldMatrix());

            PostUpdate();
        }
    }
    ImGui::End();
}
