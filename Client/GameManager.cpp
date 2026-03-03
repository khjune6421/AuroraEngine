#include "stdafx.h"

#include "GameManager.h"
#include "TimeManager.h"
#include "SceneManager.h"
#include "SceneBase.h"
#include "Renderer.h"
#include "Panel.h"
#include "InputManager.h"
#include "SoundManager.h"
#include "UIBase.h"

#include "Shared/Config/Option.h"

using namespace std;
using namespace DirectX;


namespace
{
	const char* kRankingsFile = "rankings.json";

	std::string GetCurrentDateTime()
	{
		const auto now = std::chrono::system_clock::now();
		const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
		std::tm localTime = {};
		localtime_s(&localTime, &nowTime);

		std::ostringstream oss;
		oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
		return oss.str();
	}

	struct CheatTransform
	{
		float px;
		float py;
		float pz;
		float rx;
		float ry;
		float rz;
	};

	bool TryGetCheatTransform(EMainState state, CheatTransform& out)
	{
		switch (state)
		{
		case EMainState::Tutorial:
			out = { -176.27f, 0.0f, -184.939f, 0.0f, 0.0f, 0.0f };
			return true;
		case EMainState::Stage1:
			out = { -176.27f, 0.0f, -84.939f, 0.0f, 90.0f, 0.0f };
			return true;
		case EMainState::Stage2:
			out = { -11.234f, 0.0f, 54.962f, 0.0f, 90.0f, 0.0f };
			return true;
		case EMainState::StageBoss:
			out = { 194.865f, 0.0f, 108.356f, 0.0f, 0.0f, 0.0f };
			return true;
		default:
			return false;
		}
	}
}


void GameManager::Initialize()
{
	LoadRankings();
}

void GameManager::Finalize()
{
	//
}

void GameManager::Update()
{
	UpdateLutCrossfade(TimeManager::GetInstance().GetDeltaTime());

	if (m_CurrentScene == EScene::Main)
	{
		if (!m_tutorialPopupOpen)
		{
			if (InputManager::GetInstance().GetKeyDown(KeyCode::Escape))
			{
				ToggleOption();
			}
		}

		SceneManager::GetInstance().SetPaused(m_Pause);
		if (m_Pause)
		{
			SoundManager::GetInstance().Pause();
		}
		else
		{
			SoundManager::GetInstance().Resume();
		}
	}
	else
	{
		if (m_Pause) m_Pause = false;
		SceneManager::GetInstance().SetPaused(false);
		SoundManager::GetInstance().Resume();
	}

}

void GameManager::ToggleOption()
{
	if (!m_optionPanel)
	{
		m_Pause = !m_Pause;
		return;
	}

	if (m_tutorialPopupOpen)
		return;

	const bool opening = !m_Pause;
	m_Pause = opening;
	m_optionPanel->SetActive(opening);
	if (!opening && m_cheatPanel)
	{
		m_cheatPanel->SetActive(false);
	}
	ForceShowCursor(opening);
}

void GameManager::ToggleCheatPanel()
{
	if (!m_cheatPanel)
	{
		return;
	}

	m_cheatPanel->SetActive(!m_cheatPanel->GetActive());
}


