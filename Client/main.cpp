#include "stdafx.h"

#include "WindowManager.h"
#include "SceneManager.h"
#include "NavigationManager.h"
#include "RNG.h"
#include "SoundManager.h"
#include "GameManager.h"

#include "TestScene.h"
#include "TitleScene.h"
#include "EndingScene.h"
//#include "HyojeTestScene.h"
//#include "TaehyeonTestScene.h"

using namespace std;

#ifdef _DEBUG
int main(){
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
#else

#ifdef GAMERELEASE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#else
int main() {
#endif

#endif

	WindowManager& windowManager = WindowManager::GetInstance();
	windowManager.Initialize(L"DeadOnBeat");

	NavigationManager::GetInstance().Initialize();

	SceneManager& sceneManager = SceneManager::GetInstance();
	sceneManager.Initialize();
	sceneManager.ChangeScene("TitleScene");

	RNG::GetInstance().Initialize();

	SoundManager& soundManager = SoundManager::GetInstance();
	soundManager.Initialize();

	GameManager& gameManager = GameManager::GetInstance();
	gameManager.Initialize();

	while (windowManager.ProcessMessages())
	{
		gameManager.Update();
		soundManager.Update();
		sceneManager.Run();
	}

	windowManager.Finalize();

	sceneManager.Finalize();

	#ifdef _DEBUG
	ImGui::DestroyContext();
	#endif
}
