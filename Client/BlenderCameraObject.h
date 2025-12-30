#pragma once
#include "GameObjectBase.h"

class BlenderCameraObject : public GameObjectBase
{
public:
	BlenderCameraObject() = default;
	~BlenderCameraObject() override = default;
	BlenderCameraObject(const BlenderCameraObject&) = default;
	BlenderCameraObject& operator=(const BlenderCameraObject&) = default;
	BlenderCameraObject(BlenderCameraObject&&) = default;
	BlenderCameraObject& operator=(BlenderCameraObject&&) = default;

private:
	void Initialize() override;
	void Update() override;
};