void GameManager::OnSceneEnter(EScene type)
{
	auto& sm = SoundManager::GetInstance();

	m_CurrentScene = type;
	m_Pause = false;
	if (type != EScene::Main) m_optionPanel = nullptr;
	if (type != EScene::Main) m_cheatPanel = nullptr;
	if (type == EScene::Title || type == EScene::Main)
	{
		m_isSuccess = false;
		m_isResultCommitted = false;
	}

	switch (type)
	{
	case EScene::Title:
		sm.Ambience_Shot(Config::Ambience);
		sm.Sub_BGM_Shot(Config::Title_BGM, 0.0f);

		ScoreReset();
		m_TutorialStep = ETutorialStep::WASD;
		break;

	case EScene::Main:
		m_Player = GetPlayerPtr();
		if (m_QueuedMainState == EMainState::None)
		{
			m_QueuedMainState = EMainState::Tutorial;
		}
		if (m_QueuedMainState == EMainState::Tutorial)
		{
			m_TutorialStep = ETutorialStep::WASD;
		}
		ChangeMainState(m_QueuedMainState);
		if (m_isCheat && m_Player)
		{
			CheatTransform transform = {};
			if (TryGetCheatTransform(m_MainState, transform))
			{
				m_Player->SetPosition(XMVectorSet(transform.px, transform.py, transform.pz, 1.0f));
				m_Player->SetRotation(XMVectorSet(transform.rx, transform.ry, transform.rz, 0.0f));
				m_currentScore /= 2;
			}
		}
		m_QueuedMainState = EMainState::None;
		break;

	case EScene::Result:
		if (m_currentScore > 0)
		{
			AddScore(GetCurrentDateTime(), m_currentScore);
		}
		break;
	}
}

void GameManager::OnSceneUpdate()
{
	switch (m_CurrentScene)
	{
	case EScene::Title:
		break;

	case EScene::Main:
		MainSceneControl();
		ScoreUpdate();
		break;

	case EScene::Result:
		break;
	}
}

void GameManager::OnSceneRender()
{
	switch (m_CurrentScene)
	{
	case EScene::Title:
		break;

	case EScene::Main:
		Renderer::GetInstance().UI_RENDER_FUNCTIONS().emplace_back(RenderInfo());
		break;

	case EScene::Result:
		break;
	}
}

void GameManager::OnSceneExit()
{
	auto& sm = SoundManager::GetInstance();
	switch (m_CurrentScene)
	{
	case EScene::Title:
		sm.FadeOut(sm.GetBGMCh2(), 1.0f, true);
		break;

	case EScene::Main:
		sm.FadeOut(sm.GetBGMCh1(), 1.0f, true);
		break;

	case EScene::Result:
		sm.FadeOut(sm.GetBGMCh2(), 1.0f, true);
		break;
	}
}

void GameManager::MainSceneControl()
{
	switch (m_MainState)
	{
	case EMainState::Tutorial:
		TutorialControl();
		break;

	case EMainState::Stage1:
		Stage1Control();
		break;

	case EMainState::Stage2:
		Stage2Control();
		break;

	case EMainState::StageBoss:
		Stage3Control();
		break;
	}
}

void GameManager::ChangeMainState(EMainState next)
{
	if (m_MainState == next)
		return;

	LOG("Cur Scene : " << GameManager::ToString(m_MainState) << " ->  Next Scene : " << GameManager::ToString(next));
	if (m_MainState != EMainState::None)
		OnStageExit(m_MainState);

	m_PrevMainState = m_MainState;
	m_MainState = next;

	OnStageEnter(m_MainState);
}

void GameManager::OnStageEnter(EMainState state)
{
	auto& sm = SoundManager::GetInstance();

	switch (state)
	{
	case EMainState::Tutorial:
		LOG("Tutorial Enter\n");
		sm.Main_BGM_Shot(Config::Tutori_BGM, 3.0f);
		StartLutCrossfade(0);

		break;

	case EMainState::Stage1:
		LOG("Stage1 Enter\n");
		sm.Main_BGM_Shot(Config::Stage1_BGM, 3.0f);
		StartLutCrossfade(4);

		if (!m_Player)
			m_Player = GetPlayerPtr();

		if (m_Player)
		{
			m_Player->SetAction(Action::All, true);
		}
		if (m_tutorialPopupOpen)
			CloseTutorialPopup();

		TimeManager::GetInstance().SetTimeScale(1.0f);
		Player::SetCameraSensitivity(30.0f);
		ForceShowCursor(false);

		break;

	case EMainState::Stage2:
		LOG("Stage2 Enter\n");
		sm.Main_BGM_Shot(Config::Stage2_BGM, 3.0f);
		StartLutCrossfade(0);

		break;

	case EMainState::StageBoss:
		LOG("StageBoss Enter\n");
		sm.Main_BGM_Shot(Config::Stage3_BGM, 3.0f);
		StartLutCrossfade(4);

		break;
	}
}

