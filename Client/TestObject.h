///TestObject.h의 시작
#pragma once
#include "GameObjectBase.h"

class TestObject : public GameObjectBase
{
public:
	TestObject() = default;
	~TestObject() override = default;
	TestObject(const TestObject&) = default;
	TestObject& operator=(const TestObject&) = default;
	TestObject(TestObject&&) = default;
	TestObject& operator=(TestObject&&) = default;

private:
	void InitializeGameObject() override;
	void UpdateGameObject(float deltaTime) override;
};

///TestObject.h의 끝