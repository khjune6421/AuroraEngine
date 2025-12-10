#pragma once
#include "GameObjectBase.h"

class MainCamera : public GameObjectBase // TODO: 컴포넌트로 수정
{
	const UINT m_screenWidth = 1280; // 화면 너비
	const UINT m_screenHeight = 720; // 화면 높이
	const float m_aspectRatio = static_cast<float>(m_screenWidth) / static_cast<float>(m_screenHeight); // 종횡비

	const float m_fovY = DirectX::XM_PIDIV4; // 수직 시야각 (라디안 단위)
	const float m_nearZ = 0.1f; // 근평면
	const float m_farZ = 1000.0f; // 원평면

public:
	MainCamera() = default;
	~MainCamera() = default;
	MainCamera(const MainCamera&) = delete;
	MainCamera& operator=(const MainCamera&) = delete;
	MainCamera(MainCamera&&) = delete;
	MainCamera& operator=(MainCamera&&) = delete;

	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetProjectionMatrix() const;

protected:
	void Begin() override {}
};