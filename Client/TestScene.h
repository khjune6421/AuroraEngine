#pragma once
#include "SceneBase.h"

class TestScene : public SceneBase
{
public:
	TestScene() = default;
	~TestScene() override = default;
	TestScene(const TestScene&) = default;
	TestScene& operator=(const TestScene&) = default;
	TestScene(TestScene&&) = default;
	TestScene& operator=(TestScene&&) = default;

private:
	void Initialize() override;
	GameObjectBase* CreateCameraObject() override;
};