#include "stdafx.h"
#include "Camera.h"

using namespace DirectX;

XMMATRIX Camera::GetViewMatrix() const
{
	const XMVECTOR eyePosition = GetPosition();
	const XMVECTOR forwardVector = XMVector4Normalize({ 0.0f, -0.5f, 1.0f, 0.0f });
	constexpr XMVECTOR upVector = { 0.0f, 1.0f, 0.0f, 0.0f };

	return XMMatrixLookAtLH(eyePosition, XMVectorAdd(eyePosition, forwardVector), upVector);
}

XMMATRIX Camera::GetProjectionMatrix() const
{
	return XMMatrixPerspectiveFovLH(m_fovY, m_aspectRatio, m_nearZ, m_farZ);
}