void GameManager::StartLutCrossfade(int targetIndex)
{
	Renderer& renderer = Renderer::GetInstance();
	const int currentIndex = renderer.GetSelectedLUTIndex();
	if (currentIndex == targetIndex)
	{
		SceneBase::SetPostProcessingFlag(PostProcessingBuffer::PostProcessingFlag::LUT_CROSSFADE, false);
		SceneBase::SetLutLerpFactor(0.0f);
		m_lutCrossfadeActive = false;
		return;
	}

	m_lutTargetIndex = targetIndex;
	renderer.GetSelectedLUT2Index() = targetIndex;
	m_lutCrossfadeElapsed = 0.0f;
	m_lutCrossfadeActive = true;

	SceneBase::SetPostProcessingFlag(PostProcessingBuffer::PostProcessingFlag::LUT_CROSSFADE, true);
	SceneBase::SetLutLerpFactor(0.0f);
}

void GameManager::UpdateLutCrossfade(float dt)
{
	if (!m_lutCrossfadeActive) return;

	m_lutCrossfadeElapsed += dt;
	float t = m_lutCrossfadeElapsed / m_lutCrossfadeDuration;
	if (t > 1.0f) t = 1.0f;

	const float smooth = t * t * (3.0f - 2.0f * t);
	SceneBase::SetLutLerpFactor(smooth);

	if (t >= 1.0f)
	{
		Renderer& renderer = Renderer::GetInstance();
		renderer.GetSelectedLUTIndex() = m_lutTargetIndex;
		SceneBase::SetPostProcessingFlag(PostProcessingBuffer::PostProcessingFlag::LUT_CROSSFADE, false);
		SceneBase::SetLutLerpFactor(0.0f);
		m_lutCrossfadeActive = false;
	}
}

void GameManager::TutorialControl()
{
	if (m_MainState == EMainState::Tutorial && InputManager::GetInstance().GetKeyDown(KeyCode::F7))
	{
		if (!m_Player)
			m_Player = GetPlayerPtr();
		ChangeMainState(EMainState::Stage1);
		return;
	}

	if (!m_Player)
		return;

	auto& p = m_Player;
	if (m_isCheat)
	{
		p->SetAction(Action::All, true);
		return;
	}

	p->SetAction(Action::All, false);

	switch (m_TutorialStep)
	{
	case ETutorialStep::WASD:
		p->SetAction(Action::Move, true);
		break;

	case ETutorialStep::Dash:
		if (m_tutorialPopupOpen == false)
		{
			p->SetAction(Action::Move, true);
			p->SetAction(Action::Dash, true);
		}
		break;

	case ETutorialStep::Reload:
		if (m_tutorialPopupOpen == false)
		{
			p->SetAction(Action::Reload, true);
		}
		break;

	case ETutorialStep::Shoot:
		if (m_tutorialPopupOpen == false)
		{
			p->SetAction(Action::Shoot, true);
		}
		break;

	case ETutorialStep::AutoReload:
		if (m_tutorialPopupOpen == false)
		{
			p->SetAction(Action::Shoot, true);
			p->SetAction(Action::AutoReload, true);
		}
		break;

	case ETutorialStep::DeadEye:
		p->SetAction(Action::All, false);
		break;

	case ETutorialStep::DeadTwo:
		p->SetAction(Action::DeadEye, true);
		break;

	case ETutorialStep::End:
		p->SetAction(Action::All, true);
		ChangeMainState(EMainState::Stage1);
		break;
	}

	if (m_tutorialPopupOpen)
	{
		if (AnyInputDown())
		{
			CloseTutorialPopup();
		}
		return;
	}

	if (!m_stepPopupShown && m_TutorialStep != ETutorialStep::End)
	{
		ShowTutorialPopup();
		m_stepPopupShown = true;
		return;
	}
}

