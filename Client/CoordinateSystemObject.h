///CoordinateSystemObject.h의 시작
#pragma once
#include "GameObjectBase.h"

class CoordinateSystemObject : public GameObjectBase
{
public:
	CoordinateSystemObject() = default;
	~CoordinateSystemObject() override = default;
	CoordinateSystemObject(const CoordinateSystemObject&) = default;
	CoordinateSystemObject& operator=(const CoordinateSystemObject&) = default;
	CoordinateSystemObject(CoordinateSystemObject&&) = default;
	CoordinateSystemObject& operator=(CoordinateSystemObject&&) = default;

private:
	void Initialize() override;
	void Update() override;
};

///CoordinateSystemObject.h의 끝