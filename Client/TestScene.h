//BOF TestScene.h
#pragma once
#include "SceneBase.h"
#include <array>

class TestScene : public SceneBase
{
	class Player* m_player = nullptr;

	class GameObjectBase* m_tutorialBox = nullptr;
	class GameObjectBase* m_stage2Trigger = nullptr;
	class GameObjectBase* m_stageBossTrigger = nullptr;

	float m_spawnInterval = 3.0f;
	std::vector<DirectX::XMVECTOR> m_spawnPoints = {};

	class Panel* optionPanel = nullptr;

	class Panel* m_tutorialDark = nullptr;
	class Panel* m_tutorialPopup = nullptr;

	class Panel* IngameUI = nullptr;
	class Panel* n1 = nullptr;
	class Panel* n10 = nullptr;
	class Panel* n100 = nullptr;
	class Panel* n1000 = nullptr;
	class Panel* n10000 = nullptr;
	class Panel* n100000 = nullptr;
	class Panel* combo = nullptr;
	class Panel* hpBar = nullptr;
	class Panel* hpDeco = nullptr;
	class Panel* deadEye = nullptr;
	class Panel* bullet = nullptr;

public:
	TestScene() = default;
	~TestScene() override = default;
	TestScene(const TestScene&) = default;
	TestScene& operator=(const TestScene&) = default;
	TestScene(TestScene&&) = default;
	TestScene& operator=(TestScene&&) = default;

private:
	void Initialize() override;
	void Update() override;
	void Render() override;
	void BindUIActions() override;

	#ifdef _DEBUG
	void RenderImGui() override;
	#endif
	void Finalize() override;

	nlohmann::json Serialize() override;
	void Deserialize(const nlohmann::json& jsonData) override;

	void TutorialStep();
	void CheckStageTrigger();

	void SpawnEnemy(float deltaTime);

	void RenderSpawnPoints();
	void SetScoreUI(int score);
	void UpdateHPUI(float hpRatio);
	void UpdateDeadEyeUI(float ratio);
	void UpdateBulletUI(int bulletCount);
};
