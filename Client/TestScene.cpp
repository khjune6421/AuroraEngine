#include "stdafx.h"
#include "TestScene.h"

#include "TestObject.h"

using namespace std;

void TestScene::Begin()
{
	m_mainCamera->SetPosition({ 0.0f, 5.0f, -10.0f, 1.0f });
	m_mainCamera->LookAt({ 0.0f, 0.0f, 0.0f, 1.0f });

	AddGameObject(make_unique<TestObject>());
}