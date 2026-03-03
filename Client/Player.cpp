#include "stdafx.h"
#include "Player.h"

#include "Renderer.h"
#include "SoundManager.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "ColliderComponent.h"
#include "CameraComponent.h"
#include "ResourceManager.h"
#include "ModelComponent.h"
#include "SceneBase.h"
#include "Enemy.h"
#include "ParticleObject.h"
#include "NavigationManager.h"
#include "SceneManager.h"
#include "GameManager.h"
#include "ParticleComponent.h"
#include "Boss.h"

#include "FSMComponentGun2.h"

#include "Shared/Config/Option.h"

REGISTER_TYPE(Player)

using namespace std;
using namespace DirectX;

float Player::m_cameraSensitivity = 0.05f;
bool Player::m_hasDashedForTutorial = false;
bool Player::m_hasShotForTutorial = false;
bool Player::m_hasReloadedForTutorial = false;
bool Player::m_hasAutoReloadedForTutorial = false;
bool Player::m_hasUsedDeadEyeForTutorial = false;

void Player::Initialize()
{
	XMStoreFloat3(&m_playerRotation, GetRotation());

	ResourceManager& resourceManager = ResourceManager::GetInstance();
	m_lineVertexBufferAndShader = resourceManager.GetVertexShaderAndInputLayout("VSLine.hlsl");
	m_linePixelShader = resourceManager.GetPixelShader("PSColor.hlsl");

	m_cameraComponent = GetComponent<CameraComponent>();
	m_cameraComponent->SetAsMainCamera();
	m_gunObject = GetChildGameObject("Gun");
	m_gunTip = m_gunObject->GetChildGameObject("GunTip");
	m_gunFSM = m_gunObject->GetComponent<FSMComponentGun2>();

	m_playerHitPointExpressions[0] = resourceManager.GetTextureAndOffset("UI_face_smile.png");
	m_playerHitPointExpressions[1] = resourceManager.GetTextureAndOffset("UI_face_default.png");
	m_playerHitPointExpressions[2] = resourceManager.GetTextureAndOffset("UI_Face_Sad.png");

	m_playerProfileBGTextureAndOffset = resourceManager.GetTextureAndOffset("UI_profile_BG.png");
	m_playerHitPointTextureAndOffset = resourceManager.GetTextureAndOffset("UI_Gauge_HP.png");
	m_playerHitPointDecoTextureAndOffset = resourceManager.GetTextureAndOffset("UI_Gauge_HP_Deco.png");
	m_deadEyeCoolDownTextureAndOffset = resourceManager.GetTextureAndOffset("UI_Gauge_Energy.png");
	m_deadEyeTextureAndOffset = resourceManager.GetTextureAndOffset("Crosshair.png");
	m_enemyHitTextureAndOffset = resourceManager.GetTextureAndOffset("CrosshairHit.png");

	m_bulletImgs[0] = resourceManager.GetTextureAndOffset("UI_Bullet0.png");
	m_bulletImgs[1] = resourceManager.GetTextureAndOffset("UI_Bullet1.png");
	m_bulletImgs[2] = resourceManager.GetTextureAndOffset("UI_Bullet2.png");
	m_bulletImgs[3] = resourceManager.GetTextureAndOffset("UI_Bullet3.png");
	m_bulletImgs[4] = resourceManager.GetTextureAndOffset("UI_Bullet4.png");
	m_bulletImgs[5] = resourceManager.GetTextureAndOffset("UI_Bullet5.png");
	m_bulletImgs[6] = resourceManager.GetTextureAndOffset("UI_Bullet6.png");

	m_DeadEyeCount = 4;
	m_bulletCnt = 6;

	m_bulletUIpos = { 0.965f,0.85f };
	m_bulletInterval = 0.03f;




	m_hasDashedForTutorial = false;
	m_hasShotForTutorial = false;
	m_hasReloadedForTutorial = false;
	m_hasAutoReloadedForTutorial = false;
	m_hasUsedDeadEyeForTutorial = false;
	m_originalHeight = XMVectorGetY(GetPosition());

	SetAction(Action::All, true);
}

