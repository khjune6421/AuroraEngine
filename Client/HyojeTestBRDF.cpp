/// HyojeTestBRDF.cpp¿« Ω√¿€
#include "stdafx.h"
#include "HyojeTestBRDF.h"


#include "HyojeTestObject.h"
#include "TestCameraObject.h"
#include "CamRotObject.h"

using namespace std;
using namespace DirectX;


void HyojeTestBRDF::InitializeScene()
{
	CreateRootGameObject<HyojeTestObject>();
}

GameObjectBase* HyojeTestBRDF::CreateCameraObject()
{
	return CreateRootGameObject<CamRotObject>()->CreateChildGameObject<TestCameraObject>();
}