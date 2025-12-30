///main.cpp의 시작
#include "stdafx.h"

#include "WindowManager.h"
#include "SceneManager.h"

#include "TestScene.h"
#include "HyojeTestBRDF.h"

int main()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;

	WindowManager windowManager;
	windowManager.Initialize(L"Window");

	SceneManager& sceneManager = SceneManager::GetInstance();
	//sceneManager.ChangeScene<TestScene>(); // 사실상 Initialize()
	sceneManager.ChangeScene<HyojeTestBRDF>();

	while (windowManager.ProcessMessages()) sceneManager.Run();

	windowManager.Finalize();

	ImGui::DestroyContext();
}
///main.cpp의 끝