void Player::Update()
{
	float deltaTime = TimeManager::GetInstance().GetDeltaTime();
	InputManager& input = InputManager::GetInstance();
	auto& sm = SoundManager::GetInstance();

	UpdateRotation(input, deltaTime);
	UpdateMoveDirection(input);

	TutorialStep();
	
	if (										   m_ControlState.CanAutoReload && m_bulletCnt == 0 )																	PlayerAutoReload(1);
	if (input.GetKeyDown(KeyCode::MouseLeft)	&& m_ControlState.CanShoot		&& m_bulletCnt > 0		&&	sm.CheckRhythm(Config::InputCorrection) < InputType::Miss)	PlayerShoot();
	if (input.GetKeyDown(KeyCode::Space)		&& m_ControlState.CanDash		&& !m_isDashing 		&&	sm.CheckRhythm(Config::InputCorrection) < InputType::Miss)	PlayerTriggerDash();

	if (m_ControlState.CanSkill && !m_isDeadEyeActive)
	{
		if (m_deadEyeCoolDownTimer >= m_deadEyeCoolDownDuration && input.GetKeyDown(KeyCode::MouseRight) && sm.CheckRhythm(Config::InputCorrection) < InputType::Miss) PlayerDeadEyeStart();
		else m_deadEyeCoolDownTimer += deltaTime;
	}

	XMVECTOR previousPosition = GetPosition();
	
	if (m_isDeadEyeActive)				PlayerDeadEye(deltaTime, input);	
	if (m_isDashing)					PlayerDash(deltaTime);
	else if (m_ControlState.CanMove)
	{
		MovePosition(m_normalizedMoveDirection * m_moveSpeed * deltaTime);

		float moveDirLength = XMVectorGetX(XMVector3LengthSq(m_normalizedMoveDirection));

		if (moveDirLength > 0.1f)
		{
			m_headBobTimer += deltaTime * 10.0f;
			SetPosition(XMVectorSetY(GetPosition(), m_originalHeight + (cosf(m_headBobTimer) - 1.0f) * 0.1f));
		}
		else
		{
			m_headBobTimer = 0.0f;
			float currentHeight = XMVectorGetY(GetPosition());
			float heightDiff = m_originalHeight - currentHeight;
			if (fabsf(heightDiff) > 0.01f)
			{
				float adjustSpeed = 5.0f;
				float newY = currentHeight + heightDiff * adjustSpeed * deltaTime;
				SetPosition(XMVectorSetY(GetPosition(), newY));
			}
			else SetPosition(XMVectorSetY(GetPosition(), m_originalHeight));
		}
	};
	
	if (NavigationManager::GetInstance().FindNearestPoly(XMVectorSetY(GetPosition(), 0.0f), 3.0f) < 0) SetPosition(previousPosition);

	if (input.GetKeyDown(KeyCode::R) && m_ControlState.CanReload && m_bulletCnt > 0)
	{
		switch (sm.CheckRhythm(Config::InputCorrection))
		{
		case InputType::Early:
			LOG("Early");
			PlayerReload(0);
			break;
		case InputType::Perfect:
			LOG("Perfect");
			PlayerReload(0);
			break;
		case InputType::Late:
			LOG("Late");
			PlayerReload(0);
			break;
		}
	}

	for_each(m_lineBuffers.begin(), m_lineBuffers.end(), [&](auto& pair) { pair.second -= deltaTime; });
	if (!m_lineBuffers.empty() && m_lineBuffers.front().second < 0.0f) m_lineBuffers.pop_front();
	if (m_enemyHitTimer > -1.0f) m_enemyHitTimer -= deltaTime;
	if (m_invincibilityTimer > -1.0f) m_invincibilityTimer -= deltaTime;
	if (!m_playerHitPoint && m_invincibilityTimer <= 0.0f)
	{
		GameManager::GetInstance().SetSuccess(false);
		SceneManager::GetInstance().ChangeScene("EndingScene");
	}

	if (m_redVignetteIntensity > 0.0f)
	{
		m_redVignetteIntensity -= deltaTime * 0.05f;
		SceneBase::SetVignettingIntensity(m_redVignetteIntensity);
		if (m_redVignetteIntensity <= 0.0f) SceneBase::SetPostProcessingFlag(PostProcessingBuffer::PostProcessingFlag::Vignetting, false);
	}

	if (input.GetKey(KeyCode::LeftBracket))	{ m_bulletUIpos.first += 0.1f * deltaTime; }
	if (input.GetKey(KeyCode::RightBracket)) { m_bulletUIpos.second += 0.1f * deltaTime; }
	if (input.GetKey(KeyCode::Num0)) { m_bulletInterval += 0.01f * deltaTime; }

	UpdateLutCrossfade(deltaTime);
}

void Player::Render()
{
	Renderer& renderer = Renderer::GetInstance();

	RenderPlayerHitPointUI(renderer);
	RenderDeadEyeCoolDownUI(renderer);
	if (!m_lineBuffers.empty()) RenderLineBuffers(renderer);
	if (!m_deadEyeTargets.empty()) RenderDeadEyeTargetsUI(renderer);
	if (m_enemyHitTimer > 0.0f) RenderEnemyHitUI(renderer);
	RenderBullets(renderer);
}

void Player::Finalize()
{
	TimeManager::GetInstance().SetTimeScale(1.0f);
}

