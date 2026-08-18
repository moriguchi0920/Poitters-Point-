#pragma once
#include <Game/Component/StateMachine/ComponentCPUState.h>
#include <Game/Component/State/ComponentStateTargetWalk.h>
#include <Game/Object/PoittersPoint_Rock.h>

void ComponentCPUState::Init()
{
    __super::Init();
    is_thinking_ = true;
}

void ComponentCPUState::Update()
{
    __super::Update();

    auto owner = GetOwner();

    // タイマーが未完成のため仮
    // 思考時間中の処理
    if(is_thinking_) {
        // 思考時間をカウントアップ
        tmp_count_++;
        // 仮で三秒たったら行動を変更させる
        if(second * 3 < tmp_count_) {
            while(true) {
                // 行動をenum内からランダムにとる
                int r = GetRand(CPU_ACTION::ACTION_NUM);
                // 二連続で掴みに入らないように弾く
                if(cur_action_ == CPU_ACTION::ACTION_GRAB && r == CPU_ACTION::ACTION_GRAB) {
                    continue;
                }
                // 同じく二連続で攻撃に入らないように弾く
                if(cur_action_ == CPU_ACTION::ACTION_ATTACK && r == CPU_ACTION::ACTION_ATTACK) {
                    continue;
                }
                // 現在の行動を更新
                cur_action_ = r;

                // 現在の行動指定をもとにステートを変更
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
                            ChangeState<ComponentStateTargetWalk>();
                        }
                        auto targetWalk = owner->GetComponent<ComponentStateTargetWalk>();
                        // ターゲットとなるオブジェクトを取得(ターゲットは仮で岩のみとする)
                        auto      targets   = Scene::Object::GetArray<PoittersPoint::Rock>();
                        ObjectPtr targetPtr = nullptr;
                        // 最短距離
                        float nearest_distance = 99999.99;
                        // 岩オブジェクトを走査
                        for(auto& target : targets) {
                            // 距離を算出し比較、一番近い岩を目的地へ
                            float3 vec = target->GetTranslate() - owner->GetTranslate();
                            float  dis = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
                            if(dis < nearest_distance) {
                                nearest_distance = dis;
                                targetPtr        = target;
                            }
                        }
                        // 目的地を登録
                        targetWalk->SetTargetPos(targetPtr->GetTranslate());
                        break;
                    }
                case CPU_ACTION::ACTION_AVOID_ATTACKER:
                    {
                        // すでにComponentStateTargetWalkがある場合(前回が回避だった場合想定)
                        if(owner->GetComponent<ComponentStateTargetWalk>()) {
                            // 何もしない
                        }
                        else {
                            // ステート変更
                            ChangeState<ComponentStateTargetWalk>();
                        }
                        auto targetWalk = owner->GetComponent<ComponentStateTargetWalk>();
                        break;
                    }
                case CPU_ACTION::ACTION_ATTACK:
                    {
                        break;
                    }
                }

                // 思考時間を終了
                is_thinking_ = false;
                break;
            }

            tmp_count_ = 0;
        }
    }
}

void ComponentCPUState::LateUpdate()
{
    can_grab_ = false;
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

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ComponentCPUState::GrabbableHit(bool is_hit_grabbable, ObjectPtr target)
{
    auto owner = GetOwner();
    if(owner->GetComponent<ComponentStateTargetWalk>()) {
        can_grab_            = is_hit_grabbable;
        grabbing_object_ptr_ = target;
    }
}

CEREAL_REGISTER_TYPE(ComponentCPUState)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentCPUState)
