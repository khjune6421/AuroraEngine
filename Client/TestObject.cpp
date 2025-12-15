#include "stdafx.h"
#include "TestObject.h"

#include "ModelComponent.h"

using namespace std;
using namespace DirectX;

void TestObject::Begin()
{
	//AddComponent<ModelComponent>();
	ModelComponent modelComp;
	AddComponent<ModelComponent>(modelComp);
}

void TestObject::Update(float deltaTime)
{
}