void Player::TutorialStep() const
{
	switch (GameManager::GetInstance().GetTutorialStep())
	{
	case ETutorialStep::Dash:
		if (m_hasDashedForTutorial) GameManager::GetInstance().SetTutorialStep(ETutorialStep::Shoot);
		break;

	case ETutorialStep::Shoot:
		if (m_bulletCnt < m_MaxBullet-2 && m_hasShotForTutorial) GameManager::GetInstance().SetTutorialStep(ETutorialStep::Reload);
		break;

	case ETutorialStep::Reload:
		if (m_bulletCnt == m_MaxBullet && m_hasReloadedForTutorial) GameManager::GetInstance().SetTutorialStep(ETutorialStep::AutoReload);
		break;

	case ETutorialStep::AutoReload:
		if (m_hasAutoReloadedForTutorial) GameManager::GetInstance().SetTutorialStep(ETutorialStep::DeadEye);
		break;

	case ETutorialStep::DeadEye:
		if (InputManager::GetInstance().GetKeyDown(KeyCode::MouseLeft)) GameManager::GetInstance().SetTutorialStep(ETutorialStep::DeadTwo);
		break;

	case ETutorialStep::DeadTwo:
		if (m_hasUsedDeadEyeForTutorial) GameManager::GetInstance().SetTutorialStep(ETutorialStep::End);
		break;
	}
}

void Player::TakeHit()
{
	if (m_invincibilityTimer > 0.0f) return;

	m_playerHitPoint--;
	GameManager::GetInstance().OnPlayerHit();
	SoundManager::GetInstance().UI_Shot(Config::Player_Hit);

	SceneBase::SetPostProcessingFlag(PostProcessingBuffer::PostProcessingFlag::Vignetting, true);
	SceneBase::SetVignettingColor({ 1.0f, 0.0f, 0.0f });
	m_redVignetteIntensity = 0.02f;
	m_invincibilityTimer = m_invincibilityDuration;
}

void Player::RestoreHitPoint()
{
	m_playerHitPoint = m_maxPlayerHitPoint;
}

void Player::SetAction(Action state, bool enabled)
{
	switch (state)
	{
	case Action::Move:		 m_ControlState.CanMove			= enabled;		break;
	case Action::Dash:		 m_ControlState.CanDash			= enabled;		break;
	case Action::Reload:	 m_ControlState.CanReload		= enabled;		break;
	case Action::Shoot:		 m_ControlState.CanShoot		= enabled;		break;
	case Action::AutoReload: m_ControlState.CanAutoReload	= enabled;		break;
	case Action::DeadEye:	 m_ControlState.CanSkill		= enabled;		break;

	case Action::All:		 m_ControlState.CanMove = enabled;
							 m_ControlState.CanDash = enabled;
							 m_ControlState.CanReload = enabled;
							 m_ControlState.CanShoot = enabled;
							 m_ControlState.CanAutoReload = enabled;
							 m_ControlState.CanSkill = enabled;
							 break;
	}
}

void Player::UpdateRotation(InputManager& input, float deltaTime)
{
	const POINT& mouseDelta = input.GetMouseDelta();

	m_playerRotation.x += static_cast<float>(mouseDelta.y) * m_cameraSensitivity;
	m_playerRotation.y += static_cast<float>(mouseDelta.x) * m_cameraSensitivity;

	constexpr float LIMIT = 90.0f - 1.0f;
	if (m_playerRotation.x > LIMIT) m_playerRotation.x = LIMIT;
	if (m_playerRotation.x < -LIMIT) m_playerRotation.x = -LIMIT;

	SetRotation({ m_playerRotation.x, m_playerRotation.y, 0.0f, 0.0f });

	m_playerRotation.z = lerp(m_playerRotation.z, 0.0f, deltaTime * 5.0f);
	Rotate({ 0.0f, 0.0f, m_playerRotation.z, 0.0f });
}

void Player::UpdateMoveDirection(InputManager& input)
{
	m_inputDirection =
	{
		static_cast<float>(input.GetKey(KeyCode::D)) - static_cast<float>(input.GetKey(KeyCode::A)),
		0.0f,
		static_cast<float>(input.GetKey(KeyCode::W)) - static_cast<float>(input.GetKey(KeyCode::S)),
		0.0f
	};

	XMVECTOR yawQuaternion = XMQuaternionRotationRollPitchYaw(0.0f, m_playerRotation.y * DEG_TO_RAD, 0.0f);
	m_normalizedMoveDirection = XMVector3Normalize(XMVector3Rotate(m_inputDirection, yawQuaternion));
}

