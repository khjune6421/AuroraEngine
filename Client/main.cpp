#include "stdafx.h"

#include "WindowManager.h"
#include "SceneManager.h"

#include "TestScene.h"
#include "HyojeTestScene.h"



#ifdef _DEBUG
int main()
{
	cout << "==================================" << endl;
	cout << "Welcome to Aurora Engine" << endl;
	cout << "==================================" << endl;
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	WindowManager& windowManager = WindowManager::GetInstance();
	windowManager.Initialize(L"Aurora");

	SceneManager& sceneManager = SceneManager::GetInstance();
	sceneManager.ChangeScene("HyojeTestScene");

	while (windowManager.ProcessMessages()) sceneManager.Run();

	windowManager.Finalize();

	ImGui::DestroyContext();
}