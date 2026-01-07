///LineObject.h의 시작
#pragma once
#include "GameObjectBase.h"

class LineObject : public GameObjectBase
{
public:
	LineObject() = default;
	~LineObject() override = default;
	LineObject(const LineObject&) = default;
	LineObject& operator=(const LineObject&) = default;
	LineObject(LineObject&&) = default;
	LineObject& operator=(LineObject&&) = default;

private:
	void Initialize() override;
	void Update() override;
};

///LineObject.h의 끝