#include "stdafx.h"
#include "TestCameraObject.h"

void TestCameraObject::Begin()
{
	SetPosition({ 0.0f, 10.0f, -20.0f, 1.0f });
	LookAt({ 0.0f, 0.0f, 0.0f, 1.0f });
}

void TestCameraObject::Update(float deltaTime)
{
	if (GetAsyncKeyState('W') & 0x8000) MoveDirection(20.0f * deltaTime, Direction::Up);
	if (GetAsyncKeyState('S') & 0x8000) MoveDirection(20.0f * deltaTime, Direction::Down);
	if (GetAsyncKeyState('A') & 0x8000) MoveDirection(20.0f * deltaTime, Direction::Left);
	if (GetAsyncKeyState('D') & 0x8000) MoveDirection(20.0f * deltaTime, Direction::Right);

	if (GetAsyncKeyState('Q') & 0x8000) Rotate({ 0.0f, 0.0f, -deltaTime * 2.0f, 0.0f });
	if (GetAsyncKeyState('E') & 0x8000) Rotate({ 0.0f, 0.0f, deltaTime * 2.0f, 0.0f });

	LookAt({ 0.0f, 0.0f, 0.0f, 1.0f });
}