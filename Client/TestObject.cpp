///TestObject.cpp의 시작
#include "stdafx.h"
#include "TestObject.h"

#include "ModelComponent.h"

using namespace std;
using namespace DirectX;

REGISTER_TYPE(TestObject)

void TestObject::Initialize()
{
	CreateComponent<ModelComponent>();
	SetScale({ 1.0f, 1.0f, 1.0f });
}

void TestObject::Update()
{
}

///TestObject.cpp의 끝