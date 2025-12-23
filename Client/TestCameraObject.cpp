#include "stdafx.h"
#include "TestCameraObject.h"

void TestCameraObject::InitializeGameObject()
{
	SetPosition({ 0.0f, 10.0f, -20.0f });
	LookAt({ 0.0f, 0.0f, 0.0f });
}

void TestCameraObject::UpdateGameObject(float deltaTime)
{
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) MoveDirection(50.0f * deltaTime, Direction::Left);
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) MoveDirection(50.0f * deltaTime, Direction::Right);
	if (GetAsyncKeyState(VK_UP) & 0x8000) MoveDirection(50.0f * deltaTime, Direction::Up);
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) MoveDirection(50.0f * deltaTime, Direction::Down);

	LookAt({ 0.0f, 0.0f, 0.0f });
}