#include "stdafx.h"
#include "TestScene.h"

#include "TestObject.h"
#include "TestCameraObject.h"

using namespace std;

GameObjectBase* TestScene::CreateCameraObject()
{
	return AddGameObject(make_unique<TestCameraObject>());
}

void TestScene::Begin()
{
	AddGameObject(make_unique<TestObject>());
	AddGameObject(make_unique<TestObject>())->SetPosition({ 3.0f, 0.0f, 0.0f, 1.0f });
	AddGameObject(make_unique<TestObject>())->SetPosition({ -3.0f, 0.0f, 0.0f, 1.0f });
}