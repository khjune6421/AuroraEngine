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
	InputManager& inputManager = InputManager::GetInstance();
	inputManager.Update();

	if (m_nextScene)
	{
		if (m_currentScene) m_currentScene->BaseFinalize();
		m_currentScene = move(m_nextScene);
		m_currentScene->BaseInitialize();
	}

	TimeManager::GetInstance().UpdateTime();

	m_currentScene->BaseUpdate();


	Renderer& m_renderer = Renderer::GetInstance();
	m_renderer.BeginFrame();

	m_currentScene->BaseRender();

	#ifdef _DEBUG
	m_currentScene->BaseRenderImGui();
	#endif

	m_renderer.EndFrame();
	inputManager.EndFrame();
}
///SceneManager.cppÀÇ ³¡