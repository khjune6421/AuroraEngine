/// HyojeTestBRDF.h의 시작
#pragma once
#include "SceneBase.h"


class HyojeTestBRDF : public SceneBase
{
public:
	HyojeTestBRDF() = default;
	~HyojeTestBRDF() override = default;
	HyojeTestBRDF(const HyojeTestBRDF&) = default;
	HyojeTestBRDF& operator=(const HyojeTestBRDF&) = default;
	HyojeTestBRDF(HyojeTestBRDF&&) = default;
	HyojeTestBRDF& operator=(HyojeTestBRDF&&) = default;

protected:
	virtual void Initialize() override;
	GameObjectBase* CreateCameraObject() override;
};
/// HyojeTestBRDF.h의 끝
