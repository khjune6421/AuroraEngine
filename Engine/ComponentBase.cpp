#include "stdafx.h"
#include "ComponentBase.h"

void ComponentBase::BaseInitialize()
{
	m_typeName = typeid(*this).name();
	if (m_typeName.find("class ") == 0) m_typeName = m_typeName.substr(6);

	Initialize();
}

void ComponentBase::BaseRenderImGui()
{
	if (ImGui::TreeNode(m_typeName.c_str()))
	{
		RenderImGui();

		ImGui::TreePop();
	}
}