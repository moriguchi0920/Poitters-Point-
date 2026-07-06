#pragma once
#include <Game/Component/ComponentStateThrow.h>

void ComponentStateThrow::Init()
{
	__super::Init();
}

void ComponentStateThrow::Update()
{
	__super::Update();

}

void ComponentStateThrow::GUI()
{
	__super::GUI();

	// GUI内に出現させる
	ImGui::Begin(GetOwner()->GetName().data());
	{
		ImGui::Separator();
		if(ImGui::TreeNode("State Throw"))
		{
			// 有効/無効
			bool enable = GetStatus(StatusBit::Enable);
			if(ImGui::Checkbox(u8"有効", &enable))
				SetStatus(StatusBit::Enable, enable);

			// GUI上でオーナーから自分を削除します
			if(ImGui::Button(u8"削除"))
				GetOwner()->RemoveComponent(shared_from_this());

			ImGui::TreePop();
		}
	}
	ImGui::End();
}

CEREAL_REGISTER_TYPE(ComponentStateThrow)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentStateThrow)
