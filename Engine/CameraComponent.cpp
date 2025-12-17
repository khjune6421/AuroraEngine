#include "stdafx.h"
#include "CameraComponent.h"

void CameraComponent::SerializeImGui()
{
	if (ImGui::DragFloat("FovY (radian)", &m_fovY, 0.01f, 0.1f, DirectX::XM_2PI)) UpdateProjectionMatrix();
	if (ImGui::DragInt("Screen Width", reinterpret_cast<int*>(&m_screenWidth), 1.0f, 1, 8192)) UpdateProjectionMatrix();
	if (ImGui::DragInt("Screen Height", reinterpret_cast<int*>(&m_screenHeight), 1.0f, 1, 8192)) UpdateProjectionMatrix();
	if (ImGui::DragFloat("NearZ", &m_nearZ, 0.01f, 0.01f, m_farZ - 0.01f)) UpdateProjectionMatrix();
	if (ImGui::DragFloat("FarZ", &m_farZ, 1.0f, m_nearZ + 0.01f, 10000.0f)) UpdateProjectionMatrix();
}