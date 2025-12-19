#include "stdafx.h"
#include "TestScene.h"

#include "TestObject.h"
#include "TestCameraObject.h"

using namespace std;

GameObjectBase* TestScene::CreateCameraObject()
{
	return CreateGameObject<TestCameraObject>();
}

void TestScene::Begin()
{
	CreateGameObject<TestObject>();
	CreateGameObject<TestObject>()->SetPosition({ 3.0f, 0.0f, 0.0f });
}