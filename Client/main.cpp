#include "stdafx.h"

#include "WindowManager.h"
#include "Renderer.h"
#include "SceneManager.h"

#include "TestScene.h"

int main()
{
	WindowManager windowManager;
	windowManager.Initialize(L"Window");

	Renderer::GetInstance().Initialize(windowManager.GetHWnd());

	SceneManager& sceneManager = SceneManager::GetInstance();
	sceneManager.ChangeScene<TestScene>();

	while (windowManager.ProcessMessages()) sceneManager.Run();
}