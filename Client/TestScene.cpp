#include "stdafx.h"
#include "TestScene.h"

#include "TestObject.h"
#include "TestCameraObject.h"

using namespace std;

GameObjectBase* TestScene::CreateCameraObject()
{
	return AddGameObject<TestCameraObject>();
}

void TestScene::Begin()
{
	AddGameObject<TestObject>();
	AddGameObject<TestObject>()->SetPosition({ 3.0f, 0.0f, 0.0f, 1.0f });
	AddGameObject<TestObject>()->SetPosition({ -3.0f, 0.0f, 0.0f, 1.0f });
	AddGameObject<TestObject>()->SetPosition({ 0.0f, 3.0f, 0.0f, 1.0f });
	AddGameObject<TestObject>()->SetPosition({ 0.0f, -3.0f, 0.0f, 1.0f });
	AddGameObject<TestObject>()->SetPosition({ 0.0f, 0.0f, 3.0f, 1.0f });
	AddGameObject<TestObject>()->SetPosition({ 0.0f, 0.0f, -3.0f, 1.0f });
}