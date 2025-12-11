#include "stdafx.h"
#include "SceneBase.h"

#include "GameObjectBase.h"
#include "Renderer.h"

using namespace std;
using namespace DirectX;

void SceneBase::Initialize()
{
	unique_ptr<Camera> camera = make_unique<Camera>();
	m_mainCamera = camera.get();
	AddGameObject(move(camera));
	m_viewProjectionConstantBuffer = Renderer::GetInstance().GetConstantBuffer(sizeof(ViewProjectionBuffer));

	Begin();
}

void SceneBase::Update(float deltaTime)
{
	for (unique_ptr<GameObjectBase>& gameObject : m_gameObjects) gameObject->Update(deltaTime);

	if (GetAsyncKeyState('W') & 0x8000) m_mainCamera->MoveDirection(0.1f, GameObjectBase::Direction::Forward);
	if (GetAsyncKeyState('S') & 0x8000) m_mainCamera->MoveDirection(0.1f, GameObjectBase::Direction::Backward);
	if (GetAsyncKeyState('A') & 0x8000) m_mainCamera->MoveDirection(0.1f, GameObjectBase::Direction::Left);
	if (GetAsyncKeyState('D') & 0x8000) m_mainCamera->MoveDirection(0.1f, GameObjectBase::Direction::Right);
}

void SceneBase::TransformGameObjects()
{
	// TransformGameObjects가 setDirty를 해소하므로 여기서 카메라 뷰 행렬 갱신
	m_mainCamera->UpdateViewMatrix();

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

void SceneBase::AddGameObject(unique_ptr<GameObjectBase> gameObject)
{
	gameObject->Initialize();
	m_gameObjects.push_back(move(gameObject));
}