void Player::PlayerTriggerDash()
{
	if (XMVectorGetX(XMVector3LengthSq(m_normalizedMoveDirection)) <= numeric_limits<float>::epsilon()) return;

	m_isDashing = true;
	m_dashTimer = m_kDashDuration;
	m_dashDirection = m_normalizedMoveDirection;
	const float inputX = XMVectorGetX(XMVector3Normalize(m_inputDirection));

	// 移대찓?占쏙옙 ?占쏙옙?占쏙옙
	m_playerRotation.z = -Config::Player_Dash_Tilt * inputX;

	// Radial Blur ?占쏙옙?占쏙옙
	SceneBase::SetRadialBlurCenter({ inputX * 0.5f + 0.5f, 0.5f });
	SceneBase::SetRadialBlurDist(0.33f);
	SceneBase::SetRadialBlurStrength(1.7f);
	SceneBase::SetPostProcessingFlag(PostProcessingBuffer::PostProcessingFlag::RadialBlur, true);

	// ?占쏙옙?占쏙옙?占쏙옙 ?占쏙옙湲곕떎 ?占쏙옙?占쏙옙?占쏙옙 ?占쏙옙
	SoundManager::GetInstance().SFX_Shot(GetPosition(), Config::Player_Dash);
}

void Player::PlayerDash(float deltaTime)
{
	m_dashTimer -= deltaTime;
	MovePosition(m_dashDirection * m_kDashSpeed * deltaTime);
	if (m_dashTimer <= 0.0f) m_isDashing = false;

	float t = 1.0f - (m_dashTimer / m_kDashDuration); // 0->1
	float smooth = t * t * (3.0f - 2.0f * t); // smoothstep
	SceneBase::SetRadialBlurStrength(8.0f * (1.0f - smooth)); // ?占쏙옙?占쏙옙 ?占쏙옙?占쏙옙占�?

	if (m_dashTimer <= 0.0f)
	{
		m_isDashing = false;
		m_hasDashedForTutorial = true;
		SceneBase::SetRadialBlurStrength(0.0f);
		SceneBase::SetPostProcessingFlag(PostProcessingBuffer::PostProcessingFlag::RadialBlur, false);
	}
}

void Player::PlayerShoot()
{
	if (!m_gunObject || m_isDeadEyeActive) return;

	if (m_gunFSM) m_gunFSM->Fire();

	--m_bulletCnt;

	if(!m_isDeadEyeActive)
	SoundManager::GetInstance().UI_Shot(Config::Player_Shoot);

	const XMVECTOR& origin = GetPosition();
	const XMVECTOR& direction = GetWorldDirectionVector(Direction::Forward);
	float distance = 0.0f;
	GameObjectBase* hit = ColliderComponent::CheckCollision(origin, direction, distance);
	if (!hit) distance = 100.0f;
	const XMVECTOR& hitPosition = XMVectorAdd(origin, XMVectorScale(direction, distance));

	const XMVECTOR& gunPos = m_gunTip->GetWorldPosition();

	ParticleObject* smoke = dynamic_cast<ParticleObject*>(CreatePrefabChildGameObject("Smoke.json"));
	smoke->SetPosition(gunPos);
	smoke->SetScale({ 1.0f, 1.0f, distance, 1.0f });
	smoke->GetChildGameObject("SmokeLine")->GetComponent<ParticleComponent>()->SetParticleAmount(static_cast<int>(distance) * 25);
	smoke->LookAt(hitPosition);
	smoke->SetLifetime(5.0f);

	ParticleObject* muzzleFlash = dynamic_cast<ParticleObject*>(CreatePrefabChildGameObject("MuzzleFlash.json"));
	muzzleFlash->SetPosition(gunPos);
	muzzleFlash->SetLifetime(1.0f);

	if (Enemy* enemy = dynamic_cast<Enemy*>(hit))
	{
		enemy->Die();
		ParticleObject* gem = dynamic_cast<ParticleObject*>(CreatePrefabChildGameObject("Gem.json"));
		gem->SetPosition(hitPosition);
		gem->SetLifetime(5.0f);

		m_enemyHitTimer = m_enemyHitDisplayTime;

		TriggerLUT();
	}
	if (Boss* boss = dynamic_cast<Boss*>(hit))
	{
		boss->Hit();
		ParticleObject* gem = dynamic_cast<ParticleObject*>(CreatePrefabChildGameObject("Gem.json"));
		gem->SetPosition(hitPosition);
		gem->SetLifetime(5.0f);

		m_enemyHitTimer = m_enemyHitDisplayTime;

		TriggerLUT();
	}
}

