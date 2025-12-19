#include "stdafx.h"
#include "SceneBase.h"

#include "CameraComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"

using namespace std;
using namespace DirectX;

GameObjectBase* SceneBase::CreateCameraObject()
{
	GameObjectBase* cameraGameObject = CreateGameObject<GameObjectBase>();
	cameraGameObject->SetPosition({ 0.0f, 5.0f, -10.0f });
	cameraGameObject->LookAt({ 0.0f, 0.0f, 0.0f });

	return cameraGameObject;
}

void SceneBase::Initialize()
{
	m_typeName = typeid(*this).name();
	if (m_typeName.find("class ") == 0) m_typeName = m_typeName.substr(6);

	m_viewProjectionConstantBuffer = ResourceManager::GetInstance().GetConstantBuffer(sizeof(ViewProjectionBuffer));
	m_mainCamera = CreateCameraObject()->AddComponent<CameraComponent>();

	Begin();
}

void SceneBase::Update(float deltaTime)
{
	for (unique_ptr<GameObjectBase>& gameObject : m_gameObjects) gameObject->Update(deltaTime);
}

void SceneBase::RemovePendingGameObjects()
{
	for (GameObjectBase* gameObjectToRemove : m_gameObjectsToRemove)
	{
		std::erase_if
		(
			m_gameObjects, [gameObjectToRemove](const unique_ptr<GameObjectBase>& obj)
			{
				if (obj.get() == gameObjectToRemove)
				{
					obj->Finalize();
					return true;
				}
			return false;
			}
		);
	}
	m_gameObjectsToRemove.clear();
}

void SceneBase::TransformGameObjects()
{
	for (unique_ptr<GameObjectBase>& gameObject : m_gameObjects) gameObject->UpdateWorldMatrix();
}

void SceneBase::Render()
{
	Renderer& renderer = Renderer::GetInstance();
	renderer.BeginFrame(m_clearColor);

	// 뷰-투영 상수 버퍼 업데이트 및 셰이더에 설정
	m_viewProjectionData.viewMatrix = XMMatrixTranspose(m_mainCamera->GetViewMatrix());
	m_viewProjectionData.projectionMatrix = XMMatrixTranspose(m_mainCamera->GetProjectionMatrix());

	com_ptr<ID3D11DeviceContext> deviceContext = renderer.GetDeviceContext();
	deviceContext->UpdateSubresource(m_viewProjectionConstantBuffer.Get(), 0, nullptr, &m_viewProjectionData, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, m_viewProjectionConstantBuffer.GetAddressOf());

	// 게임 오브젝트 렌더링
	for (unique_ptr<GameObjectBase>& gameObject : m_gameObjects) gameObject->Render(m_viewProjectionData.viewMatrix, m_viewProjectionData.projectionMatrix);

	// ImGui 렌더링
	RenderImGui();

	renderer.EndFrame();
}

void SceneBase::RenderImGui()
{
	ImGui::Begin(m_typeName.c_str());

	if (ImGui::ColorEdit3("Clear Color", m_clearColor.data())) {}

	ImGui::Separator();
	ImGui::Text("Game Objects:");
	for (unique_ptr<GameObjectBase>& gameObject : m_gameObjects) gameObject->RenderImGui();

	ImGui::End();
}

void SceneBase::Finalize()
{
	End();
}