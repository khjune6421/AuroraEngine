///BOF Enemy.cpp
#include "stdafx.h"
#include "Enemy.h"

#include "SkinnedModelComponent.h"
#include "ColliderComponent.h"
#include "FSMComponentEnemy.h"
#include "TimeManager.h"
#include "Player.h"
#include "SceneManager.h"
#include "SceneBase.h"
#include "NavigationManager.h"
#include "RNG.h"
#include "GameManager.h"
#include "SoundManager.h"

#include "Shared/Config/Option.h"

REGISTER_TYPE(Enemy)

using namespace std;
using namespace DirectX;

vector<Enemy*> Enemy::s_enemies = {};

void Enemy::Initialize()
{
	m_fsm = GetComponent<FSMComponentEnemy>();

	s_enemies.push_back(this);

	m_player = static_cast<Player*>(SceneManager::GetInstance().GetCurrentScene()->GetGameObjectRecursive("Player"));
	if (!m_player) LOG("Enemy 초기화 오류: Player 게임 오브젝트를 찾을 수 없습니다.");
	GameObjectBase* triggerObj = SceneManager::GetInstance().GetCurrentScene()->GetGameObjectRecursive(m_triggerColliderName);
	if (triggerObj) m_triggerCollider = triggerObj->GetComponent<ColliderComponent>();

	m_pathFindIntervalRandomOffset = RANDOM(0.0f, m_pathFindInterval);
	m_pathFindTimer = -m_pathFindIntervalRandomOffset;

	if (m_fsm)
	{
		switch (m_state)
		{
		case AIState::Idle:   m_fsm->ChangeState(FSMComponentEnemy::EIdle); break;
		case AIState::Chase:  m_fsm->ChangeState(FSMComponentEnemy::EChase); break;
		case AIState::Attack: m_fsm->ChangeState(FSMComponentEnemy::EAttack); break;
		case AIState::Dead:   m_fsm->ChangeState(FSMComponentEnemy::EDead); break;
		}
	}
}

void Enemy::Die()
{
	if (m_state == AIState::Dead) return;

	m_state = AIState::Dead;
	m_deathTimer = 0.0f;

	if (m_fsm) m_fsm->ChangeState(FSMComponentEnemy::EDead);
	
	GameManager::GetInstance().AddKill();

	//SoundManager::GetInstance().SFX_Shot(GetPosition(), Config::Enemy_Die);
}

// 적이 플레이어를 쫓기 시작하는 거리의 제곱
constexpr float CHASE_SQ_RANGE = 25.0f * 25.0f;

void Enemy::Update()
{
	float deltaTime = TimeManager::GetInstance().GetDeltaTime();

	if (!m_player) return;
	const XMVECTOR& playerPos = m_player->GetPosition();
	if (m_triggerCollider && !m_hasFoundPlayer && m_triggerCollider->CheckCollisionPoint(XMVectorSetY(playerPos, 0.0f))) m_hasFoundPlayer = true;

	switch (m_state) {
	case Enemy::AIState::Idle: 
		//1. 튜토리얼 허수아비용 
		if (m_isTutorialDummy) { return; }
		//2. Attack후 잠깐 후딜용으로 쓰기 (이건 애니메이션 받고 테스트해봐야 함)
		break;
	case Enemy::AIState::Chase:
		if(m_player){
			XMVECTOR toPlayer = playerPos - GetPosition();
			float distSq = XMVectorGetX(XMVector3LengthSq(toPlayer));

			if (distSq <= m_attackRangeSquare) {
				m_state = AIState::Attack;
				if (m_fsm) m_fsm->ChangeState(FSMComponentEnemy::EAttack);
				return;
			}

			if (!m_hasFoundPlayer && distSq > CHASE_SQ_RANGE) return;

			m_hasFoundPlayer = true;
			m_pathFindTimer += deltaTime;
			if (m_pathFindTimer >= m_pathFindInterval - m_pathFindIntervalRandomOffset) { m_path.clear(); m_pathFindTimer = -m_pathFindIntervalRandomOffset; }
			MoveAlongPath(deltaTime);
		}
		break;
	case Enemy::AIState::Attack: // 여기서 업데이트 안하고 FSMComponent에서 애니메이션을 진행함
		break;
	case Enemy::AIState::Dead:
		m_deathTimer += deltaTime;
		if (m_deathTimer >= m_deathDuration) SetAlive(false);
		return;
	default:
		break;
	}

	ApplySeparation(deltaTime);
}

#ifdef _DEBUG
void Enemy::RenderImGui()
{
	ImGui::Checkbox("Is Tutorial Dummy", &m_isTutorialDummy);
	
	array<char, 256> triggerColliderNameBuffer = {};
	strcpy_s(triggerColliderNameBuffer.data(), triggerColliderNameBuffer.size(), m_triggerColliderName.c_str());
	if (ImGui::InputText("Trigger Collider Name", triggerColliderNameBuffer.data(), triggerColliderNameBuffer.size())) m_triggerColliderName = string(triggerColliderNameBuffer.data());
}
#endif

