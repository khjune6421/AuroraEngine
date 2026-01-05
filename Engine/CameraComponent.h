#pragma once
#include "ComponentBase.h"

class CameraComponent : public ComponentBase
{
	float m_fovY = DirectX::XM_PIDIV4; // 수직 시야각 (라디안 단위)

	UINT m_screenWidth = 1280; // 화면 너비
	UINT m_screenHeight = 720; // 화면 높이

	float m_nearZ = 0.1f; // 근평면
	float m_farZ = 1000.0f; // 원평면

	DirectX::XMMATRIX m_viewMatrix = DirectX::XMMatrixIdentity(); // 뷰 행렬
	DirectX::XMMATRIX m_projectionMatrix = DirectX::XMMatrixIdentity(); // 투영 행렬
	const DirectX::XMVECTOR* m_position = nullptr; // 카메라 위치

public:
	CameraComponent() = default;
	~CameraComponent() override = default;
	CameraComponent(const CameraComponent&) = default;
	CameraComponent& operator=(const CameraComponent&) = default;
	CameraComponent(CameraComponent&&) = default;
	CameraComponent& operator=(CameraComponent&&) = default;

	void SetFovY(float fovY) { m_fovY = fovY; }
	void SetNearZ(float nearZ) { m_nearZ = nearZ; }
	void SetFarZ(float farZ) { m_farZ = farZ; }

	const DirectX::XMMATRIX& GetViewMatrix() const { return m_viewMatrix; }
	const DirectX::XMMATRIX& GetProjectionMatrix() const { return m_projectionMatrix; }
	const DirectX::XMVECTOR& GetPosition() const { return *m_position; }

	bool NeedsUpdate() const override { return true; }
	bool NeedsRender() const override { return false; }

private:
	void Initialize() override;
	// 위치, 뷰 행렬 갱신
	void Update() override { UpdateViewMatrix(); UpdateProjectionMatrix(); }
	void RenderImGui() override;

	nlohmann::json Serialize() override;
	void Deserialize(const nlohmann::json& jsonData) override;

	// 뷰 행렬 갱신
	void UpdateViewMatrix();
	// 투영 행렬 갱신
	void UpdateProjectionMatrix();
};