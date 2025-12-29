#pragma once
#include "SceneBase.h"

class SceneManager : public SingletonBase<SceneManager>
{
	friend class SingletonBase<SceneManager>;

	std::unique_ptr<class IBase> m_currentScene = nullptr;
	std::unique_ptr<class IBase> m_nextScene = nullptr;

public:
	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;
	SceneManager(SceneManager&&) = delete;
	SceneManager& operator=(SceneManager&&) = delete;

	template<typename T, typename... Args> requires std::derived_from<T, SceneBase>
	void ChangeScene(Args&&... args) { m_nextScene = std::make_unique<T>(std::forward<Args>(args)...); }

	void Run();

private:
	float GetDeltaTime();
};