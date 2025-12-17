#include "stdafx.h"
#include "GameObjectBase.h"

#include "CameraComponent.h"
#include "ModelComponent.h"
#include "Renderer.h"
#include "RenderResourceManager.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

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

void GameObjectBase::SetRotation(const Vector3& rotation)
{
	// 오일러 각도 저장
	m_euler = rotation;
	// 쿼터니언 업데이트
	m_quaternion = Quaternion::CreateFromYawPitchRoll(m_euler.y, m_euler.x, m_euler.z);

	SetDirty();
}

void GameObjectBase::Rotate(const Vector3& deltaRotation)
{
	// 오일러 각도 업데이트
	m_euler += deltaRotation;

	// 쿼터니언 업데이트
	m_quaternion = Quaternion::CreateFromYawPitchRoll(m_euler.y, m_euler.x, m_euler.z);

	SetDirty();
}

void GameObjectBase::LookAt(const XMVECTOR& targetPosition)
{
	XMVECTOR direction = XMVector3Normalize(XMVectorSubtract(targetPosition, m_position));
	XMVECTOR right = XMVector3Normalize(XMVector3Cross(GetDirectionVector(Direction::Up), direction));
	XMVECTOR up = XMVector3Cross(direction, right);

	XMMATRIX lookAtMatrix = XMMATRIX
	(
		right,
		up,
		direction,
		XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f)
	);

	m_quaternion = Quaternion::CreateFromRotationMatrix(lookAtMatrix);
	m_euler = m_quaternion.ToEuler();

	SetDirty();
}

Vector3 GameObjectBase::GetDirectionVector(Direction direction) const
{
	switch (direction)
	{
	case Direction::Forward:
		return Vector3::Transform({ 0.0f, 0.0f, 1.0f }, m_quaternion);
	case Direction::Backward:
		return Vector3::Transform({ 0.0f, 0.0f, -1.0f }, m_quaternion);
	case Direction::Left:
		return Vector3::Transform({ -1.0f, 0.0f, 0.0f }, m_quaternion);
	case Direction::Right:
		return Vector3::Transform({ 1.0f, 0.0f, 0.0 }, m_quaternion);
	case Direction::Up:
		return Vector3::Transform({ 0.0f, 1.0f, 0.0f }, m_quaternion);
	case Direction::Down:
		return Vector3::Transform({ 0.0f, -1.0f, 0.0f }, m_quaternion);
	default:
		return Vector3{ 0.0f, 0.0f, 0.0f };
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
	m_rotationMatrix = Matrix::CreateFromQuaternion(m_quaternion);
	m_scaleMatrix = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

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
		XMFLOAT3 position = {};
		XMStoreFloat3(&position, m_position);
		if (ImGui::DragFloat3("Position", &position.x, 0.1f))
		{
			SetPosition(XMLoadFloat3(&position));
		}

		// Rotation (회전 - 라디안을 도(degree)로 변환하여 표시)
		Vector3 rotationDegrees =
		{
			XMConvertToDegrees(m_euler.x),
			XMConvertToDegrees(m_euler.y),
			XMConvertToDegrees(m_euler.z)
		};
		if (ImGui::DragFloat3("Rotation", &rotationDegrees.x, 0.5f))
		{
			Vector3 rotationRadians =
			{
				XMConvertToRadians(rotationDegrees.x),
				XMConvertToRadians(rotationDegrees.y),
				XMConvertToRadians(rotationDegrees.z)
			};
			SetRotation(rotationRadians);
		}

		// Scale (크기)
		if (ImGui::DragFloat3("Scale", &m_scale.x, 0.01f, 0.001f, 100.0f))
		{
			SetDirty();
		}

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