void GameManager::Stage1Control()
{
}

void GameManager::Stage2Control()
{
}

void GameManager::Stage3Control()
{
}

void GameManager::OnStageExit(EMainState state)
{
	auto& sm = SoundManager::GetInstance();

	switch (state)
	{
	case EMainState::Tutorial:
		LOG("Tutorial Exit\n");
		sm.FadeOut(sm.GetBGMCh1(), 1.0f, true);
		m_Player->RestoreHitPoint();
		break;

	case EMainState::Stage1:
		LOG("Stage1 Exit\n");
		sm.FadeOut(sm.GetBGMCh1(), 1.0f, true);
		m_Player->RestoreHitPoint();
		break;

	case EMainState::Stage2:
		sm.FadeOut(sm.GetBGMCh1(), 1.0f, true);
		m_Player->RestoreHitPoint();
		break;

	case EMainState::StageBoss:
		sm.FadeOut(sm.GetBGMCh1(), 1.0f, true);
		m_Player->RestoreHitPoint();
		break;
	}
}

Player* GameManager::GetPlayerPtr()
{
	Player* temp = dynamic_cast<Player*>(SceneManager::GetInstance().GetCurrentScene()->GetRootGameObject("Player"));
	LOG("GM Player Get" << temp);
	return temp;
}


void GameManager::ScoreUpdate()
{
	float delta = TimeManager::GetInstance().GetDeltaTime();

	if (!m_isCombatStarted)
		return;

	m_decayTimer += delta;
	while (m_decayTimer >= Config::ScoreDecayIntervalSec)
	{
		m_decayTimer -= Config::ScoreDecayIntervalSec;

		if (m_currentScore > 0)
		{
			m_currentScore -= Config::ScoreDecayAmount;
			if (m_currentScore < 0) m_currentScore = 0;
		}
		else
		{
			m_currentScore = 0;
			break;
		}
	}
}

void GameManager::ScoreReset()
{
	m_currentScore = 0;
	m_multiplier = 1;
	m_killCountForNextLevel = 0;
	m_lastKillTime = 0.0f;
	m_isCombatStarted = false;
	m_decayTimer = 0.0f;
}

void GameManager::SetTutorialStep(ETutorialStep step)
{
	if (m_TutorialStep == step)
		return;

	if (m_MainState != EMainState::Tutorial)
		return;

	m_TutorialStep = step;

	m_stepPopupShown = false;
}

void GameManager::UpdateTutorialPopupImage()
{
	if (!m_tutorialPopup)
		return;

	std::string path = "";

	switch (m_TutorialStep)
	{
	case ETutorialStep::WASD:
		path = "1.Tuto_Move.png";
		break;

	case ETutorialStep::Dash:
		path = "2.Tuto_Dash.png";
		break;

	case ETutorialStep::Reload:
		path = "5.Tuto_Reload.png";
		break;

	case ETutorialStep::Shoot:
		path = "3.Tuto_Shoot.png";
		break;

	case ETutorialStep::AutoReload:
		path = "4.Tuto_AutoReload.png";
		break;

	case ETutorialStep::DeadEye:
		path = "6.Tuto_DeadEye1.png";
		break;

	case ETutorialStep::DeadTwo:
		path = "7.Tuto_DeadEye2.png";
		break;

	default:
		path = "";
		break;
	}

	if (!path.empty())
	{
		m_tutorialPopup->SetTextureAndOffset(path);
	}
}

