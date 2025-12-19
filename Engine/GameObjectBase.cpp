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

void GameObjectBase::MoveDirection(float distance, Direction direction)
{
	XMVECTOR directionVector = GetDirectionVector(direction);
	XMVECTOR deltaPosition = XMVectorScale(directionVector, distance);
	MovePosition(deltaPosition);
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

XMVECTOR GameObjectBase::GetDirectionVector(Direction direction)
{
	m_quaternion = XMQuaternionRotationRollPitchYawFromVector(m_euler);

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

void GameObjectBase::Initialize(SceneBase* scene)
{
	m_typeName = typeid(*this).name();
	if (m_typeName.find("class ") == 0) m_typeName = m_typeName.substr(6);

	m_scene = scene;

	m_worldWVPConstantBuffer = ResourceManager::GetInstance().GetConstantBuffer(sizeof(WorldBuffer));

	Begin();
}

void GameObjectBase::UpdateWorldMatrix()
{
	if (!m_isDirty) return;

	m_positionMatrix = XMMatrixTranslationFromVector(m_position);
	m_rotationMatrix = XMMatrixRotationRollPitchYawFromVector(m_euler);
	m_scaleMatrix = XMMatrixScalingFromVector(m_scale);

	m_worldMatrix = m_scaleMatrix * m_rotationMatrix * m_positionMatrix;

	XMVECTOR scaleSquared = XMVectorMultiply(m_scale, m_scale);
	XMVECTOR invScaleSquared = XMVectorReciprocal(scaleSquared);
	m_inverseScaleSquareMatrix = XMMatrixScalingFromVector(invScaleSquared);

	// 카메라 컴포넌트가 있으면 뷰 행렬 갱신
	CameraComponent* cameraComponent = GetComponent<CameraComponent>();
	if (cameraComponent) cameraComponent->UpdateViewMatrix(m_position, XMVectorAdd(m_position, GetDirectionVector(Direction::Forward)), GetDirectionVector(Direction::Up));

	m_isDirty = false;
}

void GameObjectBase::Render(XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
	ModelComponent* model = GetComponent<ModelComponent>();
	if (!model) return;

	// 월드 및 WVP 행렬 상수 버퍼 업데이트 및 셰이더에 설정
	m_worldData.worldMatrix = XMMatrixTranspose(m_worldMatrix);
	m_worldData.normalMatrix = XMMatrixTranspose(m_worldMatrix * m_inverseScaleSquareMatrix);
	m_worldData.WVPMatrix = projectionMatrix * viewMatrix * m_worldData.worldMatrix;

	const com_ptr<ID3D11DeviceContext> deviceContext = Renderer::GetInstance().GetDeviceContext();
	deviceContext->UpdateSubresource(m_worldWVPConstantBuffer.Get(), 0, nullptr, &m_worldData, 0, 0);
	deviceContext->VSSetConstantBuffers(1, 1, m_worldWVPConstantBuffer.GetAddressOf());

	// 모델 렌더링
	model->Render();
}

void GameObjectBase::RenderImGui()
{
	if (ImGui::TreeNode(m_typeName.c_str()))
	{
		// Position (위치)
		if (ImGui::DragFloat3("Position", &m_position.m128_f32[0], 0.05f))  SetDirty();
		// Rotation (회전)
		if (ImGui::DragFloat3("Rotation", &m_euler.m128_f32[0], 0.01f)) SetDirty();
		// Scale (크기)
		if (ImGui::DragFloat3("Scale", &m_scale.m128_f32[0], 0.01f)) SetDirty();

		SerializeImGui();

		ImGui::Separator();
		ImGui::Text("Components:");
		for (auto& [typeIndex, component] : m_components) component->RenderImGui();

		ImGui::TreePop();
	}
}

void GameObjectBase::Finalize()
{
	End();

	for (auto& [typeIndex, component] : m_components) component->Finalize();
}