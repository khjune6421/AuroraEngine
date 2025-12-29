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
	void Initialize() override;
	void Update(float deltaTime) override;
};