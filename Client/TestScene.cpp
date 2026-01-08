/// TestScene.cpp의 시작
#include "stdafx.h"
#include "TestScene.h"

#include "TestObject.h"
#include "TestCameraObject.h"
#include "CamRotObject.h"

using namespace std;

REGISTER_TYPE(TestScene)

void TestScene::Initialize()
{
	CreateRootGameObject<TestObject>();
}

GameObjectBase* TestScene::CreateCameraObject()
{
	return CreateRootGameObject<CamRotObject>("CamRotObject")->CreateChildGameObject<TestCameraObject>();
}

/// TestScene.cpp의 끝