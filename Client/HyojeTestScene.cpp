///bof  HyojeTestScene.cpp
#include "stdafx.h"
#include "HyojeTestScene.h"

#include "InputManager.h"
#include "TimeManager.h"
#include "SoundManager.h"

#include "TestCameraObject.h"
#include "CamRotObject.h"

#include "Enemy.h"
#include "RNG.h"

#include "Shared/Config/Option.h"

#include "Button.h"
#include "Panel.h"
#include "Slider.h"
#include "Text.h"

using namespace std;
using namespace DirectX;

REGISTER_TYPE(HyojeTestScene)

namespace
{
	void ForceShowCursor(bool show)
	{
		if (show) {
			while (ShowCursor(TRUE) < 0) {}
		} else {
			while (ShowCursor(FALSE) >= 0) {}
		}
	}
}

void HyojeTestScene::Initialize()
{
	if (titlePanel) titlePanel->SetActive(false);
	if (resultPanel) resultPanel->SetActive(false);

	ChangeState(EHyojeState::Title);
}


void HyojeTestScene::Update()
{
    OnHyojeStateUpdate();
}

void HyojeTestScene::Finalize() 
{
}



void HyojeTestScene::ChangeState(EHyojeState newState)
{
    OnHyojeStateExit(m_currentState);
    m_currentState = newState;
    OnHyojeStateEnter(m_currentState);
}

void HyojeTestScene::OnPlayerHit(int damage)
{
	if (m_currentState != EHyojeState::Main)
		return;
	if (m_hitCooldownTimer > 0.0f)
		return;

	m_player_hp -= damage;
	m_hitCooldownTimer = kHitCooldownSec;
	if (m_player_hp <= 0)
	{
		m_player_hp = 0;
		ChangeState(EHyojeState::Result);
	}
}


void HyojeTestScene::OnHyojeStateEnter(EHyojeState type)
{
	switch (type) {
	case EHyojeState::Title:
		LOG("[HyojeTestScene] Title State Enter");
		//ShowCursor(TRUE);
		ForceShowCursor(TRUE);
		if (titlePanel) titlePanel->SetActive(true);

		break;

	case EHyojeState::Main:
		m_player_hp = kPlayerHP;
		m_survivalTime = 0.0f;
		LOG("[HyojeTestScene] Game Start! HP Reset");
		//ShowCursor(FALSE);
		ForceShowCursor(FALSE);
		break;

	case EHyojeState::Result:
		LOG("[HyojeTestScene] Game Over / Result Show");
		//ShowCursor(TRUE);
		ForceShowCursor(TRUE);
		if (resultPanel) resultPanel->SetActive(true);
		if (resultTime)
		{
			std::ostringstream oss;
			oss.setf(std::ios::fixed);
			oss << std::setprecision(2) << "Survival Time: " << m_survivalTime << "s";
			resultTime->SetText(oss.str());
		}
		break;
	}
}

void HyojeTestScene::OnHyojeStateUpdate()
{
	switch (m_currentState) {
	case EHyojeState::Title:
		if (InputManager::GetInstance().GetKeyDown(KeyCode::Space)) {
			ChangeState(EHyojeState::Main); 
		}
		break;

	case EHyojeState::Main:
	{
		const float dt = TimeManager::GetInstance().GetDeltaTime();
		if (m_hitCooldownTimer > 0.0f)
			m_hitCooldownTimer -= dt;
		m_survivalTime += dt;

		static float time = 0.0f;
		time += dt;

		if (time > 1.0f) {
			time = 0.0f;
			constexpr float SPREAD = 10.0f;
			CreatePrefabRootGameObject("Enemy.json")->SetPosition(
				XMVectorSet(RNG::GetInstance().Range(-SPREAD, SPREAD), 0.0f, RNG::GetInstance().Range(-SPREAD, SPREAD), 1.0f)
			);
		}

		if (InputManager::GetInstance().GetKeyDown(KeyCode::Num0)) {
			ChangeState(EHyojeState::Result);
		}
		break;
	}

	case EHyojeState::Result:
		if (InputManager::GetInstance().GetKeyDown(KeyCode::R)) {
			ChangeState(EHyojeState::Title);
		}
		break;
	}
}

void HyojeTestScene::OnHyojeStateExit(EHyojeState type)
{
	switch (type) {
	case EHyojeState::Title:
		// [Title 정리]
		LOG("[HyojeTestScene] OnHyojeStateExit(Title)");
		if(titlePanel) titlePanel->SetActive(false);

		break;

	case EHyojeState::Main:
		// [Main 정리]
		LOG("[HyojeTestScene] OnHyojeStateExit(Main)");
		SoundManager::GetInstance().Stop_ChannelGroup();
		break;

	case EHyojeState::Result:
		// [Result 정리]
		LOG("[HyojeTestScene] OnHyojeStateExit(Result)");
		if (resultPanel) resultPanel->SetActive(false);

		break;
	}
}




void  HyojeTestScene::BindUIActions()
{
    //Panel* optionPanel = nullptr;
    //Panel* titlePanel = nullptr;
    //Panel* resultPanel = nullptr;

    for (const auto& uiPtr : m_UIList) {
        if (auto* panel = dynamic_cast<Panel*>(uiPtr.get())) {
			if (panel->GetName() == "option") optionPanel = panel;
			else if (panel->GetName() == "title") titlePanel = panel;
			else if (panel->GetName() == "result") resultPanel = panel;
        }
		else if (auto* text = dynamic_cast<Text*>(uiPtr.get())) {
			if (text->GetName() == "result_time") resultTime = text;
		}
    }


    for (auto& uiPtr : m_UIList) {
        // -------------------------------------------------------
        // 1. Button bindings
        // -------------------------------------------------------
        if (auto* btn = dynamic_cast<Button*>(uiPtr.get())) {
            std::string key = btn->GetActionKey();

            if (key == "start_game") {
				btn->SetOnClick([this]() { ChangeState(EHyojeState::Main); });
            } else if (key == "quit_game") {
                btn->SetOnClick([]() { PostQuitMessage(0); });
            } else if (key == "open_option") {
                if (optionPanel) btn->SetOnClick([this]() { optionPanel->SetActive(true); });
            } else if (key == "close_option") {
                if (optionPanel) btn->SetOnClick([this]() { optionPanel->SetActive(false); });
            }
        }

        // -------------------------------------------------------
        // 2. Slider bindings
        // -------------------------------------------------------
        else if (auto* slider = dynamic_cast<Slider*>(uiPtr.get())) {
            std::string key = slider->GetActionKey();

            if (key == "MasterVolume") {
                slider->AddListener([](float val) {
                    SoundManager::GetInstance().SetVolume_Main(val);
                });
            } else if (key == "Gamma") {
                slider->AddListener([](float val) {
                    SceneBase::SetGammaIntensity(val);
                });
            }
        }
    }
}
