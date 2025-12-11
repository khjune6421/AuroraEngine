#include "stdafx.h"
#include "TestCameraObject.h"

void TestCameraObject::Begin()
{
	SetPosition({ 0.0f, 5.0f, -10.0f, 1.0f });
	LookAt({ 0.0f, 0.0f, 0.0f, 1.0f });
}

void TestCameraObject::Update(float deltaTime)
{
	if (GetAsyncKeyState('W') & 0x8000) MoveDirection(10.0f * deltaTime, Direction::Up);
	if (GetAsyncKeyState('S') & 0x8000) MoveDirection(10.0f * deltaTime, Direction::Down);
	if (GetAsyncKeyState('A') & 0x8000) MoveDirection(10.0f * deltaTime, Direction::Left);
	if (GetAsyncKeyState('D') & 0x8000) MoveDirection(10.0f * deltaTime, Direction::Right);

	LookAt({ 0.0f, 0.0f, 0.0f, 1.0f });
}