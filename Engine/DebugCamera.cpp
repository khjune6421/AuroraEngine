#include "stdafx.h"
#include "DebugCamera.h"

#include "InputManager.h"
#include "TimeManager.h"

void DebugCamera::Initialize()
{
	SetPosition({ 0.0f, 5.0f, -10.0f });
}

void DebugCamera::Update()
{
	float deltaTime = TimeManager::GetInstance().GetDeltaTime();

	using enum KeyCode;
	auto& input = InputManager::GetInstance();

	if (input.GetKey(Left)) MoveDirection(deltaTime * m_moveSpeed, Direction::Left);
	if (input.GetKey(Right)) MoveDirection(deltaTime * m_moveSpeed, Direction::Right);
	if (input.GetKey(Up)) MoveDirection(deltaTime * m_moveSpeed, Direction::Up);
	if (input.GetKey(Down)) MoveDirection(deltaTime * m_moveSpeed, Direction::Down);
	if (input.GetKey(Shift)) MoveDirection(deltaTime * m_moveSpeed, Direction::Backward);
	if (input.GetKey(Space)) MoveDirection(deltaTime * m_moveSpeed, Direction::Forward);

	LookAt({ 0.0f, 0.0f, 0.0f }, GetWorldDirectionVector(Direction::Up));
}