void Player::PlayerReload(int cnt)
{
	if (m_bulletCnt == m_MaxBullet) return;

	if (m_gunFSM) m_gunFSM->Reload();

	m_ControlState.CanShoot = false;
	m_ControlState.CanReload = false;
	m_ControlState.CanAutoReload = false;


	//Reload Anime + rhythm check

	SoundManager::GetInstance().UI_Shot(Config::Player_Reload_Spin);

	int reloadCount = Config::Player_Reload_Cocking_Count + cnt;
	SoundManager::GetInstance().AddNodeDestroyedListenerOnce([this, cnt = reloadCount]()mutable ->bool
		{
			if (--cnt > 0)
			{
				return false;
			}
			SoundManager::GetInstance().UI_Shot(Config::Player_Reload_Cocking);

			SoundManager::GetInstance().AddNodeDestroyedListenerOnce([this, cnt = 2]()mutable ->bool
				{
					if (--cnt > 0)
					{
						return false;
					}
					m_ControlState.CanShoot = true;
					m_ControlState.CanReload = true;
					m_ControlState.CanAutoReload = true;
					m_hasReloadedForTutorial = true;

					return true;
				});

			m_bulletCnt = m_MaxBullet;
			return true;
		});
}

void Player::PlayerAutoReload(int cnt)
{
	if (m_gunFSM) m_gunFSM->Reload();

	m_ControlState.CanAutoReload = false;
	m_ControlState.CanShoot = false;

	ReloadState reloadState = ReloadState::WaitSpin;

	SoundManager::GetInstance().AddNodeDestroyedListenerOnce(
		[this, state = reloadState, waitCount = cnt]() mutable -> bool
		{
			switch (state)
			{
			case ReloadState::WaitSpin:
				//std::cout << waitCount;
				if (--waitCount <= 0)
				{
					SoundManager::GetInstance().UI_Shot(Config::Player_Reload_Spin);
					state = ReloadState::WaitCock;
					waitCount = 1;
					//std::cout << "WaitSpin" << std::endl;
					m_ControlState.CanAutoReload = false;
					m_ControlState.CanShoot = false;
				}
				return false;

			case ReloadState::WaitCock:
				//std::cout << waitCount;
				if (--waitCount == 0)
				{
					SoundManager::GetInstance().UI_Shot(Config::Player_Reload_Cocking);
					

					state = ReloadState::WaitEnableShoot;
					waitCount = 1;
					//std::cout << "WaitCock" << std::endl;
					m_ControlState.CanAutoReload = false;
					m_ControlState.CanShoot = false;
					m_bulletCnt = m_MaxBullet;
				}
				return false;

			case ReloadState::WaitEnableShoot:
				if (--waitCount == 0)
				{					
					m_ControlState.CanAutoReload = true;
					m_ControlState.CanShoot = true;
					m_hasAutoReloadedForTutorial = true;
					//std::cout << "WaitEnableShoot" << std::endl;
					return true;
				}
				return false;

			default:
				return true;
			}
		});
}

