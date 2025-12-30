///SceneManager.cppÀÇ ½ÃÀÛ
#include "stdafx.h"
#include "SceneManager.h"

#include "Renderer.h"
#include "TimeManager.h"

using namespace std;

void SceneManager::Initialize()
{
	TimeManager::GetInstance().Initialize();
}

void SceneManager::Run()
{

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

	// ¾À ·»´õ¸µ
	m_currentScene->BaseRender();

	#ifdef _DEBUG
	// ImGui ·»´õ¸µ
	m_currentScene->BaseRenderImGui();
	#endif

	m_renderer.EndFrame();
}

void SceneManager::Finalize()
{
	if (m_currentScene) m_currentScene->BaseFinalize();
}
