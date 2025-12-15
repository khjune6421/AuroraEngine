#include "stdafx.h"
#include "ModelComponent.h"

#include "GameObjectBase.h"
#include "Renderer.h"
#include "RenderResourceManager.h"

void ModelComponent::Render()
{
	const com_ptr<ID3D11DeviceContext> deviceContext = Renderer::GetInstance().GetDeviceContext();

	constexpr UINT stride = sizeof(RenderData::Vertex);
	constexpr UINT offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->IASetInputLayout(m_vertexShaderAndInputLayout.second.Get());
	deviceContext->VSSetShader(m_vertexShaderAndInputLayout.first.Get(), nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	deviceContext->DrawIndexed(m_indexCount, 0, 0);
}

void ModelComponent::Begin()
{
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateShaders();
}

void ModelComponent::CreateVertexBuffer()
{
	HRESULT hr = S_OK;

	const com_ptr<ID3D11Device> device = Renderer::GetInstance().GetDevice();

	const D3D11_BUFFER_DESC bufferDesc =
	{
		.ByteWidth = static_cast<UINT>(sizeof(RenderData::Vertex) * m_renderData.vertices.size()),
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};
	const D3D11_SUBRESOURCE_DATA initialData =
	{
		.pSysMem = m_renderData.vertices.data(),
		.SysMemPitch = 0,
		.SysMemSlicePitch = 0
	};
	hr = device->CreateBuffer(&bufferDesc, &initialData, m_vertexBuffer.GetAddressOf());
	CheckResult(hr, "버텍스 버퍼 생성 실패" + static_cast<char>(m_owner->GetID()));
}

void ModelComponent::CreateIndexBuffer()
{
	HRESULT hr = S_OK;

	const com_ptr<ID3D11Device> device = Renderer::GetInstance().GetDevice();

	m_indexCount = static_cast<UINT>(m_renderData.indices.size());
	const D3D11_BUFFER_DESC bufferDesc =
	{
		.ByteWidth = static_cast<UINT>(sizeof(UINT) * m_renderData.indices.size()),
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_INDEX_BUFFER,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};
	const D3D11_SUBRESOURCE_DATA initialData =
	{
		.pSysMem = m_renderData.indices.data(),
		.SysMemPitch = 0,
		.SysMemSlicePitch = 0
	};
	hr = device->CreateBuffer(&bufferDesc, &initialData, m_indexBuffer.GetAddressOf());
	CheckResult(hr, "인덱스 버퍼 생성 실패" + static_cast<char>(m_owner->GetID()));
}

void ModelComponent::CreateShaders()
{
	RenderResourceManager& resourceManager = RenderResourceManager::GetInstance();
	m_vertexShaderAndInputLayout = resourceManager.GetVertexShaderAndInputLayout(m_renderData.vsShaderName, m_renderData.m_inputElementDescs);
	m_pixelShader = resourceManager.GetPixelShader(m_renderData.psShaderName);
}