function<void()> GameManager::RenderInfo()
{
	if (m_isCheat)
	{
		return [&]() { Renderer::GetInstance().RenderTextUIPosition((L"Score: " + to_wstring(GameManager::GetInstance().GetScore())).c_str(), XMFLOAT2(0.45f, 0.1f)); };
	}

	if (m_MainState == EMainState::Tutorial)
	{
		return [&]() {
			Renderer::GetInstance().RenderTextUIPosition(
				L"Press F7 to Skip Tutorial",
				XMFLOAT2(.8f, 0.01f)
			);
		};
	}

	return []() {};
}

/*switch (m_TutorialStep)
{
case ETutorialStep::WASD:
	return [&]() { Renderer::GetInstance().RenderTextUIPosition(L"Use WASD to Move Towards the Box", XMFLOAT2(0.35f, 0.1f)); };
	break;

case ETutorialStep::Dash:
	return [&]() { Renderer::GetInstance().RenderTextUIPosition(L"Press WASD And Space to Dash", XMFLOAT2(0.4f, 0.1f)); };
	break;

case ETutorialStep::Reload:
	return [&]() { Renderer::GetInstance().RenderTextUIPosition(L"Press R to Reload", XMFLOAT2(0.4f, 0.1f)); };
	break;

case ETutorialStep::Shoot:
	return [&]() { Renderer::GetInstance().RenderTextUIPosition(L"Left Click to Shoot", XMFLOAT2(0.4f, 0.1f)); };
	break;

case ETutorialStep::AutoReload:
	return [&]() { Renderer::GetInstance().RenderTextUIPosition(L"Auto Reload Is Done When You Use All Bullets", XMFLOAT2(0.25f, 0.1f)); };
	break;

case ETutorialStep::DeadEye:
	return [&]() { Renderer::GetInstance().RenderTextUIPosition(L"Right Click to Activate Dead Eye", XMFLOAT2(0.35f, 0.1f)); };
	break;

case ETutorialStep::End:
	return [&]()
		{
			Renderer::GetInstance().RenderTextUIPosition((L"Score: " + to_wstring(m_currentScore)).c_str(), XMFLOAT2(0.45f, 0.1f));
		};
	break;

default:
	return []() {};
	break;
}*/

void GameManager::TempPrint()
{
	float dt = TimeManager::GetInstance().GetDeltaTime();

	static float scorePrintTimer = 0.0f;
	scorePrintTimer += dt;
	if (scorePrintTimer >= 0.5f) {
		scorePrintTimer = 0.0f;
		LOG(std::endl << "[Score] "
			<< GameManager::GetInstance().GetScore()
			<< " x"
			<< GameManager::GetInstance().GetMultiplier());
	}
}

void GameManager::AddKill()
{
	const float now = TimeManager::GetInstance().GetTotalTime();

	int bonus = 0;
	if (m_isCombatStarted)
	{
		if (now - m_lastKillTime <= Config::ChainKillWindowSec)
			bonus = Config::ChainKillBonus;
	}
	else
	{
		m_isCombatStarted = true;
	}

	m_lastKillTime = now;

	const int gained = (Config::BaseScore + bonus) * m_multiplier;
	m_currentScore += gained;

	if (m_multiplier < Config::MaxMultiplier) {
		++m_killCountForNextLevel;
		if (m_killCountForNextLevel >= Config::KillsPerLevel) {
			if (m_multiplier == 1) m_multiplier = 2;
			else if (m_multiplier == 2) m_multiplier = 4;
			else if (m_multiplier == 4) m_multiplier = 8;
			m_killCountForNextLevel = 0;
		}
	}
}

void GameManager::OnPlayerHit()
{
	if (m_multiplier == 8) m_multiplier = 4;
	else if (m_multiplier == 4) m_multiplier = 2;
	else if (m_multiplier == 2) m_multiplier = 1;

	m_killCountForNextLevel = 0;
}

void GameManager::OnRhythmMiss()
{
	m_killCountForNextLevel = 0;
}


