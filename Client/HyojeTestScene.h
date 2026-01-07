/// HyojeTestScene.h의 시작
#pragma once
#include "SceneBase.h"


class HyojeTestScene : public SceneBase
{
public:
	HyojeTestScene() = default;
	~HyojeTestScene() override = default;
	HyojeTestScene(const HyojeTestScene&) = default;
	HyojeTestScene& operator=(const HyojeTestScene&) = default;
	HyojeTestScene(HyojeTestScene&&) = default;
	HyojeTestScene& operator=(HyojeTestScene&&) = default;

protected:
	virtual void Initialize() override;
	GameObjectBase* CreateCameraObject() override;
};
/// HyojeTestScene.h의 끝
