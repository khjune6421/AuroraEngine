#include "stdafx.h"
#include "GameObjectBase.h"

#include "CameraComponent.h"
#include "ModelComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"

using namespace std;
using namespace DirectX;

GameObjectBase::GameObjectBase()
{
	static UINT idIndex = 0;
	m_id = idIndex++;
}

GameObjectBase::~GameObjectBase()
{
	for (auto& [typeIndex, component] : m_components) component->Finalize();
}

void GameObjectBase::Initialize()
{
	m_typeName = typeid(*this).name();
	if (m_typeName.find("class ") == 0) m_typeName = m_typeName.substr(6);
	m_typeName += "_" + to_string(m_id);

	m_worldWVPConstantBuffer = ResourceManager::GetInstance().GetConstantBuffer(sizeof(WorldBuffer));

	InitializeGameObject();
}

void GameObjectBase::Update(float deltaTime)
{
	// 게임 오브젝트 업데이트 // 파생 클래스에서 오버라이드
	UpdateGameObject(deltaTime);
	// 월드 행렬 업데이트
	UpdateWorldMatrix();
	// 컴포넌트 업데이트
	for (auto& [typeIndex, component] : m_components) component->Update(deltaTime);

	// 제거할 자식 게임 오브젝트 제거
	RemovePendingChildGameObjects();
	// 자식 게임 오브젝트 업데이트
	for (auto& child : m_childrens) child->Update(deltaTime);
}

void GameObjectBase::Render()
{
	ModelComponent* model = GetComponent<ModelComponent>();
	if (model)
	{
		// 월드 및 WVP 행렬 상수 버퍼 업데이트 및 셰이더에 설정
		m_worldData.worldMatrix = XMMatrixTranspose(m_worldMatrix);
		m_worldData.normalMatrix = XMMatrixTranspose(m_inverseScaleSquareMatrix * m_worldMatrix);

		const com_ptr<ID3D11DeviceContext> deviceContext = Renderer::GetInstance().GetDeviceContext();
		deviceContext->UpdateSubresource(m_worldWVPConstantBuffer.Get(), 0, nullptr, &m_worldData, 0, 0);
		deviceContext->VSSetConstantBuffers(static_cast<UINT>(VSConstBuffers::WorldNormal), 1, m_worldWVPConstantBuffer.GetAddressOf());

		// 모델 렌더링
		model->Render();
	}

	// 자식 게임 오브젝트 렌더링
	for (auto& child : m_childrens) child->Render();
}

void GameObjectBase::RenderImGui()
{
	if (ImGui::TreeNode(m_typeName.c_str()))
	{
		// 위치
		if (ImGui::DragFloat3("Position", &m_position.m128_f32[0], 0.05f))  SetDirty();
		// 회전
		if (ImGui::DragFloat3("Rotation", &m_euler.m128_f32[0], 0.5f))
		{
			m_quaternion = XMQuaternionRotationRollPitchYawFromVector(ToRadians(m_euler)); // 라디안으로 변환
			SetDirty();
		};
		// 크기
		if (ImGui::DragFloat3("Scale", &m_scale.m128_f32[0], 0.01f)) SetDirty();

		RenderImGuiGameObject();

		// 컴포넌트 ImGui 렌더링
		if (!m_components.empty())
		{
			ImGui::Separator();
			ImGui::Text("Components:");
			for (auto& [typeIndex, component] : m_components) component->RenderImGui();
		}

		// 자식 게임 오브젝트 ImGui 렌더링
		if (!m_childrens.empty())
		{
			ImGui::Separator();
			ImGui::Text("Children:");
			for (auto& child : m_childrens) child->RenderImGui();
		}

		ImGui::TreePop();
	}
}

void GameObjectBase::Finalize()
{
	FinalizeGameObject();

	// 컴포넌트 종료
	for (auto& [typeIndex, component] : m_components) component->Finalize();

	// 자식 게임 오브젝트 종료
	for (auto& child : m_childrens) child->Finalize();
}

