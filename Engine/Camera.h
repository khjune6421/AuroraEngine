#pragma once
#include "GameObjectBase.h"

class Camera : public GameObjectBase // TODO: 컴포넌트로 수정
{
	float m_fovY = DirectX::XM_PIDIV4; // 수직 시야각 (라디안 단위)

	UINT m_screenWidth = 1280; // 화면 너비
	UINT m_screenHeight = 720; // 화면 높이

	float m_nearZ = 0.1f; // 근평면
	float m_farZ = 1000.0f; // 원평면

	DirectX::XMMATRIX m_viewMatrix = DirectX::XMMatrixIdentity(); // 뷰 행렬
	DirectX::XMMATRIX m_projectionMatrix = DirectX::XMMatrixIdentity(); // 투영 행렬

public:
	Camera() { Begin(); }
	~Camera() = default;
	Camera(const Camera&) = delete;
	Camera& operator=(const Camera&) = delete;
	Camera(Camera&&) = delete;
	Camera& operator=(Camera&&) = delete;

	void SetFovY(float fovY) { m_fovY = fovY; UpdateProjectionMatrix(); }
	void SetScreenSize(UINT width, UINT height) { m_screenWidth = width; m_screenHeight = height; UpdateProjectionMatrix(); }
	void SetNearZ(float nearZ) { m_nearZ = nearZ; UpdateProjectionMatrix(); }
	void SetFarZ(float farZ) { m_farZ = farZ; UpdateProjectionMatrix(); }

	DirectX::XMMATRIX GetViewMatrix() const { return m_viewMatrix; }
	DirectX::XMMATRIX GetProjectionMatrix() const { return m_projectionMatrix; }

	// 뷰 행렬 갱신 // 씬이 매 프레임 Update에서 호출
	void UpdateViewMatrix();
private:
	void Begin() override { UpdateViewMatrix(); UpdateProjectionMatrix(); }
	// 투영 행렬 갱신
	void UpdateProjectionMatrix() { m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(m_fovY, static_cast<float>(m_screenWidth) / static_cast<float>(m_screenHeight), m_nearZ, m_farZ); }
};