void Player::PlayerDeadEyeStart()
{
	vector<GameObjectBase*> hits = ColliderComponent::CheckCollision(m_cameraComponent->GetBoundingFrustum());
	if (hits.empty()) return;

	bool hasEnemy = false;

	for (GameObjectBase* hit : hits)
	{
		if (Enemy* enemy = dynamic_cast<Enemy*>(hit))
		{
			float distance = 0.0f;
			const XMVECTOR& origin = GetPosition();
			const XMVECTOR& targetPos = XMVectorAdd(enemy->GetWorldPosition(), { 0.0f, 1.0f, 0.0f, 0.0f }); // ?占쏙옙 異⑹떖?占쏙옙 y = 0.0f?占쏙옙?占쏙옙 ?占쏙옙占�? ?占쏙옙占�?
			if (!dynamic_cast<Enemy*>(ColliderComponent::CheckCollision(origin, XMVectorSubtract(targetPos, origin), distance))) continue;

			hasEnemy = true;
			XMFLOAT2 distancePair = Renderer::GetInstance().ToUIPosition(m_cameraComponent->WorldToScreenPosition(targetPos));
			m_deadEyeTargets.emplace_back(powf(distancePair.x - 0.5f, 2) + powf(distancePair.y - 0.5f, 2), enemy);
		}
	}
	if (!hasEnemy) return;

	m_deadEyeCoolDownTimer = 0.0f;

	m_isDeadEyeActive = true;

	TimeManager::GetInstance().SetTimeScale(0.1f);

	m_cameraSensitivity = 0.01f;

	SceneBase::SetPostProcessingFlag(PostProcessingBuffer::PostProcessingFlag::Grayscale, true);

	sort(m_deadEyeTargets.begin(), m_deadEyeTargets.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
	if (m_deadEyeTargets.size() > 6) m_deadEyeTargets.resize(6);
	sort(m_deadEyeTargets.begin(), m_deadEyeTargets.end(), [&](const auto& a, const auto& b) { return m_cameraComponent->WorldToScreenPosition(a.second->GetWorldPosition()).x > m_cameraComponent->WorldToScreenPosition(b.second->GetWorldPosition()).x; });

	SoundManager::GetInstance().ChangeLowpass();

	m_currentNodeIndex = SoundManager::GetInstance().GetRhythmTimerIndex();
	m_DeadEyeCount = m_deadEyeTargets.size();
	m_deadEyeTotalDuration = static_cast<float>(m_DeadEyeCount) * 0.5f;
	m_deadEyeDuration = 0.0f;

	m_prevDeadEyePos = m_cameraComponent->WorldToScreenPosition(XMVectorAdd(m_deadEyeTargets.back().second->GetWorldPosition(), { 0.0f, 1.2f, 0.0f, 0.0f }));
	m_nextDeadEyePos = m_prevDeadEyePos;
}

void Player::PlayerDeadEye(float deltaTime, InputManager& input)
{
	if (m_deadEyeTargets.empty()) { PlayerDeadEyeEnd(); return; }

	m_deadEyeDuration += deltaTime;
	m_deadEyeMoveTimer += deltaTime * m_deadEyeMoveSpeed;

	float effectIntensity = min((m_deadEyeDuration / m_deadEyeTotalDuration) * 16.0f, 1.0f);
	SceneBase::SetGrayScaleIntensity(effectIntensity);

	const XMVECTOR& targetPos = XMVectorAdd(m_deadEyeTargets.back().second->GetWorldPosition(), { 0.0f, 1.2f, 0.0f, 0.0f });
	m_nextDeadEyePos = m_cameraComponent->WorldToScreenPosition(targetPos);
	//m_gunObject->LookAt(targetPos);
	//m_gunObject->Rotate({ 0.0f, 90.0f, 0.0f, 0.0f });
	//�씠寃� 留욌깘..
	// UPvector瑜� 諛붽퓭�빞 �븯�뒗�뜲 �씠�쑀瑜� 紐⑤Ⅴ寃좎쓬
	const XMVECTOR gunPos = m_gunObject->GetWorldPosition();
	XMVECTOR dir = XMVectorSubtract(targetPos, gunPos);
	dir = XMVector3Normalize(dir);

	const float dx = XMVectorGetX(dir);
	const float dy = XMVectorGetY(dir);
	const float dz = XMVectorGetZ(dir);
	const float yaw = XMConvertToDegrees(atan2f(dx, dz));
	const float pitch = XMConvertToDegrees(-atan2f(dy, sqrtf(dx * dx + dz * dz)));

	const float localYaw = yaw - m_playerRotation.y;
	const float localPitch = pitch - m_playerRotation.x;

	constexpr float kGunForwardYawOffset = 90.0f;
	m_gunObject->SetRotation({ localPitch, localYaw + kGunForwardYawOffset, 0.0f, 0.0f });

	if (input.GetKeyDown(KeyCode::MouseLeft))
	{
		SoundManager::GetInstance().SFX_Shot(GetPosition(), Config::Player_DeadEye_Shoot);

		m_prevDeadEyePos = m_nextDeadEyePos;
		m_deadEyeTargets.back().second->Die();
		if (m_deadEyeTargets.size() > 1)
		{
			m_nextDeadEyePos = m_cameraComponent->WorldToScreenPosition(XMVectorAdd(m_deadEyeTargets[m_deadEyeTargets.size() - 2].second->GetWorldPosition(), { 0.0f, 1.2f, 0.0f, 0.0f }));
			m_deadEyeMoveTimer = 0.0f;
		}

		const XMVECTOR& gunPos = m_gunTip->GetWorldPosition();

		ParticleObject* smoke = dynamic_cast<ParticleObject*>(CreatePrefabChildGameObject("Smoke.json"));
		smoke->SetPosition(gunPos);
		float length = XMVectorGetX(XMVector3LengthEst(XMVectorSubtract(gunPos, targetPos)));
		smoke->SetScale({ 1.0f, 1.0f, length, 1.0f });
		smoke->GetChildGameObject("SmokeLine")->GetComponent<ParticleComponent>()->SetParticleAmount(static_cast<int>(length) * 25);
		smoke->LookAt(targetPos);
		smoke->SetLifetime(5.0f);

		ParticleObject* muzzleFlash = dynamic_cast<ParticleObject*>(CreatePrefabChildGameObject("MuzzleFlash.json"));
		muzzleFlash->SetPosition(gunPos);
		muzzleFlash->SetLifetime(1.0f);

		ParticleObject* gem = dynamic_cast<ParticleObject*>(CreatePrefabChildGameObject("Gem.json"));
		gem->SetPosition(targetPos);
		gem->SetLifetime(5.0f);

		m_deadEyeTargets.pop_back();
	}

	//if (SoundManager::GetInstance().GetRhythmTimerIndex() >= m_currentNodeIndex + m_DeadEyeCount) PlayerDeadEyeEnd();
}

void Player::PlayerDeadEyeEnd()
{
	if (m_gunObject)
	{
		m_gunObject->SetRotation({ 0.0f, 90.0f, 0.0f, 0.0f });
		if (m_gunFSM) m_gunFSM->Fire();
		
	}

	m_isDeadEyeActive = false;

	TimeManager::GetInstance().SetTimeScale(1.0f);

	m_cameraSensitivity = 0.1f;

	m_deadEyeTargets.clear();

	SceneBase::SetPostProcessingFlag(PostProcessingBuffer::PostProcessingFlag::Grayscale, false);
	SceneBase::SetGrayScaleIntensity(0.0f);

	SoundManager::GetInstance().ChangeLowpass();

	m_hasUsedDeadEyeForTutorial = true;
	//TriggerLUT();
}

void Player::RenderPlayerHitPointUI(Renderer& renderer)
{
	renderer.UI_RENDER_FUNCTIONS().emplace_back
	(
		[&]()
		{
			//const DirectX::XMFLOAT2 profileBgPos = { 275.0f, -65.0f };
			const float hitPointRatio = static_cast<float>(m_playerHitPoint) / static_cast<float>(m_maxPlayerHitPoint);
			LONG width = static_cast<LONG>(m_playerHitPointTextureAndOffset.second.x);

			if (m_playerHitPoint != m_maxPlayerHitPoint)
			{
				width = static_cast<LONG>(lerp(m_playerHitPointTextureAndOffset.second.x * hitPointRatio, m_playerHitPointTextureAndOffset.second.x * (m_playerHitPoint + 1) / static_cast<float>(m_maxPlayerHitPoint), max(0.0f, m_invincibilityTimer / m_invincibilityDuration)));
			}

			RECT hitPointSrcRect =
			{
				.left = 0,
				.top = 0,
				.right = width,
				.bottom = static_cast<LONG>(m_playerHitPointTextureAndOffset.second.y)
			};

			Renderer& renderer = Renderer::GetInstance();

			int expressionIndex = min(static_cast<int>((1.0f - hitPointRatio) * 3.0f), 2);

			renderer.RenderImageWrapScreenPosition
			(
				m_playerProfileBGTextureAndOffset.first,
				{ 205.0f, -80.0f },
				m_playerProfileBGTextureAndOffset.second,
				0.5f,
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				0.0f
			);

			renderer.RenderImageWrapScreenPosition
			(
				m_playerHitPointExpressions[expressionIndex].first,
				{ 200.0f, -80.0f },
				m_playerHitPointExpressions[expressionIndex].second,
				0.5f,
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				0.5f
			);

			renderer.RenderImageWrapScreenPosition
			(
				m_playerHitPointTextureAndOffset.first,
				{ 392.0f, -86.0f },
				m_playerHitPointTextureAndOffset.second,
				1.0f,
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				0.0f,
				&hitPointSrcRect
			);

			renderer.RenderImageWrapScreenPosition
			(
				m_playerHitPointDecoTextureAndOffset.first,
				{ 212.0f, -86.0f },
				m_playerHitPointDecoTextureAndOffset.second,
				0.4f,
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				0.1f
			);
		}
	);
}

void Player::RenderDeadEyeCoolDownUI(Renderer& renderer)
{
	renderer.UI_RENDER_FUNCTIONS().emplace_back
	(
		[&]()
		{
			const float coolDownRatio = m_deadEyeCoolDownTimer / m_deadEyeCoolDownDuration;
			LONG width = static_cast<LONG>(m_deadEyeCoolDownTextureAndOffset.second.x * min(coolDownRatio, 1.0f));

			RECT coolDownSrcRect =
			{
				.left = 0,
				.top = 0,
				.right = width,
				.bottom = static_cast<LONG>(m_deadEyeCoolDownTextureAndOffset.second.y)
			};

			Renderer::GetInstance().RenderImageWrapScreenPosition
			(
				m_deadEyeCoolDownTextureAndOffset.first,
				{ 241.0f, -70.0f },
				m_deadEyeCoolDownTextureAndOffset.second,
				1.0f,
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				0.0f,
				&coolDownSrcRect
			);
		}
	);
}

void Player::RenderLineBuffers(Renderer& renderer)
{
	renderer.RENDER_FUNCTION(RenderStage::Scene, BlendState::Opaque).emplace_back
	(
		0.0f,
		[&]()
		{
			ResourceManager& resourceManager = ResourceManager::GetInstance();
			com_ptr<ID3D11DeviceContext> deviceContext = Renderer::GetInstance().GetDeviceContext();

			deviceContext->IASetInputLayout(m_lineVertexBufferAndShader.second.Get());
			deviceContext->VSSetShader(m_lineVertexBufferAndShader.first.Get(), nullptr, 0);
			deviceContext->PSSetShader(m_linePixelShader.Get(), nullptr, 0);

			resourceManager.SetRasterState(RasterState::SolidCullNone);
			resourceManager.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

			for (const auto& [lineBuffer, time] : m_lineBuffers)
			{
				deviceContext->UpdateSubresource(resourceManager.GetConstantBuffer(VSConstBuffers::Line).Get(), 0, nullptr, &lineBuffer, 0, 0);
				deviceContext->Draw(2, 0);
			}
		}
	);
}

void Player::RenderDeadEyeTargetsUI(Renderer& renderer)
{
	renderer.UI_RENDER_FUNCTIONS().emplace_back
	(
		[&]()
		{
			Renderer::GetInstance().RenderImageScreenPosition
			(
				m_deadEyeTextureAndOffset.first,
				{
					lerp(m_prevDeadEyePos.x, m_nextDeadEyePos.x, min(m_deadEyeMoveTimer, 1.0f)),
					lerp(m_prevDeadEyePos.y, m_nextDeadEyePos.y, min(m_deadEyeMoveTimer, 1.0f))
				},
				m_deadEyeTextureAndOffset.second, 0.5f
			);
		}
	);
}

void Player::RenderEnemyHitUI(Renderer& renderer)
{
	renderer.UI_RENDER_FUNCTIONS().emplace_back
	(
		[&]()
		{
			Renderer::GetInstance().RenderImageNrmPosition(m_enemyHitTextureAndOffset.first, { 0.5f, 0.5f }, m_enemyHitTextureAndOffset.second, 0.5f);
		}
	);
}

void Player::RenderBullets(class Renderer& renderer)
{
	renderer.UI_RENDER_FUNCTIONS().emplace_back
	(
		[&]()
		{
			Renderer::GetInstance().RenderImageNrmPosition
			(
				m_bulletImgs[m_bulletCnt].first,
				{ m_bulletUIpos.first, m_bulletUIpos.second },
				m_bulletImgs[m_bulletCnt].second,
				0.6f
			);
		}
	);
}

void Player::SetCameraSensitivity(float val)
{
	m_cameraSensitivity = val * 0.01f;
}

float Player::GetCameraSensitivity()
{
	return m_cameraSensitivity;
}

int Player::GetBulletCount() const
{
	return m_bulletCnt;
}

int Player::GetMaxBullet() const
{
	return m_MaxBullet;
}

float Player::GetHPRatio() const
{
	if (m_maxPlayerHitPoint <= 0) return 0.0f;
	float ratio = static_cast<float>(m_playerHitPoint) / static_cast<float>(m_maxPlayerHitPoint);
	if (ratio < 0.0f) ratio = 0.0f;
	if (ratio > 1.0f) ratio = 1.0f;
	return ratio;
}

float Player::GetDeadEyeCooldownRatio() const
{
	if (m_deadEyeCoolDownDuration <= 0.0f) return 0.0f;
	float ratio = m_deadEyeCoolDownTimer / m_deadEyeCoolDownDuration;
	if (ratio < 0.0f) ratio = 0.0f;
	if (ratio > 1.0f) ratio = 1.0f;
	return ratio;
}

void Player::UpdateLutCrossfade(float deltaTime)
{
	if (!m_lutCrossfadeActive) return;

	m_lutCrossfadeElapsed += deltaTime;
	float t = m_lutCrossfadeElapsed / m_lutCrossfadeDuration;
	if (t > 1.0f) t = 1.0f;

	// smoothstep (源붾걫/媛먯냽)
	float smooth = t * t * (3.0f - 2.0f * t);
	float factor = m_lutCrossfadeReverse ? (1.0f - smooth) : smooth;

	SceneBase::SetLutLerpFactor(factor);

	if (t >= 1.0f)
	{
		if (!m_lutCrossfadeReverse)
		{
			// 諛⑺뼢 
			m_lutCrossfadeReverse = true;
			m_lutCrossfadeElapsed = 0.0f;
		}
		else
		{
			// 醫낅즺: flag off + 
			m_lutCrossfadeActive = false;
			m_lutCrossfadeReverse = false;
			SceneBase::SetPostProcessingFlag(PostProcessingBuffer::PostProcessingFlag::LUT_CROSSFADE, false);
			SceneBase::SetLutLerpFactor(0.0f);
		}
	}
}

void Player::TriggerLUT()
{
	m_lutCrossfadeActive = true;
	m_lutCrossfadeReverse = false;
	m_lutCrossfadeElapsed = 0.0f;
	SceneBase::SetPostProcessingFlag(PostProcessingBuffer::PostProcessingFlag::LUT_CROSSFADE, true);
	SceneBase::SetLutLerpFactor(0.0f);
}
