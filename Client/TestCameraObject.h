#pragma once
#include "GameObjectBase.h"

class TestCameraObject : public GameObjectBase
{
public:
	TestCameraObject() = default;
	~TestCameraObject() override = default;
	TestCameraObject(const TestCameraObject&) = delete;
	TestCameraObject& operator=(const TestCameraObject&) = delete;
	TestCameraObject(TestCameraObject&&) = delete;
	TestCameraObject& operator=(TestCameraObject&&) = delete;

private:
	void Begin() override;
	void Update(float deltaTime) override;
};