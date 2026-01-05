///SceneBase.cpp의 시작
#include "stdafx.h"
#include "SceneBase.h"

#include "CameraComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"

#ifdef _DEBUG
#include "InputManager.h"
#endif

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

void SceneBase::CreateRootGameObject(string typeName)
{
	unique_ptr<Base> gameObjectPtr = TypeRegistry::GetInstance().CreateGameObject(typeName);

	gameObjectPtr->BaseInitialize();

	m_gameObjects.push_back(move(gameObjectPtr));
}

void SceneBase::BaseInitialize()
{
	m_type = GetTypeName(*this);

	// 저장된 씬 파일 불러오기
	const filesystem::path sceneFilePath = "../Asset/Scene/" + m_type + ".json";
	if (filesystem::exists(sceneFilePath))
	{
		ifstream file(sceneFilePath);
		nlohmann::json sceneData;
		file >> sceneData;
		file.close();
		BaseDeserialize(sceneData);
	}

	GetResources();

	#ifdef _DEBUG
	m_debugCamera = make_unique<DebugCamera>();
	static_cast<Base*>(m_debugCamera.get())->BaseInitialize();
	m_debugCamera->Initialize();
	m_mainCamera = static_cast<GameObjectBase*>(m_debugCamera.get())->CreateComponent<CameraComponent>();
	#else
	m_mainCamera = CreateCameraObject()->CreateComponent<CameraComponent>();
	Initialize();
	#endif
}

void SceneBase::BaseUpdate()
{
	#ifdef _DEBUG
	m_debugCamera->Update();
	static_cast<Base*>(m_debugCamera.get())->BaseUpdate();
	#else
	Update();
	#endif

	RemovePending();
	// 게임 오브젝트 업데이트
	for (unique_ptr<Base>& gameObject : m_gameObjects) gameObject->BaseUpdate();

	#ifdef _DEBUG
	// Ctrl + S 입력 시 씬 저장
	InputManager& inputManager = InputManager::GetInstance();
	if (inputManager.GetKey(KeyCode::Control) && inputManager.GetKeyDown(KeyCode::S))
	{
		cout << "씬: " << m_type << " 저장 중..." << endl;

		const filesystem::path sceneFilePath = "../Asset/Scene/" + m_type + ".json";
		ofstream file(sceneFilePath);
		file << BaseSerialize().dump(4);
		file.close();

		cout << "씬: " << m_type << " 저장 완료!" << endl;
	}
	#endif
}

void SceneBase::BaseRender()
{
	// 상수 버퍼 업데이트 및 셰이더에 설정
	UpdateConstantBuffers();

	// 샘플러 상태 설정
	m_deviceContext->PSSetSamplers(static_cast<UINT>(SamplerState::Default), 1, ResourceManager::GetInstance().GetSamplerState(SamplerState::Default).GetAddressOf());

	// 환경 맵 설정
	m_deviceContext->PSSetShaderResources(static_cast<UINT>(TextureSlots::Environment), 1, m_environmentMapSRV.GetAddressOf());

	// 스카이박스 렌더링
	RenderSkybox();

	#ifdef NDEBUG
	Render();
	#endif

	// 게임 오브젝트 렌더링
	for (unique_ptr<Base>& gameObject : m_gameObjects) gameObject->BaseRender();
}

