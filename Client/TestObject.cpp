#include "stdafx.h"
#include "TestObject.h"

#include "ModelComponent.h"

using namespace std;
using namespace DirectX;

void TestObject::InitializeGameObject()
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

void TestObject::UpdateGameObject(float deltaTime)
{
	if (GetAsyncKeyState('W') & 0x8000) Rotate({ -deltaTime, 0.0f, 0.0f });
	if (GetAsyncKeyState('S') & 0x8000) Rotate({ deltaTime, 0.0f, 0.0f });
	if (GetAsyncKeyState('A') & 0x8000) Rotate({ 0.0f, -deltaTime, 0.0f });
	if (GetAsyncKeyState('D') & 0x8000) Rotate({ 0.0f, deltaTime, 0.0f });
	if (GetAsyncKeyState('Q') & 0x8000) Rotate({ 0.0f, 0.0f, -deltaTime });
	if (GetAsyncKeyState('E') & 0x8000) Rotate({ 0.0f, 0.0f, deltaTime });
}