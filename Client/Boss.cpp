#include "stdafx.h"
#include "Boss.h"

#include "ColliderComponent.h"
#include "FSMComponentBoss.h"
#include "SceneManager.h"
#include "SceneBase.h"
#include "Player.h"
#include "TimeManager.h"
#include "RNG.h"
#include "Renderer.h"
#include "ResourceManager.h"

REGISTER_TYPE(Boss)

using namespace std;
using namespace DirectX;

void Boss::Die()
{
	if (m_state == AIState::Dead) return;

	m_state = AIState::Dead;
	m_deathTimer = 0.0f;
	if (m_fsm) m_fsm->ChangeState(FSMComponentBoss::EDead);
}

void Boss::OnAttackFinished()
{
	if (m_state == AIState::Dead) return;

	m_state = AIState::Chase;
	if (m_fsm) m_fsm->ChangeState(FSMComponentBoss::EChase);
}

void Boss::Initialize()
{
	m_fsm = GetComponent<FSMComponentBoss>();

	m_player = static_cast<Player*>(SceneManager::GetInstance().GetCurrentScene()->GetGameObjectRecursive("Player"));
	if (!m_player) LOG("Boss 초기화 오류: Player 게임 오브젝트를 찾을 수 없습니다.");
	GameObjectBase* triggerObj = SceneManager::GetInstance().GetCurrentScene()->GetGameObjectRecursive(m_triggerColliderName);
	if (triggerObj) m_triggerCollider = triggerObj->GetComponent<ColliderComponent>();

	if (m_state == AIState::Chase && m_fsm) { m_fsm->ChangeState(FSMComponentBoss::EChase); }

	ResourceManager& resourceManager = ResourceManager::GetInstance();
	m_bossHealthBarTextureAndOffset = resourceManager.GetTextureAndOffset("UI_Boss_GaugeBar.png");
	m_bossHealthBarDecoTextureAndOffset = resourceManager.GetTextureAndOffset("UI_Boss_GaugeDeco.png");
	m_bossHealthBarBackgroundTextureAndOffset = resourceManager.GetTextureAndOffset("UI_Boss_GaugeBG.png");
}

void Boss::Update()
{
	float deltaTime = TimeManager::GetInstance().GetDeltaTime();

	if (!m_player) return;
	const XMVECTOR& playerPos = m_player->GetPosition();

	if (m_triggerCollider && !m_hasFoundPlayer && m_triggerCollider->CheckCollisionPoint(XMVectorSetY(playerPos, 0.0f))) m_hasFoundPlayer = true;

	switch (m_state)
	{
	case AIState::Idle:
		break;

	case AIState::Chase:
	{
		if (!m_hasFoundPlayer) break;

		XMVECTOR toPlayer = XMVectorSubtract(playerPos, GetPosition());
		float distSq = XMVectorGetX(XMVector3LengthSq(toPlayer));

		if (distSq <= m_attackRangeSquare)
		{
			m_state = AIState::Attack;
			if (m_fsm)
			{
				if (RANDOM(0, 1)) m_fsm->ChangeState(FSMComponentBoss::EAttack);
				else m_fsm->ChangeState(FSMComponentBoss::EJump);
			}
		}
		else
		{
			LookAt(XMVectorSetY(playerPos, 0.0f));
			MoveDirection(m_moveSpeed * deltaTime, Direction::Forward);
			Rotate({ 0.0f, 180.0f , 0.0f });
		}
		break;
	}

	case AIState::Attack:
		LookAt(XMVectorSetY(playerPos, 0.0f));
		Rotate({ 0.0f, 180.0f , 0.0f });
		break;

	case AIState::Dead:
		m_deathTimer += deltaTime;
		if (m_deathTimer >= m_deathDuration) SetAlive(false);
		return;

	default:
		break;
	}
}

void Boss::Render()
{
	if (!m_hasFoundPlayer) return;

	Renderer::GetInstance().UI_RENDER_FUNCTIONS().emplace_back
	(
		[&]()
		{
			float healthRatio = static_cast<float>(m_hitPoints) / static_cast<float>(m_maxHitPoints);

			RECT hitPointRect =
			{
				0,
				0,
				static_cast<LONG>(m_bossHealthBarTextureAndOffset.second.x * healthRatio * 2.0f),
				static_cast<LONG>(m_bossHealthBarTextureAndOffset.second.y * 2.0f)
			};

			Renderer& renderer = Renderer::GetInstance();

			// 배경
			renderer.RenderImageNrmPosition
			(
				m_bossHealthBarBackgroundTextureAndOffset.first,
				{ 0.5f, 0.15f },
				m_bossHealthBarBackgroundTextureAndOffset.second,
				0.75f,
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				0.0f
			);

			// 체력 바
			renderer.RenderImageNrmPosition
			(
				m_bossHealthBarTextureAndOffset.first,
				{ 0.5f, 0.1525f },
				m_bossHealthBarTextureAndOffset.second,
				0.75f,
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				0.25f,
				&hitPointRect
			);

			// 데코
			renderer.RenderImageNrmPosition
			(
				m_bossHealthBarDecoTextureAndOffset.first,
				{ 0.5f, 0.1525f },
				m_bossHealthBarDecoTextureAndOffset.second,
				0.75f,
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				0.5f
			);
		}
	);
}

#ifdef _DEBUG
void Boss::RenderImGui()
{
	array<char, 256> triggerColliderNameBuffer = {};
	strcpy_s(triggerColliderNameBuffer.data(), triggerColliderNameBuffer.size(), m_triggerColliderName.c_str());
	if (ImGui::InputText("Trigger Collider Name", triggerColliderNameBuffer.data(), triggerColliderNameBuffer.size())) m_triggerColliderName = string(triggerColliderNameBuffer.data());
}
#endif

nlohmann::json Boss::Serialize()
{
	nlohmann::json jsonData = {};
	jsonData["triggerColliderName"] = m_triggerColliderName;
	return jsonData;
}

void Boss::Deserialize(const nlohmann::json& jsonData)
{
	if (jsonData.find("triggerColliderName") != jsonData.end()) m_triggerColliderName = jsonData["triggerColliderName"].get<string>();
	else m_triggerColliderName = "StageBossTrigger";
}