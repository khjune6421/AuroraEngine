#include "stdafx.h"
#include "YdmTestObject.h"

#include "ModelComponent.h"
#include "NetworkTestComponent.h"
#include "NetworkIdentityComponent.h"
#include "TimeManager.h"
#include "InputManager.h"

using namespace std;
using namespace DirectX;

REGISTER_TYPE(YdmTestObject)

void YdmTestObject::Initialize()
{
	CreateComponent<NetworkTestComponent>();
	SetScale({ 1.0f, 1.0f, 1.0f });
}

void YdmTestObject::Update()
{
	float deltaTime = TimeManager::GetInstance().GetDeltaTime();

	float m_moveSpeed = 10.0f;

	using enum KeyCode;
	auto& input = InputManager::GetInstance();

	if (input.GetKey(KeyCode::Left)) MoveDirection(deltaTime * m_moveSpeed, Direction::Left);
	if (input.GetKey(KeyCode::Right)) MoveDirection(deltaTime * m_moveSpeed, Direction::Right);
	if (input.GetKey(KeyCode::Up)) MoveDirection(deltaTime * m_moveSpeed, Direction::Forward);
	if (input.GetKey(KeyCode::Down)) MoveDirection(deltaTime * m_moveSpeed, Direction::Backward);
	if (input.GetKey(KeyCode::Space)) MoveDirection(deltaTime * m_moveSpeed, Direction::Up);
	if (input.GetKey(KeyCode::Shift)) MoveDirection(deltaTime * m_moveSpeed, Direction::Down);

	if (input.GetKey(J)) Rotate({ -deltaTime * 45.0f, 0.0f, 0.0f });
	if (input.GetKey(K)) Rotate({ deltaTime * 45.0f, 0.0f, 0.0f });
	if (input.GetKey(A)) Rotate({ 0.0f, -deltaTime * 45.0f, 0.0f });
	if (input.GetKey(D)) Rotate({ 0.0f, deltaTime * 45.0f, 0.0f });
	if (input.GetKey(Q)) Rotate({ 0.0f, 0.0f, -deltaTime * 45.0f });
	if (input.GetKey(E)) Rotate({ 0.0f, 0.0f, deltaTime * 45.0f });
}