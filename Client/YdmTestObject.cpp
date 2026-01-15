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
	auto* netIdComp = GetComponent<NetworkIdentityComponent>();
	bool isNetworked = (netIdComp && netIdComp->GetNetId() != 0);

	float m_moveSpeed = 10.0f;

	using enum KeyCode;
	auto& input = InputManager::GetInstance();

	if (isNetworked)
	{
		auto& net = NetManager::GetInstance();
		if (!net.IsConnected()) return;

		uint32_t actor = netIdComp->GetNetId();
		uint32_t turn = NetworkWorld::GetLocalTurn();
		uint32_t myPeerId = NetManager::GetInstance().GetSelfPeerId();

		if (input.GetKey(KeyCode::Left))
		{
			if (net.IsHost()) NetworkWorld::QueueActionMove(turn, myPeerId, actor, (uint8_t)MoveDir::Left, 1.0f);
			else             NetworkWorld::SendActionMove(turn, myPeerId, actor, (uint8_t)MoveDir::Left, 1.0f);
		}
		if (input.GetKey(KeyCode::Right))
		{
			if (net.IsHost()) NetworkWorld::QueueActionMove(turn, myPeerId, actor, (uint8_t)MoveDir::Right, 1.0f);
			else             NetworkWorld::SendActionMove(turn, myPeerId, actor, (uint8_t)MoveDir::Right, 1.0f);
		}

		if (input.GetKeyDown(KeyCode::Enter))
		{
			if (net.IsHost()) NetworkWorld::QueueActionEndTurn(turn, myPeerId, actor);
			else             NetworkWorld::SendActionEndTurn(turn, myPeerId, actor);
		}

		return;
	}

	// 로컬 오브젝트일 때:즉시적용
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