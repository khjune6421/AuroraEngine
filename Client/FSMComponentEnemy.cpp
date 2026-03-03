///BOF FSMComponentEnemy.cpp
#include "stdafx.h"
#include "FSMComponentEnemy.h"
#include "GameObjectBase.h"
#include "SkinnedModelComponent.h"
#include "TimeManager.h"
#include "../Engine/Animator.h"
#include "Enemy.h" 
#include "Player.h"
#include "SceneManager.h"
#include "HyojeTestScene.h"

REGISTER_TYPE(FSMComponentEnemy)

using namespace std;
using namespace DirectX;



void FSMComponentEnemy::Initialize()
{
	model_ = GetOwner()->GetComponent<SkinnedModelComponent>();
	owner_enemy_ = dynamic_cast<Enemy*>(GetOwner());
	if(player_ == nullptr) player_ = owner_enemy_->GetTargetPlayer();
	FSMComponent::Initialize();

}

std::string FSMComponentEnemy::StateToString(StateID state) const
{
	switch (state)
	{
	case EIdle:   return "Idle";
	case EChase:    return "Chase";
	case EAttack: return "Attack";
	case EDead:   return "Dead";
	default:     return "Unknown";
	}
}

FSMComponent::StateID FSMComponentEnemy::StringToState(const std::string& str) const
{
	if (str == "Idle")   return EIdle;
	if (str == "Chase")    return EChase;
	if (str == "Attack") return EAttack;
	if (str == "Dead")   return EDead;
	return EIdle;
}

void FSMComponentEnemy::OnEnterState(StateID state)
{
	if (!model_) return;
	if (!model_->GetAnimator()) return;

	switch (state)
	{
	case EIdle:
		model_->GetAnimator()->SetPlaybackSpeed(1.0f);
		model_->GetAnimator()->PlayAnimation(1);
		model_->SetBlendState(BlendState::Opaque);
		break;

	case EChase:
		model_->GetAnimator()->SetPlaybackSpeed(1.0f);
		model_->GetAnimator()->PlayAnimation(2, true);
		break;

	case EAttack: 
		attack_timer_ = 0.0f;
		attack_has_hit_ = false;

		model_->GetAnimator()->SetPlaybackSpeed(1.0f);
		//model_->GetAnimator()->RestartCurrentAnimation(false);
		model_->GetAnimator()->PlayAnimation(0, false);
		break;

	case EDead:
		death_timer_ = 0.0f;
		model_->GetAnimator()->SetPlaybackSpeed(0.1f);
		model_->GetAnimator()->PlayAnimation(0, false);

		model_->SetBlendState(BlendState::AlphaBlend);
		break;
	}

}

void FSMComponentEnemy::OnUpdateState(StateID state)
{
	float dt = TimeManager::GetInstance().GetDeltaTime();

	switch (state)
	{
	case EAttack:
		attack_timer_ += dt;
		
		if (attack_timer_ >= kAttackAnticipation && ! attack_has_hit_) {
			attack_has_hit_ = true;
		
			if (owner_enemy_&& player_) {
				XMVECTOR diff = player_->GetPosition() - owner_enemy_->GetPosition();
				float distSq = XMVectorGetX(XMVector3LengthSq(diff));

					if (distSq <= kAttackRange * kAttackRange) {
					LOG("Player Hit! Damage: " << kDamage);
					player_->TakeHit();

					}
				}
			}

		if (attack_timer_ >= kAttackTotalTime && owner_enemy_) {
			owner_enemy_->OnAttackFinished();
		}

		break;
	case EDead:
	{
		death_timer_ += dt;

		if (death_timer_ >= kFadeStartTime)
		{
			float progress = (death_timer_ - kFadeStartTime) / kFadeDuration;
			if (progress > 1.0f) progress = 1.0f;

			if (model_)
			{
				model_->SetAlpha(1.0f - progress);
				model_->SetDissolveThreshold(progress);
				model_->SetDissolveIntensity(progress*10.f);
				model_->SetDissolveEdgeWidth(progress*6.f);
			}
		}
	}
	break;
	}
}

void FSMComponentEnemy::OnExitState(StateID state)
{

}

#ifdef _DEBUG
void FSMComponentEnemy::RenderImGui()
{
	if (ImGui::TreeNode("FSM Component Enemy"))
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


///EOF FSMComponentEnemy.cpp
