#include "stdafx.h"
#include "TestScene.h"

#include "TestObject.h"
#include "TestCameraObject.h"

using namespace std;

void TestScene::InitializeScene()
{
	CreateRootGameObject<TestObject>();
}

GameObjectBase* TestScene::CreateCameraObject()
{
	return CreateRootGameObject<TestCameraObject>();
}