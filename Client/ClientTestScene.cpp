#include "stdafx.h"
#include "ClientTestScene.h"

#include "YdmTestObject.h"
#include "CoordinateSystemObject.h"
#include "TestCameraObject.h"
#include "CamRotObject.h"

using namespace std;
using namespace DirectX;

REGISTER_TYPE(ClientTestScene)

void ClientTestScene::Initialize()
{
	CreateRootGameObject("YdmTestObject");
	CreateRootGameObject("CoordinateSystemObject");
}

GameObjectBase* ClientTestScene::CreateCameraObject()
{
	return CreateRootGameObject<CamRotObject>("CamRotObject")->CreateChildGameObject<TestCameraObject>();
}