///HyojeTestObject.cpp의 시작
#include "stdafx.h"
#include "HyojeTestObject.h"

#include "ModelComponent.h"

using namespace std;
using namespace DirectX;

void HyojeTestObject::Initialize()
{
	CreateComponent<ModelComponent>(); // 기본 생성
	SetScale({ 1.0f, 1.0f, 1.0f });
}

void HyojeTestObject::Update(float deltaTime)
{
	if (GetAsyncKeyState('W') & 0x8000) Rotate({ -deltaTime * 45.0f, 0.0f, 0.0f });
	if (GetAsyncKeyState('S') & 0x8000) Rotate({ deltaTime * 45.0f, 0.0f, 0.0f });
	if (GetAsyncKeyState('A') & 0x8000) Rotate({ 0.0f, -deltaTime * 45.0f, 0.0f });
	if (GetAsyncKeyState('D') & 0x8000) Rotate({ 0.0f, deltaTime * 45.0f, 0.0f });
	if (GetAsyncKeyState('Q') & 0x8000) Rotate({ 0.0f, 0.0f, -deltaTime * 45.0f });
	if (GetAsyncKeyState('E') & 0x8000) Rotate({ 0.0f, 0.0f, deltaTime * 45.0f });
}

///HyojeTestObject.cpp의 끝