void SceneBase::BaseRenderImGui()
{
	#ifdef _DEBUG
	ImGui::Begin("Debug Camera");
	static_cast<Base*>(m_debugCamera.get())->BaseRenderImGui();
	ImGui::End();
	#endif

	ImGui::Begin(m_type.c_str());

	if (ImGui::DragFloat3("Light Direction", &m_directionalLightDirection.m128_f32[0], 0.001f, -1.0f, 1.0f)) {}
	if (ImGui::ColorEdit3("Scene Color", &m_lightColor.x)) {}

	RenderImGui();

	ImGui::Separator();
	ImGui::Text("Game Objects:");
	for (unique_ptr<Base>& gameObject : m_gameObjects) gameObject->BaseRenderImGui();

	ImGui::Separator();
	if (ImGui::Button("Add GameObject")) ImGui::OpenPopup("Select GameObject Type");
	if (ImGui::BeginPopup("Select GameObject Type"))
	{
		for (const auto& [typeName, factory] : TypeRegistry::GetInstance().m_gameObjectRegistry)
		{
			if (ImGui::Selectable(typeName.c_str()))
			{
				CreateRootGameObject(typeName);
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}

	ImGui::End();
}

void SceneBase::BaseFinalize()
{
	#ifdef NDEBUG
	Finalize();
	#endif

	// 게임 오브젝트 종료
	for (unique_ptr<Base>& gameObject : m_gameObjects) gameObject->BaseFinalize();
}

nlohmann::json SceneBase::BaseSerialize()
{
	nlohmann::json jsonData;

	// 기본 씬 데이터 저장
	jsonData["directionalLightDirection"] =
	{
		m_directionalLightDirection.m128_f32[0],
		m_directionalLightDirection.m128_f32[1],
		m_directionalLightDirection.m128_f32[2],
		m_directionalLightDirection.m128_f32[3]
	};
	jsonData["lightColor"] = { m_lightColor.x, m_lightColor.y, m_lightColor.z, m_lightColor.w };
	jsonData["environmentMapFileName"] = m_environmentMapFileName;

	// 파생 클래스의 직렬화 호출
	nlohmann::json derivedData = Serialize();
	if (!derivedData.is_null() && derivedData.is_object()) jsonData.merge_patch(derivedData);

	// 게임 오브젝트들 저장
	nlohmann::json gameObjectsData = nlohmann::json::array();
	for (unique_ptr<Base>& gameObject : m_gameObjects) gameObjectsData.push_back(gameObject->BaseSerialize());
	jsonData["rootGameObjects"] = gameObjectsData;

	return jsonData;
}

void SceneBase::BaseDeserialize(const nlohmann::json& jsonData)
{
	// 기본 씬 데이터 로드
	m_directionalLightDirection = XMVectorSet
	(
		jsonData["directionalLightDirection"][0].get<float>(),
		jsonData["directionalLightDirection"][1].get<float>(),
		jsonData["directionalLightDirection"][2].get<float>(),
		jsonData["directionalLightDirection"][3].get<float>()
	);
	// 광원 색상
	m_lightColor = XMFLOAT4
	(
		jsonData["lightColor"][0].get<float>(),
		jsonData["lightColor"][1].get<float>(),
		jsonData["lightColor"][2].get<float>(),
		jsonData["lightColor"][3].get<float>()
	);
	// 환경 맵 파일 이름
	m_environmentMapFileName = jsonData["environmentMapFileName"].get<string>();

	// 파생 클래스의 데이터 로드
	Deserialize(jsonData);

	// 게임 오브젝트들 로드
	for (const auto& gameObjectData : jsonData["rootGameObjects"])
	{
		string typeName = gameObjectData["type"].get<string>();
		unique_ptr<Base> gameObjectPtr = TypeRegistry::GetInstance().CreateGameObject(typeName);

		gameObjectPtr->BaseDeserialize(gameObjectData);
		gameObjectPtr->BaseInitialize();

		m_gameObjects.push_back(move(gameObjectPtr));
	}
}

void SceneBase::RemovePending()
{
	erase_if
	(
		m_gameObjects, [](const unique_ptr<Base>& gameObject)
		{
			if (!gameObject->GetAlive())
			{
				gameObject->BaseFinalize();
				return true;
			}
			return false;
		}
	);
}

void SceneBase::GetResources()
{
	ResourceManager& resourceManager = ResourceManager::GetInstance();

	m_viewProjectionConstantBuffer = resourceManager.GetConstantBuffer(sizeof(ViewProjectionBuffer)); // 뷰-투영 상수 버퍼 생성
	m_skyboxViewProjectionConstantBuffer = resourceManager.GetConstantBuffer(sizeof(XMMATRIX)); // 스카이박스 뷰-투영 역행렬 상수 버퍼 생성
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

	// 스카이박스 뷰-투영 역행렬 상수 버퍼 업데이트 및 셰이더에 설정 // m_viewProjectionData 재활용
	m_viewProjectionData.viewMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); // 뷰 행렬의 위치 성분 제거
	m_viewProjectionData.VPMatrix = XMMatrixInverse(nullptr, m_viewProjectionData.projectionMatrix * m_viewProjectionData.viewMatrix);
	m_deviceContext->UpdateSubresource(m_skyboxViewProjectionConstantBuffer.Get(), 0, nullptr, &m_viewProjectionData.VPMatrix, 0, 0);
	m_deviceContext->VSSetConstantBuffers(static_cast<UINT>(VSConstBuffers::SkyboxViewProjection), 1, m_skyboxViewProjectionConstantBuffer.GetAddressOf());

	// 카메라 위치 상수 버퍼 업데이트 및 셰이더에 설정
	m_deviceContext->UpdateSubresource(m_cameraPositionConstantBuffer.Get(), 0, nullptr, &m_mainCamera->GetPosition(), 0, 0);
	m_deviceContext->PSSetConstantBuffers(static_cast<UINT>(PSConstBuffers::CameraPosition), 1, m_cameraPositionConstantBuffer.GetAddressOf());

	// 방향광 상수 버퍼 업데이트 및 셰이더에 설정
	m_directionalLightData.lightDirection = -XMVector3Normalize(m_directionalLightDirection);
	m_directionalLightData.lightColor = m_lightColor;
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
///SceneBase.cpp의 끝