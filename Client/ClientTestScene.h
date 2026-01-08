#pragma once
#include "SceneBase.h"


class ClientTestScene : public SceneBase
{
public:
	ClientTestScene() = default;
	~ClientTestScene() override = default;
	ClientTestScene(const ClientTestScene&) = default;
	ClientTestScene& operator=(const ClientTestScene&) = default;
	ClientTestScene(ClientTestScene&&) = default;
	ClientTestScene& operator=(ClientTestScene&&) = default;

protected:
	virtual void Initialize() override;
	GameObjectBase* CreateCameraObject() override;
};