#include "stdafx.h"
#include "ClientTestScene.h"

#include "YdmTestObject.h"
#include "TestCameraObject.h"
#include "CamRotObject.h"

using namespace std;
using namespace DirectX;

REGISTER_TYPE(ClientTestScene)

void ClientTestScene::Initialize()
{
	CreateRootGameObject("YdmTestObject");
}

GameObjectBase* ClientTestScene::CreateCameraObject()
{
	return CreateRootGameObject<CamRotObject>("CamRotObject")->CreateChildGameObject<TestCameraObject>();
}