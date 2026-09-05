#pragma once
#include <Game/Component/StateMachine/ComponentCPUState.h>
#include <Game/Component/State/ComponentStateTargetWalk.h>
#include <Game/Component/State/ComponentStateGrab.h>
#include <Game/Component/State/ComponentStateThrow.h>
#include <Game/Component/ComponentGrabbable.h>
#include <Game/Object/PoittersPoint_Rock.h>
#include <Game/Object/PoittersPoint_Character.h>
#include <Game/Component/State/ComponentStateSetRangeWalk.h>

void ComponentCPUState::Init()
{
    __super::Init();
    is_thinking_                = true;
    time_count_                 = 0.0f;
    auto owner                  = GetOwner();
    auto character_casted_owner = dynamic_cast<PoittersPoint::Character*>(owner);

    // オブジェクトの制御を行うコンポーネントを追加
    if(character_casted_owner) {
        auto component_range_walk = owner->AddComponent<ComponentStateSetRangeWalk>();
        component_range_walk->SetMoveSpeed(character_casted_owner->GetMoveSpeed());
        component_range_walk->SetWalkDirection({0.0f, 0.0f, 0.0f});
        component_range_walk->SetWalkDistance(0.0f);
    }

    can_grab_   = true;
    can_throw_  = false;
    cur_action_ = CPU_ACTION::ACTION_DEFAULT;
}

