///SceneBase.cpp의 시작
#include "stdafx.h"
#include "SceneBase.h"

#include "CameraComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"

using namespace std;
using namespace DirectX;

SceneBase::SceneBase()
{
	m_deviceContext = Renderer::GetInstance().GetDeviceContext();
}

GameObjectBase* SceneBase::CreateCameraObject()
{
	GameObjectBase* cameraGameObject = CreateRootGameObject<GameObjectBase>();
	cameraGameObject->SetPosition({ 0.0f, 5.0f, -10.0f });
	cameraGameObject->LookAt({ 0.0f, 0.0f, 0.0f });

	return cameraGameObject;
}

void SceneBase::BaseInitialize()
{
	m_typeName = typeid(*this).name();
	if (m_typeName.find("class ") == 0) m_typeName = m_typeName.substr(6);

	GetResources();

	m_mainCamera = CreateCameraObject()->CreateComponent<CameraComponent>();

	Initialize();
}

void SceneBase::BaseUpdate()
{
	RemovePendingGameObjects();
	for (unique_ptr<IBase>& gameObject : m_gameObjects) gameObject->BaseUpdate();
}

void SceneBase::BaseRender()
{
	// 상수 버퍼 업데이트 및 셰이더에 설정
	UpdateConstantBuffers();

	// 샘플러 상태 설정
	m_deviceContext->PSSetSamplers(static_cast<UINT>(SamplerState::Default), 1, ResourceManager::GetInstance().GetSamplerState(SamplerState::Default).GetAddressOf());

	// 환경 맵 설정
	m_deviceContext->PSSetShaderResources(static_cast<UINT>(TextureSlots::Environment), 1, m_environmentMapSRV.GetAddressOf());

	// 게임 오브젝트 렌더링
	for (unique_ptr<IBase>& gameObject : m_gameObjects) gameObject->BaseRender();

	// 스카이박스 렌더링
	RenderSkybox();
}

void SceneBase::BaseRenderImGui()
{
	ImGui::Begin(m_typeName.c_str());

	if (ImGui::DragFloat3("Directional Light Direction", &m_directionalLightDirection.m128_f32[0], 0.001f, -1.0f, 1.0f)) {}
	if (ImGui::ColorEdit3("Scene Color", &m_sceneColor.x)) {}

	ImGui::Separator();
	ImGui::Text("Game Objects:");
	for (unique_ptr<IBase>& gameObject : m_gameObjects) gameObject->BaseRenderImGui();

	ImGui::End();
}

void SceneBase::GetResources()
{
	ResourceManager& resourceManager = ResourceManager::GetInstance();

	m_viewProjectionConstantBuffer = resourceManager.GetConstantBuffer(sizeof(ViewProjectionBuffer)); // 뷰-투영 상수 버퍼 생성
	m_cameraPositionConstantBuffer = resourceManager.GetConstantBuffer(sizeof(XMVECTOR)); // 카메라 위치 상수 버퍼 생성
	m_directionalLightConstantBuffer = resourceManager.GetConstantBuffer(sizeof(DirectionalLightBuffer)); // 방향광 상수 버퍼 생성

	m_environmentMapSRV = resourceManager.GetTexture(m_environmentMapFileName); // 환경 맵 로드

	m_skyboxVertexShaderAndInputLayout = resourceManager.GetVertexShaderAndInputLayout("VSSkybox.hlsl"); // 스카이박스 정점 셰이더 얻기
	m_skyboxPixelShader = resourceManager.GetPixelShader("PSSkybox.hlsl"); // 스카이박스 픽셀 셰이더 얻기

	m_skyboxDepthStencilState = resourceManager.GetDepthStencilState(DepthStencilState::Skybox); // 스카이박스 깊이버퍼 상태 얻기
}

void SceneBase::UpdateConstantBuffers()
{
	// 뷰-투영 상수 버퍼 업데이트 및 셰이더에 설정
	m_viewProjectionData.viewMatrix = XMMatrixTranspose(m_mainCamera->GetViewMatrix());
	m_viewProjectionData.projectionMatrix = XMMatrixTranspose(m_mainCamera->GetProjectionMatrix());
	m_viewProjectionData.VPMatrix = m_viewProjectionData.projectionMatrix * m_viewProjectionData.viewMatrix;
	m_deviceContext->UpdateSubresource(m_viewProjectionConstantBuffer.Get(), 0, nullptr, &m_viewProjectionData, 0, 0);
	m_deviceContext->VSSetConstantBuffers(static_cast<UINT>(VSConstBuffers::ViewProjection), 1, m_viewProjectionConstantBuffer.GetAddressOf());

	// 카메라 위치 상수 버퍼 업데이트 및 셰이더에 설정
	m_deviceContext->UpdateSubresource(m_cameraPositionConstantBuffer.Get(), 0, nullptr, &m_mainCamera->GetPosition(), 0, 0);
	m_deviceContext->PSSetConstantBuffers(static_cast<UINT>(PSConstBuffers::CameraPosition), 1, m_cameraPositionConstantBuffer.GetAddressOf());

	// 방향광 상수 버퍼 업데이트 및 셰이더에 설정
	m_directionalLightData.lightDirection = -XMVector3Normalize(m_directionalLightDirection);
	m_directionalLightData.lightColor = m_sceneColor;
	m_deviceContext->UpdateSubresource(m_directionalLightConstantBuffer.Get(), 0, nullptr, &m_directionalLightData, 0, 0);
	m_deviceContext->PSSetConstantBuffers(static_cast<UINT>(PSConstBuffers::DirectionalLight), 1, m_directionalLightConstantBuffer.GetAddressOf());
}

void SceneBase::RenderSkybox()
{
	m_deviceContext->IASetInputLayout(m_skyboxVertexShaderAndInputLayout.second.Get());
	m_deviceContext->VSSetShader(m_skyboxVertexShaderAndInputLayout.first.Get(), nullptr, 0);
	m_deviceContext->PSSetShader(m_skyboxPixelShader.Get(), nullptr, 0);

	m_deviceContext->OMSetDepthStencilState(m_skyboxDepthStencilState.Get(), 0);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	constexpr UINT stride = 0;
	constexpr UINT offset = 0;
	constexpr ID3D11Buffer* nullBuffer = nullptr;
	m_deviceContext->IASetVertexBuffers(0, 1, &nullBuffer, &stride, &offset);

	m_deviceContext->Draw(3, 0);

	m_deviceContext->OMSetDepthStencilState(nullptr, 0);
}

void SceneBase::RemovePendingGameObjects()
{
	for (IBase* gameObjectToRemove : m_gameObjectsToRemove)
	{
		erase_if
		(
			m_gameObjects, [gameObjectToRemove](const unique_ptr<IBase>& obj)
			{
				if (obj.get() == gameObjectToRemove)
				{
					obj->BaseFinalize();
					return true;
				}
				return false;
			}
		);
	}
	m_gameObjectsToRemove.clear();
}
///SceneBase.cpp의 끝