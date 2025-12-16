#include "stdafx.h"
#include "TestObject.h"

#include "ModelComponent.h"

using namespace std;
using namespace DirectX;

void TestObject::Begin()
{
	//AddComponent<ModelComponent>(); // 扁夯 积己
	ModelComponent modelComp;
	AddComponent<ModelComponent>(modelComp); // 汗荤 积己
	SetScale({ 1.0f, 1.0f, 1.0f });
}

void TestObject::Update(float deltaTime)
{
}