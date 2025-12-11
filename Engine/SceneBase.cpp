#include "stdafx.h"
#include "SceneBase.h"

#include "Renderer.h"

using namespace std;
using namespace DirectX;

void SceneBase::Initialize()
{
	m_viewProjectionConstantBuffer = Renderer::GetInstance().GetConstantBuffer(sizeof(ViewProjectionBuffer));
	m_mainCamera = CreateCameraObject()->AddComponent<CameraComponent>();

	Begin();
}

void SceneBase::Update(float deltaTime)
{
	for (unique_ptr<GameObjectBase>& gameObject : m_gameObjects) gameObject->Update(deltaTime);
}

void SceneBase::TransformGameObjects()
{
	for (unique_ptr<GameObjectBase>& gameObject : m_gameObjects) gameObject->UpdateWorldMatrix();
}

void SceneBase::Render()
{
	Renderer& renderer = Renderer::GetInstance();
	com_ptr<ID3D11DeviceContext> deviceContext = renderer.GetDeviceContext();

	renderer.BeginFrame(m_clearColor);

	m_viewProjectionData.viewMatrix = XMMatrixTranspose(m_mainCamera->GetViewMatrix());
	m_viewProjectionData.projectionMatrix = XMMatrixTranspose(m_mainCamera->GetProjectionMatrix());
	deviceContext->UpdateSubresource(m_viewProjectionConstantBuffer.Get(), 0, nullptr, &m_viewProjectionData, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, m_viewProjectionConstantBuffer.GetAddressOf());

	for (unique_ptr<GameObjectBase>& gameObject : m_gameObjects) gameObject->Render(m_viewProjectionData.viewMatrix, m_viewProjectionData.projectionMatrix);

	renderer.EndFrame();
}

GameObjectBase* SceneBase::CreateCameraObject()
{
	unique_ptr<GameObjectBase> cameraGameObject = make_unique<GameObjectBase>();
	cameraGameObject->SetPosition({ 0.0f, 5.0f, -10.0f, 1.0f });
	cameraGameObject->LookAt({ 0.0f, 0.0f, 0.0f, 1.0f });

	return AddGameObject(move(cameraGameObject));
}

GameObjectBase* SceneBase::AddGameObject(unique_ptr<GameObjectBase> gameObject)
{
	gameObject->Initialize();
	GameObjectBase* gameObjectPtr = gameObject.get();
	m_gameObjects.push_back(move(gameObject));

	return gameObjectPtr;
}