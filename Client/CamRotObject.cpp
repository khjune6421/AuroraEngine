#include "stdafx.h"
#include "CamRotObject.h"

void CamRotObject::InitializeGameObject()
{
}

void CamRotObject::UpdateGameObject(float deltaTime)
{
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) Rotate({ 0.0f, -deltaTime, 0.0f });
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) Rotate({ 0.0f, deltaTime, 0.0f });
	if (GetAsyncKeyState(VK_UP) & 0x8000) Rotate({ -deltaTime, 0.0f, 0.0f });
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) Rotate({ deltaTime, 0.0f, 0.0f });
}