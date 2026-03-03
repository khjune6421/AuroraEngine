///BOF FSMComponentGun2.cpp
#include "stdafx.h"
#include "FSMComponentGun2.h"
#include "GameObjectBase.h"
#include "TimeManager.h"
#include "Player.h"

REGISTER_TYPE(FSMComponentGun2)

using namespace std;
using namespace DirectX;

void FSMComponentGun2::Initialize()
{
	gun = GetOwner();
	body = gun->GetChildGameObject("body"); if (body == nullptr) LOG("[FSMComponentGun2] gun need body \n");
	cylinder = gun->GetChildGameObject("cylinder"); if (cylinder == nullptr) LOG("[FSMComponentGun2] gun need cylinder \n");
	pin = gun->GetChildGameObject("pin"); if (pin == nullptr) LOG("[FSMComponentGun2] gun need pin \n");

}

std::string FSMComponentGun2::StateToString(StateID state) const
{
	switch (state)
	{
	case EIdle:   return "Idle";
	case EAttack: return "Attack";
	case EReload: return "Reload";
	default:                
		return "Unknown";
	}
}

void FSMComponentGun2::Fire()
{
	if (current_state_ == EReload)
	{
		//std::cout << "[GunFSM] Fire blocked: current_state=Reload\n";
		return;
	}

	//std::cout << "[GunFSM] Fire -> ChangeState(EAttack)\n";
    ChangeState(EAttack);
}

void FSMComponentGun2::Reload()
{
	if (current_state_ == EReload) return;

	ChangeState(EReload);
}

FSMComponent::StateID FSMComponentGun2::StringToState(const std::string& str) const
{
	if (str == "Idle")   return EIdle;
	if (str == "Attack") return EAttack;
	if (str == "Reload") return EReload;
	return EIdle;
}

void FSMComponentGun2::OnEnterState(StateID state)
{
	m_timer = 0.0f;
	m_originRotGun = gun->GetRotation();
	if (cylinder) m_originRotCylinder = cylinder->GetRotation();
	if (pin)      m_originRotPin = pin->GetRotation();

	//std::cout << "[GunFSM] OnEnterState=" << StateToString(state) << "\n";

	switch (state)
	{
	case EIdle:
		break;

	case EAttack:
		m_targetCylinderAngleX = m_originRotCylinder.m128_f32[0] + 60.0f;
        break;

	case EReload:
		//m_originRotation = GetOwner()->GetRotation();
		break;
	}
}

