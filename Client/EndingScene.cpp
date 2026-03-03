
#include "stdafx.h"
#include "EndingScene.h"

#include "SceneManager.h"
#include "TimeManager.h"
#include "SoundManager.h"
#include "GameManager.h"
#include "RNG.h"

#include "CameraComponent.h"

#include "UIBase.h"
#include "Panel.h"
#include "Button.h"
#include "Slider.h"


REGISTER_TYPE(EndingScene)

using namespace std;
using namespace DirectX;


void EndingScene::Initialize()
{
	GameManager::GetInstance().ForceShowCursor(TRUE);
	GameManager::GetInstance().OnSceneEnter(EScene::Result);

	GetRootGameObject("MainCam")->GetComponent<class CameraComponent>()->SetAsMainCamera();

	const int finalScore = GameManager::GetInstance().GetScore();
	SetScoreUI(finalScore);
	SetGradeUI(finalScore);

	const bool isSuccess = GameManager::GetInstance().IsSuccess();
	SetSucessUI(isSuccess);

}

void EndingScene::Update()
{
	GameManager::GetInstance().OnSceneUpdate();
}

void EndingScene::BindUIActions()
{
	for (const auto& uiPtr : m_UIList) {
		if (auto* panel = dynamic_cast<Panel*>(uiPtr.get())) {
			if (panel->GetName() == "1") n1 = panel;
			else if (panel->GetName() == "10") n10 = panel;
			else if (panel->GetName() == "100") n100 = panel;
			else if (panel->GetName() == "1000") n1000 = panel;
			else if (panel->GetName() == "10000") n10000 = panel;
			else if (panel->GetName() == "100000") n100000 = panel;
			else if (panel->GetName() == "namu_pan") namu_pan = panel;
			else if (panel->GetName() == "Grade") Grade = panel;
			else if (panel->GetName() == "pass") pass = panel;
			else if (panel->GetName() == "fail") fail = panel;
		}
	}


	for (auto& uiPtr : m_UIList) {
		// -------------------------------------------------------
		// 1. Button bindings
		// -------------------------------------------------------
		if (auto* btn = dynamic_cast<Button*>(uiPtr.get())) {
			std::string key = btn->GetActionKey();

			if (key == "RETRY") {
				btn->SetOnClick([]() {
					SceneManager::GetInstance().ChangeScene("TestScene");
					});
			} else if (key == "TITLE") {
				btn->SetOnClick([]() {
					SceneManager::GetInstance().ChangeScene("TitleScene");
					});
			}
		}
	}
}

void EndingScene::SetScoreUI(int score)
{
	if (score < 0) score = 0;
	if (score > 999999) score = 999999;

	int d1 = score % 10;           // 1의 자리
	int d10 = (score / 10) % 10;    // 10의 자리
	int d100 = (score / 100) % 10;   // 100의 자리
	int d1000 = (score / 1000) % 10;  // 1000의 자리
	int d10000 = (score / 10000) % 10; // 10000의 자리
	int d100000 = (score / 100000) % 10;// 100000의 자리


	if (n1)      n1->SetTextureAndOffset("UI_n" + std::to_string(d1) + ".png");
	if (n10)     n10->SetTextureAndOffset("UI_n" + std::to_string(d10) + ".png");
	if (n100)    n100->SetTextureAndOffset("UI_n" + std::to_string(d100) + ".png");
	if (n1000)   n1000->SetTextureAndOffset("UI_n" + std::to_string(d1000) + ".png");
	if (n10000)  n10000->SetTextureAndOffset("UI_n" + std::to_string(d10000) + ".png");
	if (n100000) n100000->SetTextureAndOffset("UI_n" + std::to_string(d100000) + ".png");
}

void EndingScene::SetGradeUI(int score)
{
	if (Grade == nullptr) return;

	const std::string gradeTextureName = GameManager::GetInstance().GetGradeTextureName(score);
	Grade->SetTextureAndOffset(gradeTextureName);
}

void EndingScene::SetSucessUI(bool isSuccess)
{
	const int variant = RNG::GetInstance().Range(1, 5);
	if (namu_pan) {
		const std::string prefix = isSuccess ? "UI_Result_Sign_Clear" : "UI_Result_Sign_Gameover";
		namu_pan->SetTextureAndOffset(prefix + std::to_string(variant) + ".png");
	}

	if (isSuccess) 		{
		pass->SetActive(true);
		fail->SetActive(false);
	} else {
		pass->SetActive(false);
		fail->SetActive(true);
	}

}
