#include "stdafx.h"
#include "SceneBase.h"

#include "GameObject.h"
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
	m_renderer->BeginFrame(m_clearColor);

	XMMATRIX viewMatrix = m_mainCamera.GetViewMatrix();
	XMMATRIX projectionMatrix = m_mainCamera.GetProjectionMatrix();

	for (auto& gameObject : m_gameObjects) gameObject->Render(viewMatrix, projectionMatrix);

	m_renderer->EndFrame();
}

void SceneBase::AddGameObject(unique_ptr<GameObject> gameObject)
{
	gameObject->Initialize(m_renderer);
	gameObject->Begin();
	m_gameObjects.push_back(move(gameObject));
}