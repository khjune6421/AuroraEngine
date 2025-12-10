#include "stdafx.h"
#include "TestObject.h"

#include "ModelComponent.h"

using namespace DirectX;

void TestObject::Update(float deltaTime)
{
	// 오브젝트를 천천히 회전시킴
	XMVECTOR rotation = GetRotation();
	rotation = XMVectorAdd(rotation, XMVectorSet(0.0f, deltaTime * 0.5f, 0.0f, 0.0f));
	SetRotation(rotation);
}

void TestObject::Begin()
{
	AddComponent<ModelComponent>();
}