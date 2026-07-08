#include <System/Component/ComponentTargetTracking.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentTransform.h>
#include <System/Object.h>
#include <System/Scene.h>

#include <System/ImGui.h>
#include <System/Utils/HelperLib.h>

namespace {
std::string null_name = "  ";

// string用のCombo
static int Combo(const std::string& caption, std::string& current_item, const std::vector<std::string_view>& items)
{
    int select_index = -1;

    if(ImGui::BeginCombo(caption.c_str(), current_item.c_str())) {
        for(int i = 0; i < items.size(); i++) {
            auto& item = items[i];

            bool is_selected = (current_item == item);
            if(ImGui::Selectable(item.data(), is_selected)) {
                current_item = item;
                select_index = i;
            }
            if(is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    return select_index;
}

}    // namespace

//---------------------------------------------------------
//! カメラ初期化
//---------------------------------------------------------
void ComponentTargetTracking::Init()
{
    __super::Init();

    tracking_status_.on(TrackingBit::Initialized);
}

//---------------------------------------------------------
//! 更新
//---------------------------------------------------------
void ComponentTargetTracking::PreUpdate()
{
    // 一旦リセットする
    if(auto model = owner_model_.lock())
        MV1ResetFrameUserLocalMatrix(model->GetModel(), tracked_node_index_);
}

void ComponentTargetTracking::PostUpdate()
{
    // アニメーション適応後、
    // tracked_node_index_からノードマトリクスを取得してそれに対して
    // さらなる向きの方向(now_xy_)を適応してTrackingを行うように変更する

    if(tracked_node_index_ < 0)
        return;

    float3 target_pos = look_at_;

    if(auto target = tracking_object_.lock())
        target_pos = target->GetTranslate();

    // Objectのモデルを先にWeakPtrで取得して確保しておく
    if(owner_model_.lock() == nullptr)
        owner_model_ = GetOwner()->GetComponent<ComponentModel>();

    if(auto model = owner_model_.lock()) {
        // trackingするマトリクス
        tracking_matrix_ = cast(MV1GetFrameLocalMatrix(model->GetModel(), tracked_node_index_));
        // 正しい向きのノードMatrixを取得しておく
        float3 node_rot;
        float3 node_trans;
        float3 node_scale;
        DecomposeMatrixToComponents((float*)tracking_matrix_.f32_128_0, (float*)&node_trans, (float*)&node_rot, (float*)&node_scale);
#if 1
        // モデルの方向を取得する
        float3 model_vec = normalize(mul(float4(front_vector_, 0), model->GetWorldMatrix()).xyz);

        // モデルを通常に戻すインバーズマトリクスを取得しておく
        auto fmat = inverse(HelperLib::Math::CreateMatrixByFrontVector(model_vec));

        // ターゲットへの向きを取得して( MV1GetFrameLocalWorldMatrixでとると100%位置はあっているがどうやら…1Frame遅れる)
        float3 node_pos  = tracking_matrix_._41_42_43;
        float3 model_pos = mul(float4(node_pos, 1), GetOwner()->GetMatrix()).xyz;

        float3 vec = target_pos - model_pos;
        auto   mat = HelperLib::Math::CreateMatrixByFrontVector(vec, {0, 1, 0}, true);

        // それにインバースを掛け合わせて0基準のベクトルに変換する
        mat           = mul(mat, fmat);
        mat._41_42_43 = node_pos;

        // 現状の回転と照らし合わせて回転を計算する
        float3 new_rot;
        float3 new_trans;
        float3 new_scale;
        // その回転をノードMatrixにさらに加えて向きの回転を実現する
        DecomposeMatrixToComponents((float*)mat.f32_128_0, (float*)&new_trans, (float*)&new_rot, (float*)&new_scale);

        // ZベクトルをY軸固定で角度に変化させる
        float3 zvec  = mat.axisZ();
        float  lenzx = sqrt((zvec.z * zvec.z) + (zvec.x * zvec.x));
        float  lenzy = sqrt((zvec.z * zvec.z) + (zvec.y * zvec.y));

        new_rot.x = atan2((float)zvec.y, (float)lenzx) * RadToDeg;
        new_rot.y = atan2((float)zvec.x, (float)lenzy) * RadToDeg;
        new_rot.z = 0;

        if(new_rot.y > 180)
            new_rot.y = new_rot.y - 360;
        if(new_rot.x < -180)
            new_rot.x += 360;

        // 右リミット
        if((float)new_rot.y > (float)limit_lr_.y) {
            new_rot.y = limit_lr_.y;
        }
        // 左リミット
        if((float)new_rot.y < -(float)limit_lr_.x) {
            new_rot.y = -limit_lr_.x;
        }

        // 上リミット
        if((float)new_rot.x > (float)limit_ud_.x) {
            new_rot.x = limit_ud_.x;
        }
        // 下リミット
        if((float)new_rot.x < -(float)limit_ud_.y) {
            new_rot.x = -limit_ud_.y;
        }

        float2 diff = {new_rot.x - now_rot_.x, new_rot.y - now_rot_.y};
        float  len  = length(diff);
        if(len > limit_frame_) {
            diff = normalize(diff) * limit_frame_;
        }

        now_rot_.x += diff.x;
        now_rot_.y += diff.y;

        node_rot.x += now_rot_.x;
        node_rot.y += now_rot_.y;

#endif
        RecomposeMatrixFromComponents((float*)&node_trans, (float*)&node_rot, (float*)&node_scale, (float*)mat.f32_128_0);

        //auto roty = matrix::rotateY(radians((float1)theta_xz));
        //auto rotx = matrix::rotateX(radians((float1)theta_yz));
        //mat = mul(mul(tracking_matrix_, rotx), roty);

        MV1SetFrameUserLocalMatrix(model->GetModel(), tracked_node_index_, cast(mat));

        // これをONにすると他のGizmoのIsOverなどがチェックが取れずPICKが常に有効になる(地面が選ばれてしまう)
        //ShowGizmo( (float*)mat.f32_128_0, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL, 0xab1245 );
    }
}

//---------------------------------------------------------
//! 向きのマトリクス表示(デバッグ用)
//---------------------------------------------------------
void ComponentTargetTracking::Draw()
{
}

//---------------------------------------------------------
//! カメラ終了処理
//---------------------------------------------------------
void ComponentTargetTracking::Exit()
{
    __super::Exit();
}

//---------------------------------------------------------
//! カメラGUI処理
//---------------------------------------------------------
void ComponentTargetTracking::GUI()
{
    assert(GetOwner());
    auto obj_name = GetOwner()->GetName();

    ImGui::Begin(obj_name.data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode(u8"TargetTracking")) {
            if(ImGui::Button(u8"削除")) {
                GetOwner()->RemoveComponent(shared_from_this());
            }

            int select_node_index = -1;
            if(auto model = GetOwner()->GetComponent<ComponentModel>()) {
                auto list         = model->GetNodesName();
                select_node_index = Combo(u8"トラッキングノード", tracked_node_, list);
                if(select_node_index < list.size())
                    SetTrackingNode(select_node_index);
            }

            if(auto scene = Scene::GetCurrentScene()) {
                std::string name = "";
                if(auto obj = tracking_object_.lock()) {
                    name = obj->GetName();
                }

                auto                          objs = scene->GetObjectPtrVec();
                std::vector<std::string_view> names;
                names.push_back(null_name);
                for(auto obj : objs) {
                    names.push_back(obj->GetName());
                }
                int id = Combo(u8"ターゲットオブジェクト", name, names) - 1;

                if(id >= 0) {
                    tracking_object_ = objs[id];
                }
                else if(id == -1) {
                    tracking_object_.reset();
                }
            }

            tracking_status_.off(TrackingBit::ObjectTracking);
            if(auto obj = tracking_object_.lock()) {
                tracking_status_.on(TrackingBit::ObjectTracking);
            }

            if(!tracking_status_.is(TrackingBit::ObjectTracking)) {
                ImGui::DragFloat3(u8"ターゲットポイント", (float*)&look_at_, 0.01f);
            }

            ImGui::Separator();
            ImGui::DragFloat2(u8"左右角度制限", (float*)&limit_lr_, 0.1f, 0, 180, "%.1f");
            ImGui::DragFloat2(u8"上下角度制限", (float*)&limit_ud_, 0.1f, 0, 89, "%.1f");
            ImGui::Separator();
            ImGui::DragFloat3(u8"前ベクトル", (float*)&front_vector_, 0.1f, 0, 180, "%.1f");

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ComponentTargetTracking::SetTargetObjectPtr(ObjectWeakPtr obj)
{
    tracking_object_ = obj;
    tracking_status_.on(TrackingBit::ObjectTracking);
    if(tracking_object_.expired()) {
        tracking_status_.off(TrackingBit::ObjectTracking);
    }
}

void ComponentTargetTracking::SetTargetObjectPtr(const std::string_view obj_name)
{
    auto obj = Scene::GetObjectPtr<Object>(obj_name);
    SetTargetObjectPtr(obj);
}

ObjectPtr ComponentTargetTracking::GetTargetObjectPtr()
{
    if(auto obj = tracking_object_.lock()) {
        return obj;
    }

    return nullptr;
}

void ComponentTargetTracking::SetTargetPoint(float3 target)
{
    tracking_object_.reset();
    look_at_ = target;
    tracking_status_.off(TrackingBit::ObjectTracking);
}

void ComponentTargetTracking::SetTrackingNode(const std::string& name)
{
    if(owner_model_.lock() == nullptr)
        owner_model_ = GetOwner()->GetComponent<ComponentModel>();

    if(auto model = owner_model_.lock()) {
        tracked_node_ = name;

        tracked_node_index_ = MV1SearchFrame(model->GetModel(), name.c_str());

        auto mat         = cast(MV1GetFrameLocalMatrix(model->GetModel(), tracked_node_index_));
        tracking_matrix_ = mat;
    }
}

void ComponentTargetTracking::SetTrackingNode(int tracking_node_index)
{
    if(owner_model_.lock() == nullptr)
        owner_model_ = GetOwner()->GetComponent<ComponentModel>();

    if(auto model = owner_model_.lock()) {
        tracked_node_index_ = tracking_node_index;

        auto mat         = cast(MV1GetFrameLocalMatrix(model->GetModel(), tracked_node_index_));
        tracking_matrix_ = mat;
    }
}

void ComponentTargetTracking::SetTrackingLimitLeftRight(float2 left_right)
{
    limit_lr_ = left_right;
}

void ComponentTargetTracking::SetTrackingLimitUpDown(float2 up_down)
{
    if(up_down.x > 89.0f)
        up_down.x = 89.0f;
    if(up_down.x < -89.0f)
        up_down.x = -89.0f;

    if(up_down.y > 89.0f)
        up_down.y = 89.0f;
    if(up_down.y < -89.0f)
        up_down.y = -89.0f;
    limit_ud_ = up_down;
}

void ComponentTargetTracking::SetTrackingFrameLimit(float limit)
{
    limit_frame_ = limit;
}

void ComponentTargetTracking::SetFrontVector(float3 vec)
{
    front_vector_ = normalize(vec);
}
