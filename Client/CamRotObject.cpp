#include "stdafx.h"
#include "CamRotObject.h"

#include "TimeManager.h"

void CamRotObject::Update()
{
	float deltaTime = TimeManager::GetInstance().GetDeltaTime();

	if (GetAsyncKeyState(VK_LEFT) & 0x8000) Rotate({ 0.0f, deltaTime * 45.0f, 0.0f });
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) Rotate({ 0.0f, -deltaTime * 45.0f, 0.0f });
	if (GetAsyncKeyState(VK_UP) & 0x8000) Rotate({ deltaTime * 45.0f, 0.0f, 0.0f });
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) Rotate({ -deltaTime * 45.0f, 0.0f, 0.0f });
}