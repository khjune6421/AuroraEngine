#include "stdafx.h"
#include "GameObjectBase.h"

#include "Renderer.h"
#include "ModelComponent.h"

using namespace std;
using namespace DirectX;

static UINT s_idIndex = 0;

GameObjectBase::GameObjectBase() { m_id = s_idIndex++; }

void GameObjectBase::Initialize()
{
	m_worldWVPConstantBuffer = Renderer::GetInstance().GetConstantBuffer(sizeof(WorldWVPBuffer));

	Begin();
}

void GameObjectBase::UpdateWorldMatrix()
{
	if (!m_isDirty) return;

	m_positionMatrix = XMMatrixTranslationFromVector(m_position);
	m_rotationMatrix = XMMatrixRotationRollPitchYawFromVector(m_rotation);
	m_scaleMatrix = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

	m_worldMatrix = m_scaleMatrix * m_rotationMatrix * m_positionMatrix;

	m_isDirty = false;
}

void GameObjectBase::Render(XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
	auto model = GetComponent<ModelComponent>();
	if (!model) return;

	const com_ptr<ID3D11DeviceContext> deviceContext = Renderer::GetInstance().GetDeviceContext();

	m_worldWVPData.worldMatrix = XMMatrixTranspose(m_worldMatrix);
	m_worldWVPData.WVPMatrix = projectionMatrix * viewMatrix * m_worldWVPData.worldMatrix;
	deviceContext->UpdateSubresource(m_worldWVPConstantBuffer.Get(), 0, nullptr, &m_worldWVPData, 0, 0);
	deviceContext->VSSetConstantBuffers(1, 1, m_worldWVPConstantBuffer.GetAddressOf());

	model->Render();
}