#pragma once
#include "GameObjectBase.h"

class TestObject : public GameObjectBase
{
public:
	TestObject() = default;
	~TestObject() override = default;
	TestObject(const TestObject&) = delete;
	TestObject& operator=(const TestObject&) = delete;
	TestObject(TestObject&&) = delete;
	TestObject& operator=(TestObject&&) = delete;

private:
	void Begin() override;
	void Update(float deltaTime) override;
};