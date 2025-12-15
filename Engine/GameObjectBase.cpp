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
	m_quaternion = XMQuaternionRotationRollPitchYawFromVector(rotation);

	SetDirty();
}

XMVECTOR GameObjectBase::GetRotation() const
{
	float x = XMVectorGetX(m_quaternion);
	float y = XMVectorGetY(m_quaternion);
	float z = XMVectorGetZ(m_quaternion);
	float w = XMVectorGetW(m_quaternion);

	float sinPitch = 2.0f * (w * x + y * z);
	float cosPitch = 1.0f - 2.0f * (x * x + y * y);
	float pitch = atan2f(sinPitch, cosPitch);

	float sinYaw = 2.0f * (w * y - z * x);
	float yaw = 0.0f;
	if (fabsf(sinYaw) >= 1.0f) yaw = copysignf(XM_PIDIV2, sinYaw); // 짐벌락 터짐
	else yaw = asinf(sinYaw);

	float sinRoll = 2.0f * (w * z + x * y);
	float cosRoll = 1.0f - 2.0f * (y * y + z * z);
	float roll = atan2f(sinRoll, cosRoll);

	return XMVectorSet(pitch, yaw, roll, 0.0f);
}

void GameObjectBase::Rotate(const XMVECTOR& deltaRotation)
{
	XMVECTOR deltaQuaternion = XMQuaternionRotationRollPitchYawFromVector(deltaRotation);
	m_quaternion = XMQuaternionMultiply(m_quaternion, deltaQuaternion);

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
	m_quaternion = XMQuaternionRotationMatrix(lookAtMatrix);

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
		#ifdef _DEBUG
		cerr << "GameObjectBase::GetDirectionVector: 알 수 없는 방향입니다." << endl;
		#else
		MessageBoxA(nullptr, "GameObjectBase::GetDirectionVector: 알 수 없는 방향입니다.", "오류", MB_OK | MB_ICONERROR);
		#endif
		return XMVectorZero();
	}
}

void GameObjectBase::Scale(const XMFLOAT3& deltaScale)
{
	m_scale.x *= deltaScale.x;
	m_scale.y *= deltaScale.y;
	m_scale.z *= deltaScale.z;

	SetDirty();
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
	m_scaleMatrix = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

	m_worldMatrix = m_scaleMatrix * m_rotationMatrix * m_positionMatrix;

	// 카메라 컴포넌트가 있으면 뷰 행렬 갱신
	CameraComponent* cameraComponent = GetComponent<CameraComponent>();
	if (cameraComponent) cameraComponent->UpdateViewMatrix(m_position, XMVectorAdd(m_position, GetDirectionVector(Direction::Forward)), GetDirectionVector(Direction::Up));

	m_isDirty = false;
}

void GameObjectBase::Render(XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
	ModelComponent* model = GetComponent<ModelComponent>();
	if (!model) return;

	const com_ptr<ID3D11DeviceContext> deviceContext = Renderer::GetInstance().GetDeviceContext();

	m_worldWVPData.worldMatrix = XMMatrixTranspose(m_worldMatrix);
	m_worldWVPData.WVPMatrix = projectionMatrix * viewMatrix * m_worldWVPData.worldMatrix;
	deviceContext->UpdateSubresource(m_worldWVPConstantBuffer.Get(), 0, nullptr, &m_worldWVPData, 0, 0);
	deviceContext->VSSetConstantBuffers(1, 1, m_worldWVPConstantBuffer.GetAddressOf());

	model->Render();
}

void GameObjectBase::Finalize()
{
	End();

	for (auto& [typeIndex, component] : m_components) component->Finalize();
}