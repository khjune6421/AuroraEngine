#include "stdafx.h"
#include "TestScene.h"

using namespace std;

void TestScene::Begin()
{
	m_mainCamera.SetPosition({ 0.0f, 0.0f, -5.0f, 1.0f });

	unique_ptr<GameObject> triangle = make_unique<GameObject>();
	GameObject* trianglePtr = triangle.get();
	AddGameObject(move(triangle));

	trianglePtr->SetPosition({ 0.0f, 0.0f, 1.0f, 1.0f });
	trianglePtr->SetRotation({ 0.0f, 0.0f, 0.0f, 0.0f });

	trianglePtr->CreateConstantBuffer();

	const vector<GameObject::Vertex> vertices =
	{
		{ .position = { 0.0f, 0.5f, 0.0f, 1.0f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ .position = { 0.5f, -0.5f, 0.0f, 1.0f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ .position = { -0.5f, -0.5f, 0.0f, 1.0f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f } }
	};
	trianglePtr->CreateVertexBuffer(vertices);

	const vector<UINT> indices = { 0, 1, 2 };
	trianglePtr->CreateIndexBuffer(indices);

	const vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescs =
	{
		D3D11_INPUT_ELEMENT_DESC
		{
			.SemanticName = "POSITION",
			.SemanticIndex = 0,
			.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		},
		D3D11_INPUT_ELEMENT_DESC
		{
			.SemanticName = "COLOR",
			.SemanticIndex = 0,
			.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		}
	};
	trianglePtr->CreateShaders(L"VSVertexColor.hlsl", L"PSVertexColor.hlsl", inputElementDescs);
	trianglePtr->SetActive(true);
}