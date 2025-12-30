///TestObject.cpp의 시작
#include "stdafx.h"
#include "TestObject.h"

#include "ModelComponent.h"
#include "InputManager.h"

using namespace std;
using namespace DirectX;

void TestObject::Initialize()
{
	CreateComponent<ModelComponent>(); // 기본 생성
	SetScale({ 1.0f, 1.0f, 1.0f });

	static int count = 0;
	if (count < 10)
	{
		count++;
		CreateChildGameObject<TestObject>()->SetPosition({ 0.0f, 0.0f, 2.0f });
	}
}

void TestObject::Update(float deltaTime)
{
	using enum KeyCode;
	auto& input = InputManager::GetInstance();

	if (input.GetKey(W)) Rotate({ -deltaTime * 45.0f, 0.0f, 0.0f });
	if (input.GetKey(S)) Rotate({ deltaTime * 45.0f, 0.0f, 0.0f });
	if (input.GetKey(A)) Rotate({ 0.0f, -deltaTime * 45.0f, 0.0f });
	if (input.GetKey(D)) Rotate({ 0.0f, deltaTime * 45.0f, 0.0f });
	if (input.GetKey(Q)) Rotate({ 0.0f, 0.0f, -deltaTime * 45.0f });
	if (input.GetKey(E)) Rotate({ 0.0f, 0.0f, deltaTime * 45.0f });

	//if (GetAsyncKeyState('W') & 0x8000) Rotate({ -deltaTime * 45.0f, 0.0f, 0.0f });
	//if (GetAsyncKeyState('S') & 0x8000) Rotate({ deltaTime * 45.0f, 0.0f, 0.0f });
	//if (GetAsyncKeyState('A') & 0x8000) Rotate({ 0.0f, -deltaTime * 45.0f, 0.0f });
	//if (GetAsyncKeyState('D') & 0x8000) Rotate({ 0.0f, deltaTime * 45.0f, 0.0f });
	//if (GetAsyncKeyState('Q') & 0x8000) Rotate({ 0.0f, 0.0f, -deltaTime * 45.0f });
	//if (GetAsyncKeyState('E') & 0x8000) Rotate({ 0.0f, 0.0f, deltaTime * 45.0f });
}

///TestObject.cpp의 끝