#include "stdafx.h"
#include "ComponentBase.h"

void ComponentBase::BaseInitialize()
{
	m_type = GetTypeName(*this);

	Initialize();
}

void ComponentBase::BaseRenderImGui()
{
	if (ImGui::TreeNode(m_type.c_str()))
	{
		RenderImGui();

		ImGui::TreePop();
	}
}