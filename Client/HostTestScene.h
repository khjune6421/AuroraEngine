#pragma once
#include "SceneBase.h"


class HostTestScene : public SceneBase
{
public:
	HostTestScene() = default;
	~HostTestScene() override = default;
	HostTestScene(const HostTestScene&) = default;
	HostTestScene& operator=(const HostTestScene&) = default;
	HostTestScene(HostTestScene&&) = default;
	HostTestScene& operator=(HostTestScene&&) = default;

protected:
	virtual void Initialize() override;
	GameObjectBase* CreateCameraObject() override;
};