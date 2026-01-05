#include "stdafx.h"
#include "ComponentBase.h"

void ComponentBase::BaseInitialize()
{
	m_type = GetTypeName(*this);

	Initialize();
}

void ComponentBase::BaseRenderImGui()
{
	ImGui::PushID(this);

	if (ImGui::Button("Remove")) SetAlive(false);

	ImGui::SameLine();
	if (ImGui::TreeNode(m_type.c_str()))
	{
		RenderImGui();

		ImGui::TreePop();
	}

	ImGui::PopID();
}

nlohmann::json ComponentBase::BaseSerialize()
{
	nlohmann::json jsonData;

	// 컴포넌트 타입 저장
	jsonData["type"] = m_type;

	// 파생 클래스의 직렬화 호출
	nlohmann::json derivedData = Serialize();
	if (!derivedData.is_null() && derivedData.is_object()) jsonData.merge_patch(derivedData);

	return jsonData;
}