void ComponentCPUState::Update()
{
    __super::Update();

    auto owner                  = GetOwner();
    auto character_casted_owner = dynamic_cast<PoittersPoint::Character*>(owner);

    // 思考時間中の処理
    if(is_thinking_) {
        // 思考時間をカウントアップ
        time_count_ += GetDeltaTime();
        // 仮で三秒たったら行動を変更させる
        if(1.0f < time_count_) {
            int prev = cur_action_;
            while(true) {
                // 行動をenum内からランダムにとる
                int r = 1 + GetRand(CPU_ACTION::ACTION_NUM - 2);
                // 二連続で掴みに入らないように弾く
                if(cur_action_ == CPU_ACTION::ACTION_GRAB && r == CPU_ACTION::ACTION_GRAB) {
                    continue;
                }
                // 同じく二連続で攻撃に入らないように弾く
                if(cur_action_ == CPU_ACTION::ACTION_ATTACK && r == CPU_ACTION::ACTION_ATTACK) {
                    continue;
                }
                if(grabbing_object_ptr_.expired() && r == CPU_ACTION::ACTION_ATTACK) {
                    continue;
                }
                if(r == CPU_ACTION::ACTION_DEFAULT) {
                    continue;
                }
                // 現在の行動を更新
                cur_action_ = r;
                break;
            }
            // 現在の行動指定をもとにステートを変更(思考終了時の処理)
            switch(cur_action_) {
                // 地面にあるアイテムを持ち上げるアクション
            case CPU_ACTION::ACTION_GRAB:
                {
                    // すでにComponentStateTargetWalkがある場合(前回が回避だった場合想定)
                    if(owner->GetComponent<ComponentStateTargetWalk>()) {
                        // 何もしない
                    }
                    else {
                        // ステート変更
                        ChangeState<ComponentStateTargetWalk>()->SetMoveSpeed(character_casted_owner->GetMoveSpeed());
                    }
                    auto component_targetWalk = owner->GetComponent<ComponentStateTargetWalk>();
                    component_targetWalk->ResetTargetPtr();
                    // ターゲットとなるオブジェクトを取得(ターゲットは仮で岩のみとする)
                    auto      targets   = Scene::Object::GetArray<PoittersPoint::Rock>();
                    ObjectPtr targetPtr = nullptr;
                    // 最短距離
                    float nearest_distance = 99999.99;
                    // 岩オブジェクトを走査
                    for(auto& target : targets) {
                        // 距離を算出し比較、一番近い岩を目的地へ
                        if(auto grabbable = target->GetComponent<ComponentGrabbable>()) {
                            if(grabbable->GetIsGrabbed()) {
                                continue;
                            }
                        }
                        // 座標を減算しベクトルを取得
                        float3 vec = target->GetTranslate() - owner->GetTranslate();
                        // 距離取得
                        float  dis = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
                        // 比較して最短であれば
                        if(dis < nearest_distance) {
                            // 最短距離を更新
                            nearest_distance = dis;
                            // ターゲットとして登録
                            targetPtr        = target;
                        }
                    }
                    // 目的地を登録
                    if(targetPtr) {
                        float3 target = targetPtr->GetTranslate();
                        target.y      = 0.0f;
                        component_targetWalk->SetTargetPos(target);
                    }

                    break;
                }
                // 攻撃者や一番近いキャラクターから逃げるアクション
            case CPU_ACTION::ACTION_AVOID_ATTACKER:
                {
                    // ステートをRangeWalkに変更し逃げるように歩かせる
                    if(owner->GetComponent<ComponentStateSetRangeWalk>()) {
                    }
                    else {
                        if(prev == CPU_ACTION::ACTION_GRAB) {
                            // ステート変更
                            ChangeState<ComponentStateSetRangeWalk>()->SetMoveSpeed(character_casted_owner->GetMoveSpeed())->SetIsHolding(true);
                        }
                        else {
                            // ステート変更
                            ChangeState<ComponentStateSetRangeWalk>()->SetMoveSpeed(character_casted_owner->GetMoveSpeed());
                        }
                    }
                    // キャラクターを配列で取得
                    auto      characters       = Scene::Object::GetArray<PoittersPoint::Character>();
                    // 一番近いキャラクターのポインタ
                    ObjectPtr nearest_ptr      = nullptr;
                    // 一番近いものを持っているキャラクターのポインタ
                    ObjectPtr attacker_ptr     = nullptr;
                    // キャラクターへの距離を比べる用のfloat最大値(キャラクターの中から一番近いものを求めるため)
                    float     nearest_distance = FLT_MAX;
                    // キャラクター配列を走査
                    for(auto& character : characters) {
                        // 自分ははじく
                        if(std::static_pointer_cast<Object>(character) == owner->SharedThis())
                            continue;
                        // 対象のステートマシン取得
                        if(auto state_machine = character->GetComponent<ComponentStateMachine>()) {
                            // 座標を減算しベクトルを取得
                            float3 vec = character->GetTranslate() - owner->GetTranslate();
                            // ベクトルから距離を取得
                            float  dis = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
                            // 距離を比較して最短距離であれば
                            if(dis < nearest_distance) {
                                // 最短距離を更新
                                nearest_distance = dis;
                                // 一番近いキャラクターとして登録
                                nearest_ptr      = character;
                                // ものを持っていたら
                                if(state_machine->GetGrabbing()) {
                                    // 攻撃者として登録
                                    attacker_ptr = character;
                                }
                            }
                        }
                    }
                    // 攻撃者がいなければ
                    if(attacker_ptr == nullptr) {
                        // 一番近いキャラクターを攻撃者として扱う
                        attacker_ptr = nearest_ptr;
                    }
                    // 移動ベクトル
                    float3 move               = normalize(owner->GetTranslate() - attacker_ptr->GetTranslate());
                    // Y座標はモデルのがたつきが発生したためフリーズ
                    move.y                    = 0.0f;
                    // 歩きコンポーネントを取得
                    auto component_range_walk = owner->GetComponent<ComponentStateSetRangeWalk>();
                    // 移動方向を登録
                    component_range_walk->SetWalkDirection(move);
                    // 移動距離を設定
                    component_range_walk->SetWalkDistance(escape_offset_);

                    break;
                }
                // ものを持っているときにそれを投げて攻撃するアクション
            case CPU_ACTION::ACTION_ATTACK:
                {
                    // すでにComponentStateTargetWalkがある場合
                    if(owner->GetComponent<ComponentStateTargetWalk>()) {
                        // 何もしない
                    }
                    else {
                        // ステート変更
                        ChangeState<ComponentStateTargetWalk>()->SetMoveSpeed(character_casted_owner->GetMoveSpeed())->SetIsHolding(true);
                    }
                    // 歩きコンポーネントを取得
                    auto      component_target_walk = owner->GetComponent<ComponentStateTargetWalk>();
                    // キャラクターを配列で取得
                    auto      characters            = Scene::Object::GetArray<PoittersPoint::Character>();
                    // 一番近いオブジェクトのポインタ
                    ObjectPtr nearest_ptr           = nullptr;
                    // キャラクターへの距離を比べる用のfloat最大値(キャラクターの中から一番近いものを求めるため)
                    float     nearest_distance      = FLT_MAX;
                    for(auto& character : characters) {
                        if(character == owner->SharedThis())
                            continue;
                        if(auto state_machine = character->GetComponent<ComponentStateMachine>()) {
                            float3 vec = character->GetTranslate() - owner->GetTranslate();
                            float  dis = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
                            if(dis < nearest_distance) {
                                nearest_distance = dis;
                                nearest_ptr      = character;
                            }
                        }
                    }
                    component_target_walk->SetTargetPtr(nearest_ptr);

                    break;
                }
            }

            // 思考時間を終了
            is_thinking_ = false;

            time_count_ = 0.0f;
        }
    }

    // 行動ごとの更新
    switch(cur_action_) {
    case CPU_ACTION::ACTION_GRAB:
        {
            if(auto component_walk = owner->GetComponent<ComponentStateWalkBase>()) {
                if(!grabbing_object_ptr_.expired()) {
                    // 持ち上げるオブジェクトのGrabbableコンポーネントを取得
                    auto grabbable = grabbing_object_ptr_.lock()->GetComponent<ComponentGrabbable>();
                    // コンポーネントがあったら
                    if(grabbable) {
                        // 持ち上げ相手が持てる状態なら
                        if(grabbable->GetCanGrab()) {
                            // ステートをGrabステートに
                            ChangeState<ComponentStateGrab>()->SetLiftTime(grabbable->GetLiftTime());
                            can_grab_ = false;
                            grabbable->SetCanGrab(false);
                        }
                        else {
                            is_thinking_ = true;
                        }
                    }
                }
            }
            if(auto component_grab = owner->GetComponent<ComponentStateGrab>()) {
                if(component_grab->GetIsFinished() && can_throw_ == false) {
                    // 掴みオブジェクトがある時
                    if(!grabbing_object_ptr_.expired()) {
                        auto object = grabbing_object_ptr_.lock();

                        if(auto collider = object->GetComponent<ComponentCollision>()) {
                            collider->SetCollisionStatus(ComponentCollision::CollisionBit::DisableHit, true);
                        }

                        auto grabbable = object->GetComponent<ComponentGrabbable>();
                        grabbable->SetIsGrabbed(true);

                        grabbing_object_ptr_.lock()->AddComponent<ComponentAttachModel>()->SetAttachObject(owner->GetName(), "mixamorig:RightHand");
                    }
                    can_throw_ = true;

                    is_thinking_ = true;
                }
            }
            break;
        }
    case CPU_ACTION::ACTION_AVOID_ATTACKER:
        {
            if(auto component_set_range_walk = owner->GetComponent<ComponentStateSetRangeWalk>()) {
                if(component_set_range_walk->GetArrival() || component_set_range_walk->GetStopped()) {
                    is_thinking_ = true;
                }
            }
            break;
        }
    case CPU_ACTION::ACTION_ATTACK:
        {
            if(auto component_target_walk = owner->GetComponent<ComponentStateTargetWalk>()) {
                if((component_target_walk->GetArrival() || component_target_walk->GetStopped()) && can_throw_) {
                    ChangeState<ComponentStateThrow>()->SetThrowObject(grabbing_object_ptr_);
                    can_throw_ = false;
                }
            }
            if(auto component_throw = owner->GetComponent<ComponentStateThrow>()) {
                if(component_throw->GetIsFinished()) {
                    can_grab_    = true;
                    is_thinking_ = true;
                }
            }
        }
    }
}

void ComponentCPUState::GUI()
{
    __super::GUI();

    // GUI内に出現させる
    ImGui::Begin(GetOwner()->GetName().data());
    {
        ImGui::Separator();    // 線が出てくる
        if(ImGui::TreeNode("CPU State")) {
            //-------------------------------------------------------
            // 共通部分(共通化したい)

            // 有効/無効
            bool enable = GetStatus(StatusBit::Enable);
            if(ImGui::Checkbox(u8"有効", &enable))
                SetStatus(StatusBit::Enable, enable);

            // GUI上でオーナーから自分(SampleObjectController)を削除します
            if(ImGui::Button(u8"削除"))
                GetOwner()->RemoveComponent(shared_from_this());
            //-------------------------------------------------------

            ImGui::DragInt(u8"状態", &cur_action_, 1.0f);

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentCPUState)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentCPUState)
