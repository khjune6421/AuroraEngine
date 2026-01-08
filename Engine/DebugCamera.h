// BOF DebugCamera.h
#pragma once
#include "GameObjectBase.h"

#ifndef HYOJE_BLENDER_CAMERA

class DebugCamera : public GameObjectBase
{
private:
    DirectX::XMFLOAT3 m_targetPosition;

    // SPHERICAL: 타겟으로부터의 거리 및 각도
    float m_distance = 10.0f;
    float m_pitch = 0.0f; // 상하 (X축 회전)
    float m_yaw = 0.0f;   // 좌우 (Y축 회전)

    // SENSITIVITY
    float m_orbitSensitivity = 0.005f; // 회전 감도
    float m_panSensitivity = 0.01f;    // 이동 감도
    float m_wheelSensitivity = 1.0f;   // 휠 감도

public:
    DebugCamera() = default;
    ~DebugCamera() override = default;

    void Initialize() override;
    void Update() override;
    void FocusTarget(const DirectX::XMVECTOR& targetPos, float distance = 5.0f);
	void GetCameraBasis(DirectX::XMVECTOR& outRight, DirectX::XMVECTOR& outUp, DirectX::XMVECTOR& outForward);
};

#else

class DebugCamera : public GameObjectBase
{
	float m_moveSpeed = 10.0f; // 이동 속도

public:
	DebugCamera() = default;
	~DebugCamera() override = default;
	DebugCamera(const DebugCamera&) = default;
	DebugCamera& operator=(const DebugCamera&) = default;
	DebugCamera(DebugCamera&&) = default;
	DebugCamera& operator=(DebugCamera&&) = default;

	void Initialize() override;
	void Update() override;
};

#endif // HYOJE_BLENDER_CAMERA

// EOF DebugCamera.h