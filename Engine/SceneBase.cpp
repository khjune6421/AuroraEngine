#include "stdafx.h"
#include "SceneBase.h"

#include "CameraComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"

using namespace std;
using namespace DirectX;

void SceneBase::Initialize()
{
	m_typeName = typeid(*this).name();
	if (m_typeName.find("class ") == 0) m_typeName = m_typeName.substr(6);

	ResourceManager& resourceManager = ResourceManager::GetInstance();
	m_viewProjectionConstantBuffer = resourceManager.GetConstantBuffer(sizeof(ViewProjectionBuffer)); // 뷰-투영 상수 버퍼 생성
	m_directionalLightConstantBuffer = resourceManager.GetConstantBuffer(sizeof(DirectionalLightBuffer)); // 방향광 상수 버퍼 생성
	m_environmentMapSRV = resourceManager.GetTexture(m_environmentMapFileName); // 환경 맵 로드

	m_mainCamera = CreateCameraObject()->CreateComponent<CameraComponent>();

	InitializeScene();
}

void SceneBase::Update(float deltaTime)
{
	RemovePendingGameObjects();
	for (unique_ptr<GameObjectBase>& gameObject : m_gameObjects) gameObject->Update(deltaTime);
}

void SceneBase::Render()
{
	Renderer& renderer = Renderer::GetInstance();
	renderer.BeginFrame(m_sceneColor);

	com_ptr<ID3D11DeviceContext> deviceContext = renderer.GetDeviceContext();

	// 뷰-투영 상수 버퍼 업데이트 및 셰이더에 설정
	m_viewProjectionData.viewMatrix = XMMatrixTranspose(m_mainCamera->GetViewMatrix());
	m_viewProjectionData.projectionMatrix = XMMatrixTranspose(m_mainCamera->GetProjectionMatrix());
	m_viewProjectionData.VPMatrix = m_viewProjectionData.projectionMatrix * m_viewProjectionData.viewMatrix;
	deviceContext->UpdateSubresource(m_viewProjectionConstantBuffer.Get(), 0, nullptr, &m_viewProjectionData, 0, 0);
	deviceContext->VSSetConstantBuffers(static_cast<UINT>(VSConstBuffers::ViewProjection), 1, m_viewProjectionConstantBuffer.GetAddressOf());

	// 방향광 상수 버퍼 업데이트 및 셰이더에 설정
	m_directionalLightData.lightDirection = -XMVector3Normalize(m_directionalLightDirection);
	m_directionalLightData.lightColor = m_sceneColor;
	deviceContext->UpdateSubresource(m_directionalLightConstantBuffer.Get(), 0, nullptr, &m_directionalLightData, 0, 0);
	deviceContext->PSSetConstantBuffers(static_cast<UINT>(PSConstBuffers::DirectionalLight), 1, m_directionalLightConstantBuffer.GetAddressOf());

	// 샘플러 상태 설정
	deviceContext->PSSetSamplers(static_cast<UINT>(SamplerState::Default), 1, ResourceManager::GetInstance().GetSamplerState(SamplerState::Default).GetAddressOf());

	// 환경 맵 설정
	deviceContext->PSSetShaderResources(static_cast<UINT>(TextureSlots::Environment), 1, m_environmentMapSRV.GetAddressOf());

	// 게임 오브젝트 렌더링
	for (unique_ptr<GameObjectBase>& gameObject : m_gameObjects) gameObject->Render();

	// ImGui 렌더링
	RenderImGui();

	renderer.EndFrame();
}

void SceneBase::RenderImGui()
{
	ImGui::Begin(m_typeName.c_str());

	if (ImGui::DragFloat3("Directional Light Direction", &m_directionalLightDirection.m128_f32[0], 0.001f, -1.0f, 1.0f)) {}
	if (ImGui::ColorEdit3("Scene Color", &m_sceneColor.x)) {}

	ImGui::Separator();
	ImGui::Text("Game Objects:");
	for (unique_ptr<GameObjectBase>& gameObject : m_gameObjects) gameObject->RenderImGui();

	ImGui::End();
}

GameObjectBase* SceneBase::CreateCameraObject()
{
	GameObjectBase* cameraGameObject = CreateRootGameObject<GameObjectBase>();
	cameraGameObject->SetPosition({ 0.0f, 5.0f, -10.0f });
	cameraGameObject->LookAt({ 0.0f, 0.0f, 0.0f });

	return cameraGameObject;
}

void SceneBase::RemovePendingGameObjects()
{
	for (GameObjectBase* gameObjectToRemove : m_gameObjectsToRemove)
	{
		erase_if
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