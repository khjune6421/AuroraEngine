///TestObject.cpp의 시작
#include "stdafx.h"
#include "TestObject.h"

#include "ModelComponent.h"
#include "TimeManager.h"
#include "InputManager.h"

using namespace std;
using namespace DirectX;

REGISTER_TYPE(TestObject)

void TestObject::Initialize()
{
	CreateComponent<ModelComponent>(); // 기본 생성
	SetScale({ 1.0f, 1.0f, 1.0f });
}

void TestObject::Update()
{
	float deltaTime = TimeManager::GetInstance().GetDeltaTime();

	using enum KeyCode;
	auto& input = InputManager::GetInstance();

	if (input.GetKey(W)) Rotate({ -deltaTime * 45.0f, 0.0f, 0.0f });
	if (input.GetKey(S)) Rotate({ deltaTime * 45.0f, 0.0f, 0.0f });
	if (input.GetKey(A)) Rotate({ 0.0f, -deltaTime * 45.0f, 0.0f });
	if (input.GetKey(D)) Rotate({ 0.0f, deltaTime * 45.0f, 0.0f });
	if (input.GetKey(Q)) Rotate({ 0.0f, 0.0f, -deltaTime * 45.0f });
	if (input.GetKey(E)) Rotate({ 0.0f, 0.0f, deltaTime * 45.0f });
}

///TestObject.cpp의 끝