void Enemy::SetAsTutorialDummy()
{
	m_isTutorialDummy = true;
	m_state = AIState::Idle;
	
	if (m_fsm) m_fsm->ChangeState(FSMComponentEnemy::EIdle);
}


void Enemy::MoveAlongPath(float dt)
{
	// 플레이어까지 경로 계산
	if (m_path.empty())
	{
		m_path = NavigationManager::GetInstance().FindPath(GetPosition(), m_player->GetPosition());
		for (auto& point : m_path) point = XMVectorSetY(point, 0.0f);
	}

	// 경로 따라 이동
	XMVECTOR toTarget = XMVectorSubtract(m_path.front(), GetPosition());
	float distanceSquared = XMVectorGetX(XMVector3LengthSq(toTarget));
	if (distanceSquared < 0.1f * 0.1f)
	{
		m_path.pop_front();
	}
	else
	{
		m_fsm->ChangeState(FSMComponentEnemy::EChase);

		XMVECTOR direction = XMVector3Normalize(toTarget);

		//회전 - 모델이 돌아가 있을 것이라고 추정함
		float targetYawRad = atan2f(XMVectorGetX(direction), XMVectorGetZ(direction));
		targetYawRad += XM_PI;
		float targetYawDeg = XMConvertToDegrees(targetYawRad);
		XMFLOAT3 currentEuler = {};
		XMStoreFloat3(&currentEuler, GetRotation());
		float currentYawDeg = currentEuler.y;
		float diff = targetYawDeg - currentYawDeg;
		while (diff > 180.0f)  diff -= 360.0f;
		while (diff < -180.0f) diff += 360.0f;
		float step = m_rotationSpeed * 50.0f * dt; 
		float nextYawDeg = 0.0f;
		if (abs(diff) <= step) {nextYawDeg = targetYawDeg;} 
		else {nextYawDeg = currentYawDeg + (diff > 0 ? step : -step);}
		SetRotation(XMVectorSet(0.0f, nextYawDeg, 0.0f, 0.0f));

		XMVECTOR deltaPosition = XMVectorScale(direction, m_moveSpeedSquared * TimeManager::GetInstance().GetDeltaTime());
		MovePosition(deltaPosition);
	}
}

void Enemy::OnAttackFinished()
{
	if (m_state == AIState::Dead) return;

	// 공격이 끝났으니 다시 추적 상태로 복귀
	// (만약 튜토리얼용 허수아비라면 여기서 Idle로 보내는 분기 추가 가능)
	m_state = AIState::Chase;

	if (m_fsm) m_fsm->ChangeState(FSMComponentEnemy::EChase);
}

void Enemy::Finalize()
{
	auto it = find(s_enemies.begin(), s_enemies.end(), this);
	if (it != s_enemies.end()) s_enemies.erase(it);
}

nlohmann::json Enemy::Serialize()
{
	nlohmann::json jsonData = {};

	jsonData["isTutorialDummy"] = m_isTutorialDummy;
	jsonData["triggerColliderName"] = m_triggerColliderName;

	return jsonData;
}

void Enemy::Deserialize(const nlohmann::json& jsonData)
{
	if (jsonData.find("isTutorialDummy") != jsonData.end()) if (jsonData["isTutorialDummy"].get<bool>()) SetAsTutorialDummy();
	if (jsonData.find("triggerColliderName") != jsonData.end()) m_triggerColliderName = jsonData["triggerColliderName"].get<string>();
}

void Enemy::ApplySeparation(float dt)
{
	if (m_state == AIState::Dead || !GetAlive()) return;
	if (s_enemies.size() <= 1) return;

	const float radius = m_separationRadius;
	const float radiusSq = radius * radius;

	XMVECTOR myPos = GetPosition();
	XMVECTOR push = XMVectorZero();

	for (Enemy* other : s_enemies)
	{
		if (!other || other == this) continue;
		if (!other->GetAlive()) continue;
		if (other->m_state == AIState::Dead) continue;

		XMVECTOR diff = XMVectorSubtract(myPos, other->GetPosition());
		diff = XMVectorSetY(diff, 0.0f);

		float distSq = XMVectorGetX(XMVector3LengthSq(diff));
		if (distSq < numeric_limits<float>::epsilon() || distSq > radiusSq) continue;

		float dist = sqrtf(distSq);
		float strength = (radius - dist) / radius;

		XMVECTOR dir = XMVectorScale(diff, 1.0f / dist);
		push = XMVectorAdd(push, XMVectorScale(dir, strength));
	}

	float pushLenSq = XMVectorGetX(XMVector3LengthSq(push));
	if (pushLenSq < numeric_limits<float>::epsilon()) return;

	XMVECTOR pushDir = XMVector3Normalize(push);
	XMVECTOR delta = XMVectorScale(pushDir, m_separationStrength * dt);

	float maxPush = m_separationMaxPush * dt;
	float deltaLen = XMVectorGetX(XMVector3Length(delta));
	if (deltaLen > maxPush)
	{
		delta = XMVectorScale(pushDir, maxPush);
	}

	delta = XMVectorSet(XMVectorGetX(delta), 0.0f, XMVectorGetZ(delta), 0.0f);
	MovePosition(delta);
}
