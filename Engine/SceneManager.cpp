#include "stdafx.h"
#include "SceneManager.h"
#include "InputManager.h"

#include "Renderer.h"
#include "TimeManager.h"

using namespace std;

void SceneManager::Initialize()
{
	TimeManager::GetInstance().Initialize();
}

void SceneManager::Run()
{
	InputManager::GetInstance().Update();

	if (m_nextScene)
	{
		if (m_currentScene) m_currentScene->BaseFinalize();
		m_currentScene = move(m_nextScene);
		m_currentScene->BaseInitialize();
	}

	#ifdef NDEBUG
	TimeManager::GetInstance().UpdateTime();
	#endif

	m_currentScene->BaseUpdate();

	Renderer& m_renderer = Renderer::GetInstance();
	m_renderer.BeginFrame();

	m_currentScene->BaseRender();

	#ifdef _DEBUG
	m_currentScene->BaseRenderImGui();
	#endif

	m_renderer.EndFrame();

	InputManager::GetInstance().EndFrame();
}
///SceneManager.cppÀÇ ³¡