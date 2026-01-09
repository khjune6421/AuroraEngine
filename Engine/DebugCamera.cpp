#include "stdafx.h"
#include "DebugCamera.h"

#include "InputManager.h"
#include "TimeManager.h"

void DebugCamera::Initialize()
{
	SetPosition({ 0.0f, 5.0f, -10.0f });
	LookAt({ 0.0f, 0.0f, 0.0f });
}

void DebugCamera::Update()
{
	float deltaTime = TimeManager::GetInstance().GetDeltaTime();
	InputManager& input = InputManager::GetInstance();

	/*if (input.GetKey(KeyCode::Left)) MoveDirection(deltaTime * m_moveSpeed, Direction::Left);
	if (input.GetKey(KeyCode::Right)) MoveDirection(deltaTime * m_moveSpeed, Direction::Right);
	if (input.GetKey(KeyCode::Up)) MoveDirection(deltaTime * m_moveSpeed, Direction::Forward);
	if (input.GetKey(KeyCode::Down)) MoveDirection(deltaTime * m_moveSpeed, Direction::Backward);
	if (input.GetKey(KeyCode::Space)) MoveDirection(deltaTime * m_moveSpeed, Direction::Up);
	if (input.GetKey(KeyCode::Shift)) MoveDirection(deltaTime * m_moveSpeed, Direction::Down);*/

	deltaTime *= 15.0f; // 회전 속도 보정
	if (input.GetKey(KeyCode::W)) Rotate({ -deltaTime * m_moveSpeed, 0.0f, 0.0f, 0.0f });
	if (input.GetKey(KeyCode::S)) Rotate({ deltaTime * m_moveSpeed, 0.0f, 0.0f, 0.0f });
	if (input.GetKey(KeyCode::A)) Rotate({ 0.0f, -deltaTime * m_moveSpeed, 0.0f, 0.0f });
	if (input.GetKey(KeyCode::D)) Rotate({ 0.0f, deltaTime * m_moveSpeed, 0.0f, 0.0f });
	if (input.GetKey(KeyCode::E)) Rotate({ 0.0f, 0.0f, -deltaTime * m_moveSpeed, 0.0f });
	if (input.GetKey(KeyCode::Q)) Rotate({ 0.0f, 0.0f, deltaTime * m_moveSpeed, 0.0f });
}