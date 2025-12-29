#include "stdafx.h"
#include "CameraComponent.h"

#include "GameObjectBase.h"

using namespace DirectX;

void CameraComponent::Initialize()
{
	m_position = &m_owner->GetWorldMatrix().r[3];
	UpdateProjectionMatrix();
}

void CameraComponent::UpdateViewMatrix()
{
	m_viewMatrix = XMMatrixLookAtLH
	(
		*m_position, // 카메라 위치
		XMVectorAdd(*m_position, m_owner->GetDirectionVector(Direction::Forward)), // 카메라 앞 방향
		m_owner->GetDirectionVector(Direction::Up) // 카메라 위 방향
	);
}

void CameraComponent::UpdateProjectionMatrix()
{
	m_projectionMatrix = XMMatrixPerspectiveFovLH(m_fovY, static_cast<float>(m_screenWidth) / static_cast<float>(m_screenHeight), m_nearZ, m_farZ);
}

void CameraComponent::RenderImGui()
{
	if (ImGui::DragFloat("FovY (radian)", &m_fovY, 0.01f, 0.1f, XM_2PI)) UpdateProjectionMatrix();
	if (ImGui::DragInt("Screen Width", reinterpret_cast<int*>(&m_screenWidth), 1.0f, 1, 8192)) UpdateProjectionMatrix();
	if (ImGui::DragInt("Screen Height", reinterpret_cast<int*>(&m_screenHeight), 1.0f, 1, 8192)) UpdateProjectionMatrix();
	if (ImGui::DragFloat("NearZ", &m_nearZ, 0.01f, 0.01f, m_farZ - 0.01f)) UpdateProjectionMatrix();
	if (ImGui::DragFloat("FarZ", &m_farZ, 1.0f, m_nearZ + 0.01f, 10000.0f)) UpdateProjectionMatrix();
}