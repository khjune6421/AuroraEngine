///HyojeTestObject.h의 시작
#pragma once
#include "GameObjectBase.h"

class HyojeTestObject : public GameObjectBase
{
public:
	HyojeTestObject() = default;
	~HyojeTestObject() override = default;
	HyojeTestObject(const HyojeTestObject&) = default;
	HyojeTestObject& operator=(const HyojeTestObject&) = default;
	HyojeTestObject(HyojeTestObject&&) = default;
	HyojeTestObject& operator=(HyojeTestObject&&) = default;

private:
	void Initialize() override;
	void Update(float deltaTime) override;
};

///HyojeTestObject.h의 끝