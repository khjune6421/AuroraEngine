#include "stdafx.h"
#include "TestObject.h"

#include "ModelComponent.h"

using namespace std;
using namespace DirectX;

void TestObject::Begin()
{
	//AddComponent<ModelComponent>();
	ModelComponent modelComp;
	AddComponent<ModelComponent>(modelComp); // 복사 생성
	SetScale({ 0.025f, 0.025f, 0.025f });
}

void TestObject::Update(float deltaTime)
{
}