#include "stdafx.h"
#include "TestScene.h"

#include "TestObject.h"
#include "TestCameraObject.h"
#include "CamRotObject.h"

using namespace std;

void TestScene::Initialize()
{
	CreateRootGameObject<TestObject>();
}

GameObjectBase* TestScene::CreateCameraObject()
{
	return CreateRootGameObject<CamRotObject>()->CreateChildGameObject<TestCameraObject>();
}