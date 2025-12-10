#include "stdafx.h"
#include "SceneBase.h"

#include "GameObjectBase.h"
#include "Renderer.h"

using namespace std;
using namespace DirectX;

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
	renderer.BeginFrame(m_clearColor);

	XMMATRIX viewMatrix = m_mainCamera.GetViewMatrix();
	XMMATRIX projectionMatrix = m_mainCamera.GetProjectionMatrix();

	for (auto& gameObject : m_gameObjects) gameObject->Render(viewMatrix, projectionMatrix);

	renderer.EndFrame();
}

void SceneBase::AddGameObject(unique_ptr<GameObjectBase> gameObject)
{
	gameObject->Initialize();
	m_gameObjects.push_back(move(gameObject));
}