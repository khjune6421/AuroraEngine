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
	// 씬이 켜질 때 할 일 (오브젝트 배치 등)
	virtual void InitializeScene() override;

	GameObjectBase* CreateCameraObject() override;

	// 씬이 꺼질 때 할 일 (뒷정리)
	virtual void FinalizeScene() override {};
};
/// HyojeTestBRDF.h의 끝
