#pragma once
#include "SceneBase.h"

class TestScene : public SceneBase
{
public:
	TestScene() = default;
	~TestScene() override = default;
	TestScene(const TestScene&) = delete;
	TestScene& operator=(const TestScene&) = delete;
	TestScene(TestScene&&) = delete;
	TestScene& operator=(TestScene&&) = delete;

private:
	GameObjectBase* CreateCameraObject() override;
	void Begin() override;
};