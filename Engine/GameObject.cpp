#include "stdafx.h"
#include "GameObject.h"

#include "Renderer.h"

using namespace std;
using namespace DirectX;

static UINT s_idIndex = 0;

GameObject::GameObject() { m_id = s_idIndex++; }

void GameObject::Initialize(Renderer* renderer)
{
	m_renderer = renderer;
	m_device = renderer->GetDevice();
	m_deviceContext = renderer->GetDeviceContext();
};

void GameObject::CreateRenderResources()
{
	CreateConstantBuffer();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateShaders();
	SetActive(true);
}

void GameObject::UpdateWorldMatrix()
{
	if (!m_isDirty) return;

	m_positionMatrix = XMMatrixTranslationFromVector(m_position);
	m_rotationMatrix = XMMatrixRotationRollPitchYawFromVector(m_rotation);
	m_scaleMatrix = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

	m_worldMatrix = m_scaleMatrix * m_rotationMatrix * m_positionMatrix;

	m_isDirty = false;
}

void GameObject::Render(XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
	if (!m_isActive) return;

	const XMMATRIX wvpMatrix = XMMatrixTranspose(m_worldMatrix * viewMatrix * projectionMatrix);
	m_deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &wvpMatrix, 0, 0);
	m_deviceContext->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

	constexpr UINT stride = sizeof(RenderData::Vertex);
	constexpr UINT offset = 0;
	m_deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	m_deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_deviceContext->IASetInputLayout(m_inputLayout.Get());
	m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	m_deviceContext->DrawIndexed(m_indexCount, 0, 0);
}

void GameObject::CreateConstantBuffer()
{
	HRESULT hr = S_OK;

	const D3D11_BUFFER_DESC bufferDesc =
	{
		.ByteWidth = sizeof(XMMATRIX),
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, m_constantBuffer.GetAddressOf());
	m_renderer->CheckResult(hr, "상수 버퍼 생성 실패. 오브젝트 ID: " + static_cast<char>(m_id));
}

void GameObject::CreateVertexBuffer()
{
	HRESULT hr = S_OK;

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

	hr = m_device->CreateBuffer(&bufferDesc, &initialData, m_vertexBuffer.GetAddressOf());
	m_renderer->CheckResult(hr, "버텍스 버퍼 생성 실패. 오브젝트 ID: " + static_cast<char>(m_id));
}

void GameObject::CreateIndexBuffer()
{
	HRESULT hr = S_OK;

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

	hr = m_device->CreateBuffer(&bufferDesc, &initialData, m_indexBuffer.GetAddressOf());
	m_renderer->CheckResult(hr, "인덱스 버퍼 생성 실패. 오브젝트 ID: " + static_cast<char>(m_id));
}

void GameObject::CreateShaders()
{
	HRESULT hr = S_OK;

	// 버텍스 셰이더 컴파일
	com_ptr<ID3DBlob> VSCode = nullptr;
	hr = m_renderer->CompileShader(m_renderData.vsShaderName, VSCode.GetAddressOf(), "vs_5_0");
	m_renderer->CheckResult(hr, "버텍스 셰이더 컴파일 실패. 오브젝트 ID: " + static_cast<char>(m_id));

	// 버텍스 셰이더 생성
	hr = m_device->CreateVertexShader
	(
		VSCode->GetBufferPointer(),
		VSCode->GetBufferSize(),
		nullptr,
		m_vertexShader.GetAddressOf()
	);
	m_renderer->CheckResult(hr, "버텍스 셰이더 생성 실패. 오브젝트 ID: " + static_cast<char>(m_id));

	// 입력 레이아웃 생성
	hr = m_device->CreateInputLayout
	(
		m_renderData.m_inputElementDescs.data(),
		static_cast<UINT>(m_renderData.m_inputElementDescs.size()),
		VSCode->GetBufferPointer(),
		VSCode->GetBufferSize(),
		m_inputLayout.GetAddressOf()
	);

	// 픽셀 셰이더 컴파일
	com_ptr<ID3DBlob> PSCode = nullptr;
	hr = m_renderer->CompileShader(m_renderData.psShaderName, PSCode.GetAddressOf(), "ps_5_0");
	m_renderer->CheckResult(hr, "픽셀 셰이더 컴파일 실패. 오브젝트 ID: " + static_cast<char>(m_id));

	// 픽셀 셰이더 생성
	hr = m_device->CreatePixelShader
	(
		PSCode->GetBufferPointer(),
		PSCode->GetBufferSize(),
		nullptr,
		m_pixelShader.GetAddressOf()
	);
	m_renderer->CheckResult(hr, "픽셀 셰이더 생성 실패. 오브젝트 ID: " + static_cast<char>(m_id));
}