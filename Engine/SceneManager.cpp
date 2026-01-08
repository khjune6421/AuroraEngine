#include "stdafx.h"
#include "SceneManager.h"
#include "InputManager.h"

#include "Renderer.h"
#include "TimeManager.h"
#include "NetManager.h"
#include "NetworkWorld.h"

using namespace std;

void SceneManager::Initialize()
{
	TimeManager::GetInstance().Initialize();
	NetManager::GetInstance().Initialize();
	NetworkWorld::Initialize();
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

		NetworkWorld::SetScene(dynamic_cast<SceneBase*>(m_currentScene.get()));
	}

	TimeManager::GetInstance().UpdateTime();
	NetManager::GetInstance().Update();

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