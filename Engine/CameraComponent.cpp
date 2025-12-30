#include "stdafx.h"
#include "CameraComponent.h"

#include "GameObjectBase.h"
#include "Renderer.h"

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
		XMVectorAdd(*m_position, m_owner->GetWorldDirectionVector(Direction::Forward)), // 카메라 앞 방향
		m_owner->GetWorldDirectionVector(Direction::Up) // 카메라 위 방향
	);
}

void CameraComponent::UpdateProjectionMatrix()
{
	m_projectionMatrix = XMMatrixPerspectiveFovLH(m_fovY, Renderer::GetInstance().GetAspectRatio(), m_nearZ, m_farZ);
}

void CameraComponent::RenderImGui()
{
	float fovYInDegrees = XMConvertToDegrees(m_fovY);
	if (ImGui::DragFloat("FovY", &fovYInDegrees, 0.1f, 1.0f, 179.0f)) m_fovY = XMConvertToRadians(fovYInDegrees);
	ImGui::DragFloat("NearZ", &m_nearZ, 0.01f, 0.01f, m_farZ - 0.01f);
	ImGui::DragFloat("FarZ", &m_farZ, 1.0f, m_nearZ + 0.01f, 10000.0f);
}