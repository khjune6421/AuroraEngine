#include "stdafx.h"

#include "WindowManager.h"
#include "SceneManager.h"

#include "TestScene.h"

int main()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;

	WindowManager windowManager;
	windowManager.Initialize(L"Window");

	SceneManager& sceneManager = SceneManager::GetInstance();
	sceneManager.ChangeScene<TestScene>(); // »ç½Ç»ó Initialize()

	while (windowManager.ProcessMessages()) sceneManager.Run();

	windowManager.Finalize();

	ImGui::DestroyContext();
}