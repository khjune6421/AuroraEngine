// TestScene.h의 시작
#pragma once
#include "SceneBase.h"

class EndingScene : public SceneBase
{
	class Panel* n1 = nullptr;
	class Panel* n10 = nullptr;
	class Panel* n100 = nullptr;
	class Panel* n1000 = nullptr;
	class Panel* n10000 = nullptr;
	class Panel* n100000 = nullptr;
	class Panel* namu_pan = nullptr;
	class Panel* Grade = nullptr;
	class Panel* pass = nullptr;
	class Panel* fail = nullptr;

public:
	EndingScene() = default;
	~EndingScene() override = default;
	EndingScene(const EndingScene&) = default;
	EndingScene& operator=(const EndingScene&) = default;
	EndingScene(EndingScene&&) = default;
	EndingScene& operator=(EndingScene&&) = default;

	void SetScoreUI(int score);
	void SetGradeUI(int score);
	void SetSucessUI(bool isSuccess);

private:
	void Initialize() override;
	void Update() override;
	void BindUIActions() override;
};