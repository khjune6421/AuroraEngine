#include "stdafx.h"
#include "TestObject.h"

#include "ModelComponent.h"

using namespace std;
using namespace DirectX;

void TestObject::Begin()
{
	AddComponent<ModelComponent>(); // 기본 생성
	SetScale({ 1.0f, 1.0f, 1.0f });
}

void TestObject::Update(float deltaTime)
{
	if (GetAsyncKeyState('W') & 0x8000) Rotate({ -deltaTime * 2.0f, 0.0f, 0.0f });
	if (GetAsyncKeyState('S') & 0x8000) Rotate({ deltaTime * 2.0f, 0.0f, 0.0f });
	if (GetAsyncKeyState('A') & 0x8000) Rotate({ 0.0f, -deltaTime * 2.0f, 0.0f });
	if (GetAsyncKeyState('D') & 0x8000) Rotate({ 0.0f, deltaTime * 2.0f, 0.0f });
	if (GetAsyncKeyState('Q') & 0x8000) Rotate({ 0.0f, 0.0f, -deltaTime * 2.0f });
	if (GetAsyncKeyState('E') & 0x8000) Rotate({ 0.0f, 0.0f, deltaTime * 2.0f });
}