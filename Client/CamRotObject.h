#pragma once
#include "GameObjectBase.h"

class CamRotObject : public GameObjectBase
{
public:
	CamRotObject() = default;
	~CamRotObject() override = default;
	CamRotObject(const CamRotObject&) = default;
	CamRotObject& operator=(const CamRotObject&) = default;
	CamRotObject(CamRotObject&&) = default;
	CamRotObject& operator=(CamRotObject&&) = default;

private:
	void Update() override;
};

