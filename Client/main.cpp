#include "stdafx.h"

#include "WindowManager.h"
#include "SceneManager.h"

#include "TestScene.h"

int main()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	WindowManager& windowManager = WindowManager::GetInstance();
	windowManager.Initialize(L"Aurora");

	SceneManager& sceneManager = SceneManager::GetInstance();
	sceneManager.ChangeScene<TestScene>();

	while (windowManager.ProcessMessages()) sceneManager.Run();

	windowManager.Finalize();

	ImGui::DestroyContext();
}