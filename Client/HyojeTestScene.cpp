/// HyojeTestScene.cpp의 시작
#include "stdafx.h"
#include "HyojeTestScene.h"


#include "HyojeTestObject.h"
#include "TestCameraObject.h"
#include "CamRotObject.h"

using namespace std;
using namespace DirectX;

REGISTER_TYPE(HyojeTestScene)

void HyojeTestScene::Initialize()
{
	CreateRootGameObject("HyojeTestObject");
}

GameObjectBase* HyojeTestScene::CreateCameraObject()
{
	return CreateRootGameObject<CamRotObject>("CamRotObject")->CreateChildGameObject<TestCameraObject>();
}