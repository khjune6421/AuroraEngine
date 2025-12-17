#include "stdafx.h"
#include "ComponentBase.h"

void ComponentBase::RenderImGui()
{
	if (ImGui::TreeNode(m_name.c_str()))
	{
		SerializeImGui();
		ImGui::TreePop();
	}
}