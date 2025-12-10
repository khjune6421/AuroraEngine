#pragma once
#include "GameObject.h"

class TestObject : public GameObject
{
public:
	TestObject() = default;
	~TestObject() override = default;
	TestObject(const TestObject&) = delete;
	TestObject& operator=(const TestObject&) = delete;
	TestObject(TestObject&&) = delete;
	TestObject& operator=(TestObject&&) = delete;

	void Begin() override;
	void Update(float deltaTime) override;
};