void FSMComponentGun2::OnUpdateState(StateID state)
{
	const float dt = TimeManager::GetInstance().GetDeltaTime();

	switch (state)
	{
	case EIdle:
		break;

    case EAttack:
    {
        m_timer += dt;
        //std::cout << "[GunFSM] EAttack - (갔다가 돌아오기)\n";
        // ==========================================
        // 1. 반동 (Gun 전체) - 기존 로직 유지
        // ==========================================
        //constexpr float kRecoilPitch = -15.0f;
        //constexpr float kRecoilDuration = 0.2f; // 조금 늘림
        constexpr float kRecoilPitch = -15.0f;
        constexpr float kRecoilDuration = 0.2f; // 조금 늘림

        // 반동 로직 (갔다가 돌아오기)
        if (m_timer <= kRecoilDuration) {
            float t = m_timer / kRecoilDuration;
            float recoilVal = 0.0f;
            //std::cout << "[GunFSM] EAttack - (갔다가 돌아오기)\n";


            // 0.0 ~ 0.5 (들림), 0.5 ~ 1.0 (복귀)
            if (t <= 0.5f)
                recoilVal = XMVectorLerp(XMVectorSet(0, 0, 0, 0), XMVectorSet(kRecoilPitch, 0, 0, 0), t * 2.0f).m128_f32[0];
            else
                recoilVal = XMVectorLerp(XMVectorSet(kRecoilPitch, 0, 0, 0), XMVectorSet(0, 0, 0, 0), (t - 0.5f) * 2.0f).m128_f32[0];

            XMVECTOR currentRecoil = m_originRotGun;
            //currentRecoil.m128_f32[0] += recoilVal;
            currentRecoil.m128_f32[2] += recoilVal; // 하드코딩의 시간...
            gun->SetRotation(currentRecoil);
        } else {
            gun->SetRotation(m_originRotGun); // 반동 끝
            Player::SetShotEndForTutorial();
        }


        // ==========================================
        // 2. 실린더 회전 (X축 60도)
        // ==========================================
        if (cylinder) {
            //std::cout << "[GunFSM] EAttack - (실린더 회전)\n";

            // 실린더는 반동보다 조금 천천히 돌아가도 멋짐
            constexpr float kCylinderDuration = 0.15f;
            float t = std::min(m_timer / kCylinderDuration, 1.0f);

            // 현재(Origin) -> 목표(Origin + 60) 로 선형 보간
            XMVECTOR nextRot = m_originRotCylinder;
            // Lerp 수식: Start + (End - Start) * t  => Origin + 60 * t
            nextRot.m128_f32[0] = m_originRotCylinder.m128_f32[0] + (60.0f * t);

            cylinder->SetRotation(nextRot);
        }

        // ==========================================
        // 3. Pin (공이) 동작 (0 -> 45 -> 0)
        // ==========================================
        if (pin) {
            //std::cout << "[GunFSM] EAttack - (Pin)\n";

            // 공이는 아주 빠르게 움직여야 함
            constexpr float kPinDuration = 0.1f;
            float t = m_timer / kPinDuration;

            if (t <= 1.0f) {
                float angle = 0.0f;
                // 절반(0.5)까지는 45도로 감, 나머지 절반은 0도로 복귀
                if (t <= 0.5f)
                    angle = (t * 2.0f) * 45.0f; // 0 -> 45
                else
                    angle = 45.0f - ((t - 0.5f) * 2.0f * 45.0f); // 45 -> 0

                // Pin이 회전할 축(여기선 X축 가정)
                XMVECTOR pinRot = m_originRotPin;
                pinRot.m128_f32[0] += angle;
                pin->SetRotation(pinRot);
            } else {
                pin->SetRotation(m_originRotPin); // 원위치
            }
        }

        // 모든 애니메이션 종료 체크 (가장 긴 시간 기준)
        if (m_timer >= kRecoilDuration) {
            ChangeState(EIdle);
        }
    }
    break;

    case EReload:
    {
        // ==========================================
        // Reload 연출: 실린더 고속 회전 (Spinning)
        // ==========================================
        m_timer += dt;

        constexpr float kReloadDuration = 0.5f;  // <- 여기만 조작할것
        constexpr float kSpinSpeed = 360.0f / kReloadDuration;

        if (m_timer <= kReloadDuration) {
            // 1. 실린더 회전 (계속 누적)
            if (cylinder) {
                //std::cout << "[GunFSM] EReload - (cylinder)\n";

                XMVECTOR spinRot = m_originRotCylinder;
                // 시간 * 속도만큼 각도 더하기
                // EaseOut(점점 느려지게)을 넣고 싶으면 t를 조작하면 됨. 여기선 등속.
                float addedAngle = kSpinSpeed * m_timer;

                // X축 기준 회전
                spinRot.m128_f32[0] += addedAngle;
                cylinder->SetRotation(spinRot);
            }

            // 2. (선택사항) Gun Body 살짝 기울이기 (재장전 중임을 티내기)
            /*
            XMVECTOR tiltRot = m_originRotGun;
            tiltRot.m128_f32[2] += 30.0f; // Z축으로 30도 갸우뚱
            gun->SetRotation(tiltRot);
            */

            if (gun){
                //std::cout << "[GunFSM] EReload - (gun)\n";

                XMVECTOR spinRot = m_originRotGun;
                float addedAngle = kSpinSpeed * m_timer;
                spinRot.m128_f32[2] -= addedAngle;
                gun->SetRotation(spinRot);
            }

        } else {
            // 종료: 각도 정리
            // 실린더는 많이 돌아갔으므로, 360도로 나눈 나머지값 등으로 깔끔하게 처리하거나
            // 현재 각도를 새로운 Origin으로 삼아야 함.
            // 여기서는 단순히 마지막 상태 유지 후 Idle로 갑니다.

            // 만약 총을 기울였다면 원위치
            gun->SetRotation(m_originRotGun);

            //std::cout << "[GunFSM] Reload end -> ChangeState(EIdle)\n";
            ChangeState(EIdle);
        }
    }
    break;
    }
}

void FSMComponentGun2::OnExitState(StateID state)
{
    switch (state) {
    case EIdle:
        break;
    case EAttack:
        // 사격 상태가 도중에 끊기더라도, 각도를 강제로 기준점으로 스냅(Snap)
        if (gun) gun->SetRotation(m_originRotGun);
        if (pin) pin->SetRotation(m_originRotPin);
        if (cylinder) {
            // 실린더는 돌아가야 했던 최종 목표치(60도 더해진 값)로 맞춰줌
            XMVECTOR finalCyl = m_originRotCylinder;
            finalCyl.m128_f32[0] += 60.0f;
            cylinder->SetRotation(finalCyl);
        }
        break;
    case EReload:
        // 장전이 도중에 끊길 경우 총체 각도 원상복구
        if (gun) gun->SetRotation(m_originRotGun);
        break;
    }
}

#ifdef _DEBUG
void FSMComponentGun2::RenderImGui()
{
	if (ImGui::TreeNode("FSM Component Gun2"))
	{
		string currentName = StateToString(current_state_);
		ImGui::Text("Current State: %s", currentName.c_str());

		if (ImGui::BeginCombo("Force State", currentName.c_str()))
		{
			for (int i = 0; i < ECount; ++i)
			{
				EState state = (EState)i;
				string stateName = StateToString(state);
				bool isSelected = (current_state_ == state);
				if (ImGui::Selectable(stateName.c_str(), isSelected))
				{
					ChangeState(state);
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::TreePop();
	}
}
#endif


///EOF FSMComponentGun2.cpp
