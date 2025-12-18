#include "stdafx.h"
#include "SceneManager.h"

#include "SceneBase.h"

using namespace std;

void SceneManager::Run()
{
	if (m_nextScene)
	{
		if (m_currentScene) m_currentScene->Finalize();
		m_currentScene = move(m_nextScene);
		m_currentScene->Initialize();
	}

	m_currentScene->Update(GetDeltaTime());
	m_currentScene->RemovePendingGameObjects();
	m_currentScene->TransformGameObjects();
	m_currentScene->Render();
}

float SceneManager::GetDeltaTime()
{
	static DWORD previousTime = timeGetTime();
	const DWORD currentTime = timeGetTime();
	const float deltaTime = static_cast<float>(currentTime - previousTime) / 1000.0f;
	previousTime = currentTime;

	return deltaTime;
}