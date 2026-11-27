//---------------------------------------------------------------------------
//! @file   ComponentStateMachine.cpp
//! @brief  キャラクターのステートマシン基底 (プレイヤー入力/CPU 共通)
//---------------------------------------------------------------------------
#include <Game/Component/ComponentStateMachine.h>
#include <Game/Component/ComponentGrabbable.h>
#include <Game/Component/ComponentStateGrab.h>
#include <Game/Component/ComponentStateThrow.h>
#include <Game/Component/ComponentStatePlayerJump.h>
#include <Game/Component/ComponentStateDown.h>
#include <Game/Component/ComponentCharacterPhysics.h>
#include <Game/Component/ComponentHitPoints.h>
#include <Game/Object/PoittersPoint_Player.h>

void ComponentStateMachine::Init()
{
    __super::Init();
}

void ComponentStateMachine::Update()
{
    __super::Update();

    // HPが0になったら脱落：持ち物を落として倒れ、起き上がらない
    // (掴まれている最中は、離されてから倒れる)
    if(IsEliminated()) {
        auto down = GetOwner()->GetComponent<ComponentStateDown>();
        if(!down) {
            DropHolding();
            grab_candidate_.reset();
            down = std::dynamic_pointer_cast<ComponentStateDown>(ChangeState<ComponentStateDown>()->shared_from_this());
        }
        down->SetEliminated();
        return;
    }

    // ダウンが終わったら派生クラスに通常ステートへ戻してもらう
    if(auto down = GetOwner()->GetComponent<ComponentStateDown>()) {
        if(down->GetIsFinished())
            OnRecover();
    }
}

void ComponentStateMachine::GUI()
{
    __super::GUI();

    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();
        if(ImGui::TreeNode("State Machine")) {
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable))
                SetStatus(StatusBit::Enable, enable);

            if(ImGui::Button(u8"削除"))
                GetOwner()->RemoveComponent(shared_from_this());

            ImGui::Text(u8"ステート: %s", GetStateName().c_str());
            auto candidate = grab_candidate_.lock();
            auto holding   = grabbing_object_ptr_.lock();
            ImGui::Text(u8"掴み候補: %s", candidate ? candidate->GetName().data() : "-");
            ImGui::Text(u8"持っている物: %s (投げられる: %s)", holding ? holding->GetName().data() : "-", CanThrow() ? u8"〇" : u8"×");

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

const std::string ComponentStateMachine::GetStateName() const
{
    if(auto state = GetOwner()->GetComponent<ComponentState>()) {
        return state->GetName().data();
    }
    return "";
}

//============================================================================
// 掴む・投げる
//============================================================================
bool ComponentStateMachine::GrabbableHit(ObjectPtr target)
{
    if(!target || IsBusy())
        return false;

    auto grabbable = target->GetComponent<ComponentGrabbable>();
    if(!grabbable || !grabbable->CanBeGrabbed())
        return false;

    grab_candidate_ = target;
    return true;
}

bool ComponentStateMachine::StartGrab()
{
    // モーション中・物を持っている時は持てない
    if(IsBusy())
        return false;

    auto target = grab_candidate_.lock();
    if(!target)
        return false;

    auto grabbable = target->GetComponent<ComponentGrabbable>();
    // 誰かが持ち上げ始めていたら Reserve が失敗する
    if(!grabbable || !grabbable->Reserve(GetOwner()->SharedThis()))
        return false;

    grabbing_object_ptr_ = target;
    grab_candidate_.reset();
    can_throw_ = false;

    ChangeState<ComponentStateGrab>()->SetLiftTime(grabbable->GetLiftTime() * GetLiftTimeRatio());
    return true;
}

bool ComponentStateMachine::FinishGrab()
{
    auto component_grab = GetOwner()->GetComponent<ComponentStateGrab>();
    if(!component_grab || !component_grab->GetIsFinished())
        return false;

    can_throw_ = false;
    if(auto object = grabbing_object_ptr_.lock()) {
        if(auto grabbable = object->GetComponent<ComponentGrabbable>()) {
            grabbable->Hold(GetOwner()->SharedThis());
            can_throw_ = grabbable->IsHeld();
        }
    }
    if(!can_throw_)
        grabbing_object_ptr_.reset();

    return true;
}

ComponentStateThrow* ComponentStateMachine::StartThrow(bool in_air)
{
    if(!CanThrow())
        return nullptr;

    can_throw_  = false;
    auto state  = ChangeState<ComponentStateThrow>();
    state->SetInAir(in_air);
    return state;
}

void ComponentStateMachine::ReleaseThrow(const float3& velocity)
{
    if(auto object = grabbing_object_ptr_.lock()) {
        if(auto grabbable = object->GetComponent<ComponentGrabbable>())
            grabbable->Throw(GetOwner()->SharedThis(), velocity);
    }
    grabbing_object_ptr_.reset();
    can_throw_ = false;
}

bool ComponentStateMachine::IsBusy() const
{
    if(IsHolding())
        return true;

    auto owner = GetOwner();
    return owner->GetComponent<ComponentStateGrab>() || owner->GetComponent<ComponentStateThrow>() ||
           owner->GetComponent<ComponentStatePlayerJump>() || owner->GetComponent<ComponentStateDown>();
}

void ComponentStateMachine::DropHolding()
{
    if(auto object = grabbing_object_ptr_.lock()) {
        if(auto grabbable = object->GetComponent<ComponentGrabbable>()) {
            // 持ち上げ中なら取り消し、持っていたら落とす
            grabbable->Drop();
        }
    }
    grabbing_object_ptr_.reset();
    can_throw_ = false;
}

float ComponentStateMachine::GetLiftTimeRatio() const
{
    if(auto character = dynamic_cast<const PoittersPoint::Player*>(GetOwner()))
        return character->GetLiftTimeRatio();
    return 1.0f;
}

//============================================================================
// 被弾・被掴み
//============================================================================
void ComponentStateMachine::KnockDown(const float3& velocity)
{
    DropHolding();
    grab_candidate_.reset();

    if(auto down = GetOwner()->GetComponent<ComponentStateDown>())
        down->Restart();
    else
        ChangeState<ComponentStateDown>();

    if(auto mover = GetOwner()->GetComponent<ComponentCharacterPhysics>())
        mover->Launch(velocity);
}

void ComponentStateMachine::OnGrabbed()
{
    DropHolding();
    grab_candidate_.reset();

    ComponentStateDown* down = GetOwner()->GetComponent<ComponentStateDown>().get();
    if(!down)
        down = ChangeState<ComponentStateDown>();
    down->Restart();
    down->SetHeld(true);

    if(auto mover = GetOwner()->GetComponent<ComponentCharacterPhysics>())
        mover->SetControlEnabled(false);
}

void ComponentStateMachine::OnReleased(bool thrown)
{
    if(auto down = GetOwner()->GetComponent<ComponentStateDown>()) {
        down->SetHeld(false);
        down->Restart();
        if(!thrown)
            down->RecoverSoon();
    }
}

bool ComponentStateMachine::IsEliminated() const
{
    auto hp = GetOwner()->GetComponent<ComponentHitPoints>();
    return hp && hp->IsDead();
}

bool ComponentStateMachine::IsDown() const
{
    return GetOwner()->GetComponent<ComponentStateDown>() != nullptr;
}

CEREAL_REGISTER_TYPE(ComponentStateMachine)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateMachine)
