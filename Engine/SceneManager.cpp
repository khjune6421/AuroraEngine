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

	#ifdef _DEBUG
	// Ctrl + S 입력 시 씬 저장
	if (inputManager.GetKey(KeyCode::Control) && inputManager.GetKeyDown(KeyCode::S)) SaveCurrentScene();
	#endif

	Renderer& m_renderer = Renderer::GetInstance();
	m_renderer.BeginFrame();

	m_currentScene->BaseRender();

	#ifdef _DEBUG
	m_currentScene->BaseRenderImGui();
	#endif

	m_renderer.EndFrame();
	inputManager.EndFrame();
}

void SceneManager::SaveCurrentScene()
{
	cout << "저장 중..." << endl;

	filesystem::path sceneFilePath = "../Asset/Scene/" + m_currentScene->GetType() + ".json";
	ofstream file(sceneFilePath);
	file << m_currentScene->BaseSerialize().dump(4);
	file.close();

	cout << "저장 완료!" << endl;
}
///SceneManager.cpp의 끝