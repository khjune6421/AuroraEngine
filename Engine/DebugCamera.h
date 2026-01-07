#pragma once
#include "GameObjectBase.h"

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