void GameManager::LoadRankings()
{
	m_rankings.clear();

	ifstream file(kRankingsFile);
	if (!file.is_open())
		return;

	nlohmann::json data = {};
	file >> data;
	file.close();

	const nlohmann::json* rankings = nullptr;
	if (data.is_array())
	{
		rankings = &data;
	}
	else if (data.contains("rankings") && data["rankings"].is_array())
	{
		rankings = &data["rankings"];
	}
	if (!rankings)
		return;

	for (const auto& entry : *rankings) {
		if (!entry.is_object())
			continue;

		const auto playedAtIt = entry.find("played_at");
		const auto scoreIt = entry.find("score");
		if (playedAtIt == entry.end() || scoreIt == entry.end())
			continue;
		if (!playedAtIt->is_string() || !scoreIt->is_number_integer())
			continue;

		m_rankings.emplace_back(playedAtIt->get<string>(), scoreIt->get<int>());
	}

	sort(m_rankings.begin(), m_rankings.end(), [](const auto& left, const auto& right) {
		return left.second > right.second;
		});
}


void GameManager::RegisterTutorialUI(Panel* dark, Panel* popup)
{
	m_tutorialPanel = dark;
	m_tutorialPopup = popup;

	LOG(m_tutorialPanel << " " << m_tutorialPopup);
}

void GameManager::ShowTutorialPopup()
{
	m_tutorialPopupOpen = true;

	TimeManager::GetInstance().SetTimeScale(0.0f);
	Player::SetCameraSensitivity(0.0f);

	UpdateTutorialPopupImage();

	if (m_tutorialPanel) m_tutorialPanel->SetActive(true);
	if (m_tutorialPopup) m_tutorialPopup->SetActive(true);

	ForceShowCursor(true);
}

void GameManager::CloseTutorialPopup()
{
	m_tutorialPopupOpen = false;
	m_Pause = false;

	TimeManager::GetInstance().SetTimeScale(1.0f);
	Player::SetCameraSensitivity(30.0f);

	if (m_tutorialPanel) m_tutorialPanel->SetActive(false);
	if (m_tutorialPopup) m_tutorialPopup->SetActive(false);

	ForceShowCursor(false);
}

bool GameManager::AnyInputDown()
{
	auto& in = InputManager::GetInstance();

	return in.GetKeyDown(KeyCode::MouseLeft);
}

void GameManager::SaveRankings() const
{
	nlohmann::json data = nlohmann::json::array();

	for (const auto& [playedAt, score] : m_rankings) {
		data.push_back({ { "played_at", playedAt }, { "score", score } });
	}

	ofstream file(kRankingsFile);
	file << data.dump(4);
	file.close();
}

void GameManager::AddScore(const std::string& playedAt, int score)
{
	if (score <= 0)
		return;

	m_rankings.emplace_back(playedAt, score);

	sort(m_rankings.begin(), m_rankings.end(), [](const auto& left, const auto& right) {
		return left.second > right.second;
		});

	SaveRankings();
}

std::string GameManager::GetGradeTextureName(int score) const
{
	if (m_rankings.empty())
		return "UI_Grade_F.png";
	if (m_rankings.size() == 1)
		return "UI_Grade_S.png";

	size_t index = m_rankings.size() - 1;
	for (size_t i = 0; i < m_rankings.size(); ++i)
	{
		if (m_rankings[i].second == score)
		{
			index = i;
			break;
		}
	}

	const float rank = static_cast<float>(index + 1);
	const float total = static_cast<float>(m_rankings.size());
	const float topPercent = (rank / total) * 100.0f;

	if (topPercent <= 10.0f) return "UI_Grade_S.png";
	if (topPercent <= 20.0f) return "UI_Grade_A.png";
	if (topPercent <= 30.0f) return "UI_Grade_B.png";
	if (topPercent <= 50.0f) return "UI_Grade_C.png";
	if (topPercent <= 70.0f) return "UI_Grade_D.png";
	if (topPercent <= 90.0f) return "UI_Grade_E.png";
	return "UI_Grade_F.png";
}
