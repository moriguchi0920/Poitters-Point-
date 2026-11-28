#include <Game/Component/ComponentStateSetRangeWalk.h>

void ComponentStateSetRangeWalk::Init()
{
    __super::Init();
    SetName<Component>("State SetRangeWalk");
}

void ComponentStateSetRangeWalk::Update()
{
    auto owner = GetOwner();
    __super::Update();

    // 実際に進んだ距離だけ残り距離を減らす(壁に当たって進めない場合は止まっている扱い)
    const float3 pos = owner->GetTranslate();
    if(has_prev_pos_) {
        float3 moved    = pos - prev_pos_;
        moved.y         = 0.0f;
        float  distance = (float)length(moved);
        walk_distance_ -= distance;

        const float expected = move_speed_ * GetDeltaTime() * 60.0f;
        if(distance < expected * 0.5f)
            stopping_time_ += GetDeltaTime();
        else
            stopping_time_ = 0.0f;
    }
    prev_pos_     = pos;
    has_prev_pos_ = true;

    float3 dir = {0.0f, 0.0f, 0.0f};
    if(walk_distance_ > 0.1f && (float)length(walk_direction_) > 0.0f) {
        dir = normalize(walk_direction_);
        MoveOwner(dir * move_speed_);
    }

    UpdateModel(dir);
}

void ComponentStateSetRangeWalk::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("State SetRangeWalk")) {
            //-------------------------------------------------------
            // 共通部分

            // 有効/無効
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable))
                SetStatus(StatusBit::Enable, enable);

            // GUI上でオーナーから自分(SampleObjectController)を削除します
            if(ImGui::Button(u8"削除"))
                GetOwner()->RemoveComponent(shared_from_this());
            //-------------------------------------------------------

            // 移動の基本情報
            ImGui::DragFloat(u8"移動速度", &move_speed_, 0.1f);
            ImGui::DragFloat(u8"移動回転角度", &rot_speed_, 1.0f);

            // 移動の基本情報
            ImGui::DragFloat(u8"オブジェクト オフセット回転", &front_rot_, 1.0f);

            //if(ImGui::TreeNode("State IdleWalk")) とセット
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ComponentStateSetRangeWalk::SetWalkDirection(const float3& direction)
{
    walk_direction_ = direction;
}

void ComponentStateSetRangeWalk::SetWalkDistance(const float& distance)
{
    walk_distance_ = distance;
}

bool ComponentStateSetRangeWalk::GetArrival()
{
    return walk_distance_ < 0.1f;
}

bool ComponentStateSetRangeWalk::GetStopped()
{
    return stop_judge_time_ < stopping_time_;
}

bool ComponentStateSetRangeWalk::GetNearby()
{
    return walk_distance_ < 1.0f;
}

CEREAL_REGISTER_TYPE(ComponentStateSetRangeWalk)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateSetRangeWalk)
