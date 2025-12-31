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

	// Ctrl + S 입력 시 씬 저장
	#ifdef _DEBUG
	if (inputManager.GetKeyDown(KeyCode::S))
	{
		cout << "저장 중..." << endl;

		filesystem::path sceneFilePath = "../Asset/Scene/" + m_currentScene->GetTypeName() + ".json";
		ofstream file(sceneFilePath);
		file << m_currentScene->BaseSerialize().dump(4);
		file.close();

		cout << "저장 완료!" << endl;
	}
	#endif

	inputManager.EndFrame();
}
///SceneManager.cpp의 끝