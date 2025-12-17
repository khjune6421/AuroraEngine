#include "stdafx.h"
#include "GameObjectBase.h"

#include "CameraComponent.h"
#include "ModelComponent.h"
#include "Renderer.h"
#include "RenderResourceManager.h"

using namespace std;
using namespace DirectX;

GameObjectBase::GameObjectBase()
{
	static UINT idIndex = 0;
	m_id = idIndex++;
}

void GameObjectBase::MoveDirection(float distance, Direction direction)
{
	XMVECTOR directionVector = GetDirectionVector(direction);
	XMVECTOR deltaPosition = XMVectorScale(directionVector, distance);
	MovePosition(deltaPosition);
}

void GameObjectBase::SetRotation(const XMVECTOR& rotation)
{
	// 오일러 각도 저장
	m_euler = rotation;
	// 쿼터니언 업데이트
	m_quaternion = XMQuaternionRotationRollPitchYawFromVector(m_euler);

	SetDirty();
}

void GameObjectBase::Rotate(const XMVECTOR& deltaRotation)
{
	// 오일러 각도 업데이트
	m_euler += deltaRotation;
	// 쿼터니언 업데이트
	m_quaternion = XMQuaternionRotationRollPitchYawFromVector(m_euler);

	SetDirty();
}

void GameObjectBase::LookAt(const XMVECTOR& targetPosition)
{
	XMVECTOR direction = XMVector3Normalize(XMVectorSubtract(targetPosition, m_position));
	XMVECTOR right = XMVector3Cross(GetDirectionVector(Direction::Up), direction);
	XMVECTOR up = XMVector3Cross(direction, right);

	m_quaternion = XMQuaternionRotationMatrix({ right, up, direction, { 0.0f, 0.0f, 0.0f, 1.0f } });
	m_euler = static_cast<XMVECTOR>(static_cast<SimpleMath::Quaternion>(m_quaternion).ToEuler());

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

void GameObjectBase::Initialize()
{
	m_worldWVPConstantBuffer = RenderResourceManager::GetInstance().GetConstantBuffer(sizeof(WorldWVPBuffer));

	Begin();
}

void GameObjectBase::UpdateWorldMatrix()
{
	if (!m_isDirty) return;

	m_positionMatrix = XMMatrixTranslationFromVector(m_position);
	m_rotationMatrix = XMMatrixRotationQuaternion(m_quaternion);
	m_scaleMatrix = XMMatrixScalingFromVector(m_scale);

	m_worldMatrix = m_scaleMatrix * m_rotationMatrix * m_positionMatrix;

	// 카메라 컴포넌트가 있으면 뷰 행렬 갱신
	CameraComponent* cameraComponent = GetComponent<CameraComponent>();
	if (cameraComponent) cameraComponent->UpdateViewMatrix(m_position, XMVectorAdd(m_position, GetDirectionVector(Direction::Forward)), GetDirectionVector(Direction::Up));

	m_isDirty = false;
}

void GameObjectBase::Render(XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
	if (ImGui::TreeNode((void*)(intptr_t)m_id, "GameObject #%u", m_id))
	{
		// Position (위치)
		if (ImGui::DragFloat3("Position", &m_position.m128_f32[0], 0.1f))  SetDirty();

		// Rotation (회전 - 라디안을 도(degree)로 변환하여 표시)
		XMFLOAT3 rotationDegrees = {};
		XMStoreFloat3(&rotationDegrees, XMVectorScale(m_euler, 180.0f / XM_PI));
		if (ImGui::DragFloat3("Rotation", &rotationDegrees.x, 0.5f))
		{
			XMVECTOR rotationRadians = XMVectorScale(XMLoadFloat3(&rotationDegrees), XM_PI / 180.0f);
			SetRotation(rotationRadians);
		}

		// Scale (크기)
		if (ImGui::DragFloat3("Scale", &m_scale.m128_f32[0], 0.1f)) SetDirty();

		ImGui::TreePop();
	}

	ModelComponent* model = GetComponent<ModelComponent>();
	if (!model) return;

	// 월드 및 WVP 행렬 상수 버퍼 업데이트 및 셰이더에 설정
	const com_ptr<ID3D11DeviceContext> deviceContext = Renderer::GetInstance().GetDeviceContext();
	m_worldWVPData.worldMatrix = XMMatrixTranspose(m_worldMatrix);
	m_worldWVPData.WVPMatrix = projectionMatrix * viewMatrix * m_worldWVPData.worldMatrix;
	deviceContext->UpdateSubresource(m_worldWVPConstantBuffer.Get(), 0, nullptr, &m_worldWVPData, 0, 0);
	deviceContext->VSSetConstantBuffers(1, 1, m_worldWVPConstantBuffer.GetAddressOf());

	// 모델 렌더링
	model->Render();
}

void GameObjectBase::Finalize()
{
	End();

	for (auto& [typeIndex, component] : m_components) component->Finalize();
}