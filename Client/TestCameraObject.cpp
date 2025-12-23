#include "stdafx.h"
#include "TestCameraObject.h"

void TestCameraObject::InitializeGameObject()
{
	SetPosition({ 0.0f, 10.0f, -20.0f });
	LookAt({ 0.0f, 0.0f, 0.0f });
}

void TestCameraObject::UpdateGameObject(float deltaTime)
{
	LookAt({ 0.0f, 0.0f, 0.0f });
}