#include "stdafx.h"
#include "SceneBase.h"

#include "GameObjectBase.h"
#include "Renderer.h"

using namespace std;
using namespace DirectX;

void SceneBase::Initialize()
{
	m_viewProjectionConstantBuffer = Renderer::GetInstance().GetConstantBuffer(sizeof(ViewProjectionBuffer));

	Begin();
}

void SceneBase::Update(float deltaTime)
{
	for (auto& gameObject : m_gameObjects) gameObject->Update(deltaTime);
}

void SceneBase::TransformGameObjects()
{
	for (auto& gameObject : m_gameObjects) gameObject->UpdateWorldMatrix();
}

void SceneBase::Render()
{
	Renderer& renderer = Renderer::GetInstance();
	com_ptr<ID3D11DeviceContext> deviceContext = renderer.GetDeviceContext();

	renderer.BeginFrame(m_clearColor);

	m_viewProjectionData.viewMatrix = XMMatrixTranspose(m_mainCamera.GetViewMatrix());
	m_viewProjectionData.projectionMatrix = XMMatrixTranspose(m_mainCamera.GetProjectionMatrix());
	deviceContext->UpdateSubresource(m_viewProjectionConstantBuffer.Get(), 0, nullptr, &m_viewProjectionData, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, m_viewProjectionConstantBuffer.GetAddressOf());

	for (auto& gameObject : m_gameObjects) gameObject->Render(m_viewProjectionData.projectionMatrix, m_viewProjectionData.projectionMatrix);

	renderer.EndFrame();
}

void SceneBase::AddGameObject(unique_ptr<GameObjectBase> gameObject)
{
	gameObject->Initialize();
	m_gameObjects.push_back(move(gameObject));
}