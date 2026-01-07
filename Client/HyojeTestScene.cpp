/// HyojeTestScene.cpp의 시작
#include "stdafx.h"
#include "HyojeTestScene.h"


#include "HyojeTestObject.h"
#include "CoordinateSystemObject.h"
//#include "BlenderCameraObject.h"
#include "TestCameraObject.h"
#include "CamRotObject.h"

using namespace std;
using namespace DirectX;

REGISTER_TYPE(HyojeTestScene)

void HyojeTestScene::Initialize()
{
	CreateRootGameObject("HyojeTestObject");
	CreateRootGameObject("CoordinateSystemObject");
}

GameObjectBase* HyojeTestScene::CreateCameraObject()
{
	return CreateRootGameObject<CamRotObject>("CamRotObject")->CreateChildGameObject<TestCameraObject>();
}