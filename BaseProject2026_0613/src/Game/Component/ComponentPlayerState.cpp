//---------------------------------------------------------------------------
//! @file   ComponentPlayerState.cpp
//! @brief  プレイヤー状態制御コンポーネント
//---------------------------------------------------------------------------
#include <Game/Component/ComponentPlayerState.h>
#include <Game/Component/ComponentStateControllerWalk.h>
#include <Game/Component/ComponentStateGrab.h>
#include <Game/Component/ComponentStateThrow.h>
#include <Game/Component/ComponentStatePlayerJump.h>
#include <Game/Component/ComponentGrabbable.h>
#include <Game/Object/PoittersPoint_Player.h>

//===========================================================================
// 初期化処理
//===========================================================================
void ComponentPlayerState::Init()
{
    __super::Init();
    auto owner                  = GetOwner();
    auto character_casted_owner = dynamic_cast<PoittersPoint::Player*>(owner);

    // オブジェクトの制御を行うコンポーネントを追加
    if(character_casted_owner) {
        owner->AddComponent<ComponentStateControllerWalk>()->SetMoveSpeed(character_casted_owner->GetMoveSpeed());
    }
}

//===========================================================================
// 更新処理
//===========================================================================
void ComponentPlayerState::Update()
{
    __super::Update();

    // ダウン中(掴まれている間も含む)は操作できない
    if(IsDown())
        return;

    auto owner = GetOwner();
    auto walk  = owner->GetComponent<ComponentStateControllerWalk>();
    auto jump  = owner->GetComponent<ComponentStatePlayerJump>();

    // Eキー：持っていれば投げる / 持っていなければ視線内の物を持ち上げる
    if(Input::IsKeyDown(KEY_INPUT_E)) {
        if(CanThrow()) {
            // 地上(ControllerWalk) または 空中(PlayerJump) のどちらでも投げを許可
            if(walk || jump)
                StartThrow(jump != nullptr);
        }
        else if(walk) {
            // 持ち上げは地上の歩き/待機中のみ
            StartGrab();
        }
    }

    // 持ち上げモーションが終了したら歩きステートに戻る
    if(FinishGrab()) {
        ReturnToWalk();
        return;
    }

    // 投げモーションが終了したら歩きステートに戻る
    if(auto component_throw = owner->GetComponent<ComponentStateThrow>()) {
        if(component_throw->GetIsFinished())
            ReturnToWalk();
    }
}

//===========================================================================
// ダウンからの復帰
//===========================================================================
void ComponentPlayerState::OnRecover()
{
    ReturnToWalk();
}

//===========================================================================
// 歩きステートへ戻す
//===========================================================================
void ComponentPlayerState::ReturnToWalk()
{
    float speed = 0.3f;
    if(auto character = dynamic_cast<PoittersPoint::Player*>(GetOwner()))
        speed = character->GetMoveSpeed();

    ChangeState<ComponentStateControllerWalk>()->SetMoveSpeed(speed)->SetIsHolding(IsHolding());
}

//===========================================================================
// 更新後処理
//===========================================================================
void ComponentPlayerState::LateUpdate()
{
}

//===========================================================================
// GUI処理
//===========================================================================
void ComponentPlayerState::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("Player State")) {
            //-------------------------------------------------------
            // 共通部分

            // 有効/無効
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable))
                SetStatus(StatusBit::Enable, enable);

            // GUI上でオーナーから自分を削除します
            if(ImGui::Button(u8"削除"))
                GetOwner()->RemoveComponent(shared_from_this());
            //-------------------------------------------------------

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentPlayerState)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentPlayerState)
