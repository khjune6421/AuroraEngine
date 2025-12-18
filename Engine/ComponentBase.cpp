#include "stdafx.h"
#include "ComponentBase.h"

void ComponentBase::Initialize(GameObjectBase* owner)
{
	m_typeName = typeid(*this).name();
	if (m_typeName.find("class ") == 0) m_typeName = m_typeName.substr(6);

	m_owner = owner;

	Begin();
}

void ComponentBase::RenderImGui()
{
	if (ImGui::TreeNode(m_typeName.c_str()))
	{
		SerializeImGui();
		ImGui::TreePop();
	}
}