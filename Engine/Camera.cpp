#include "stdafx.h"
#include "Camera.h"

using namespace DirectX;

void Camera::UpdateViewMatrix()
{
	if (!m_isDirty) return; // 위치 갱신 필요 없으면 종료

	const XMVECTOR eyePosition = GetPosition();
	constexpr XMVECTOR upVector = { 0.0f, 1.0f, 0.0f, 0.0f };

	m_viewMatrix = XMMatrixLookAtLH(eyePosition, XMVectorAdd(eyePosition, GetDirectionVector(Direction::Forward)), upVector);
}