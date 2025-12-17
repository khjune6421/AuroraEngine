#include "stdafx.h"
#include "ComponentBase.h"

void ComponentBase::RenderImGui()
{
	if (ImGui::TreeNode(typeid(*this).name()))
	{
		SerializeImGui();
		ImGui::TreePop();
	}
}