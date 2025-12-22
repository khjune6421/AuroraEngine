#include "stdafx.h"
#include "TestScene.h"

#include "TestObject.h"
#include "TestCameraObject.h"

using namespace std;

void TestScene::InitializeScene()
{
	CreateGameObject<TestObject>();
	CreateGameObject<TestObject>()->SetPosition({ 3.0f, 0.0f, 0.0f });
}

GameObjectBase* TestScene::CreateCameraObject()
{
	return CreateGameObject<TestCameraObject>();
}