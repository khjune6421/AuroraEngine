#include "stdafx.h"
#include "CamRotObject.h"
#include "InputManager.h"

#include "TimeManager.h"

REGISTER_TYPE(CamRotObject)

void CamRotObject::Update()
{
	float deltaTime = TimeManager::GetInstance().GetDeltaTime();

	using enum KeyCode;
	auto& input = InputManager::GetInstance();

	if (input.GetKey(Left)) Rotate({ 0.0f, deltaTime * 45.0f, 0.0f });
	if (input.GetKey(Right)) Rotate({ 0.0f, -deltaTime * 45.0f, 0.0f });
	if (input.GetKey(Up)) Rotate({ deltaTime * 45.0f, 0.0f, 0.0f });
	if (input.GetKey(Down)) Rotate({ -deltaTime * 45.0f, 0.0f, 0.0f });

}