void GameObjectBase::MoveDirection(float distance, Direction direction)
{
	XMVECTOR directionVector = GetDirectionVector(direction);
	XMVECTOR deltaPosition = XMVectorScale(directionVector, distance);

	MovePosition(deltaPosition);
}

void GameObjectBase::SetRotation(const XMVECTOR& rotation)
{
	m_euler = rotation;
	m_quaternion = XMQuaternionRotationRollPitchYawFromVector(ToRadians(m_euler)); // 라디안으로 변환

	SetDirty();
}

void GameObjectBase::Rotate(const XMVECTOR& deltaRotation)
{
	m_euler = XMVectorAdd(m_euler, deltaRotation);
	m_quaternion = XMQuaternionRotationRollPitchYawFromVector(ToRadians(m_euler)); // 라디안으로 변환

	SetDirty();
}

void GameObjectBase::LookAt(const XMVECTOR& targetPosition)
{
	XMVECTOR direction = XMVector3Normalize(XMVectorSubtract(targetPosition, UpdateWorldMatrix().r[3]));
	XMVECTOR right = XMVector3Cross(GetDirectionVector(Direction::Up), direction);
	XMVECTOR up = XMVector3Cross(direction, right);

	m_quaternion = XMQuaternionRotationMatrix({ right, up, direction, { 0.0f, 0.0f, 0.0f, 1.0f } });
	m_euler = ToDegrees(static_cast<XMVECTOR>(static_cast<SimpleMath::Quaternion>(m_quaternion).ToEuler())); // 도 단위로 변환

	SetDirty();
}

XMVECTOR GameObjectBase::GetDirectionVector(Direction direction) const
{
	switch (direction)
	{
	case Direction::Forward:
		return XMVector3Rotate(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), m_quaternion);
	case Direction::Backward:
		return XMVector3Rotate(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), m_quaternion);

	case Direction::Left:
		return XMVector3Rotate(XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f), m_quaternion);
	case Direction::Right:
		return XMVector3Rotate(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), m_quaternion);

	case Direction::Up:
		return XMVector3Rotate(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), m_quaternion);
	case Direction::Down:
		return XMVector3Rotate(XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), m_quaternion);

	default:
		return XMVectorZero();
	}
}

void GameObjectBase::SetDirty()
{
	m_isDirty = true;
	for (auto& child : m_childrens) child->SetDirty();
}

void GameObjectBase::RemovePendingChildGameObjects()
{
	for (GameObjectBase* childToRemove : m_childrenToRemove)
	{
		erase_if
		(
			m_childrens, [childToRemove](const unique_ptr<GameObjectBase>& obj)
			{
				if (obj.get() == childToRemove)
				{
					obj->Finalize();
					return true;
				}
				return false;
			}
		);
	}
	m_childrenToRemove.clear();
}

const XMMATRIX& GameObjectBase::UpdateWorldMatrix()
{
	if (m_isDirty)
	{
		m_positionMatrix = XMMatrixTranslationFromVector(m_position);
		m_rotationMatrix = XMMatrixRotationQuaternion(m_quaternion);
		m_scaleMatrix = XMMatrixScalingFromVector(m_scale);

		m_worldMatrix = m_scaleMatrix * m_rotationMatrix * m_positionMatrix;

		XMVECTOR scaleSquared = XMVectorMultiply(m_scale, m_scale);
		XMVECTOR invScaleSquared = XMVectorReciprocal(scaleSquared);
		m_inverseScaleSquareMatrix = XMMatrixScalingFromVector(invScaleSquared);

		if (m_parent)
		{
			m_worldMatrix *= m_parent->UpdateWorldMatrix();
			m_inverseScaleSquareMatrix *= m_parent->m_inverseScaleSquareMatrix;
		}

		m_isDirty = false;
	}

	return m_worldMatrix;
}