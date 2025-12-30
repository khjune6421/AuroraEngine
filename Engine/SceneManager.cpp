///SceneManager.cpp의 시작
#include "stdafx.h"
#include "SceneManager.h"
#include "InputManager.h"

using namespace std;

void SceneManager::Run()
{
	InputManager::GetInstance().Update();

	if (m_nextScene)
	{
		if (m_currentScene) m_currentScene->BaseFinalize();
		m_currentScene = move(m_nextScene);
		m_currentScene->BaseInitialize();
	}

	m_currentScene->BaseUpdate(GetDeltaTime());
	m_currentScene->BaseRender();


	InputManager::GetInstance().EndFrame();
}

float SceneManager::GetDeltaTime()
{
	static DWORD previousTime = timeGetTime();
	const DWORD currentTime = timeGetTime();
	const float deltaTime = static_cast<float>(currentTime - previousTime) / 1000.0f;
	previousTime = currentTime;

	return deltaTime;
}
///SceneManager.cpp의 끝