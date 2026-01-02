#pragma once
#include "SceneBase.h"

class SceneManager : public Singleton<SceneManager>
{
	friend class Singleton<SceneManager>;

	std::unique_ptr<class Base> m_currentScene = nullptr;
	std::unique_ptr<class Base> m_nextScene = nullptr;

public:
	~SceneManager() = default;
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;
	SceneManager(SceneManager&&) = delete;
	SceneManager& operator=(SceneManager&&) = delete;

	void Initialize();
	void Run();
	void Finalize() { if (m_currentScene) m_currentScene->BaseFinalize(); }

	void ChangeScene(std::string sceneTypeName) { m_nextScene = TypeRegistry::GetInstance().Create(sceneTypeName); }

	void SaveCurrentScene();

private:
	SceneManager() = default;
};
