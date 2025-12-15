#include "stdafx.h"
#include "RenderResourceManager.h"

using namespace std;

void RenderResourceManager::Initialize(com_ptr<ID3D11Device> device)
{
	m_device = device;

	CreateRasterStates();
	CreateSamplerStates();
}

com_ptr<ID3D11Buffer> RenderResourceManager::GetConstantBuffer(UINT bufferSize)
{
	// 기존에 생성된 상수 버퍼가 있으면 재사용
	auto it = m_constantBuffers.find(bufferSize);
	if (it != m_constantBuffers.end()) return it->second;

	HRESULT hr = S_OK;

	com_ptr<ID3D11Buffer> constantBuffer = nullptr;
	const D3D11_BUFFER_DESC bufferDesc =
	{
		.ByteWidth = bufferSize,
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, constantBuffer.GetAddressOf());
	CheckResult(hr, "상수 버퍼 생성 실패.");

	m_constantBuffers[bufferSize] = constantBuffer;

	return constantBuffer;
}

pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>> RenderResourceManager::GetVertexShaderAndInputLayout(string shaderName, const vector<D3D11_INPUT_ELEMENT_DESC>& inputElementDescs)
{
	// 기존에 생성된 셰이더 및 입력 레이아웃이 있으면 재사용
	auto it = m_vertexShadersAndInputLayouts.find(shaderName);
	if (it != m_vertexShadersAndInputLayouts.end()) return it->second;

	HRESULT hr = S_OK;

	// 버텍스 셰이더 컴파일
	com_ptr<ID3DBlob> VSCode = CompileShader(shaderName, "vs_5_0");
	hr = m_device->CreateVertexShader
	(
		VSCode->GetBufferPointer(),
		VSCode->GetBufferSize(),
		nullptr,
		m_vertexShadersAndInputLayouts[shaderName].first.GetAddressOf()
	);
	CheckResult(hr, "버텍스 셰이더 생성 실패.");

	// 입력 레이아웃 생성
	hr = m_device->CreateInputLayout
	(
		inputElementDescs.data(),
		static_cast<UINT>(inputElementDescs.size()),
		VSCode->GetBufferPointer(),
		VSCode->GetBufferSize(),
		m_vertexShadersAndInputLayouts[shaderName].second.GetAddressOf()
	);
	CheckResult(hr, "입력 레이아웃 생성 실패.");

	return m_vertexShadersAndInputLayouts[shaderName];
}

com_ptr<ID3D11PixelShader> RenderResourceManager::GetPixelShader(string shaderName)
{
	// 기존에 생성된 픽셀 셰이더가 있으면 재사용
	auto it = m_pixelShaders.find(shaderName);
	if (it != m_pixelShaders.end()) return it->second;

	HRESULT hr = S_OK;

	// 픽셀 셰이더 컴파일
	com_ptr<ID3DBlob> PSCode = CompileShader(shaderName, "ps_5_0");
	hr = m_device->CreatePixelShader
	(
		PSCode->GetBufferPointer(),
		PSCode->GetBufferSize(),
		nullptr,
		m_pixelShaders[shaderName].GetAddressOf()
	);
	CheckResult(hr, "픽셀 셰이더 생성 실패.");

	return m_pixelShaders[shaderName];
}

com_ptr<ID3DBlob> RenderResourceManager::CompileShader(string shaderName, const char* shaderModel)
{
	HRESULT hr = S_OK;

	const filesystem::path shaderPath = "../Asset/Shader/" + shaderName;
	com_ptr<ID3DBlob> shaderCode = nullptr;
	com_ptr<ID3DBlob> errorBlob = nullptr;

	hr = D3DCompileFromFile
	(
		shaderPath.wstring().c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		shaderModel,
		#ifdef _DEBUG
		D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		#else
		D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL3,
		#endif
		0,
		shaderCode.GetAddressOf(),
		errorBlob.GetAddressOf()
	);
	if (errorBlob) cerr << shaderName << " 셰이더 컴파일 오류: " << static_cast<const char*>(errorBlob->GetBufferPointer()) << endl;

	return shaderCode;
}

void RenderResourceManager::CreateRasterStates()
{
	HRESULT hr = S_OK;

	for (size_t i = 0; i < RSCount; ++i)
	{
		hr = m_device->CreateRasterizerState(&RASTERIZER_DESC_TEMPLATES[i], m_rasterStates[i].GetAddressOf());
		CheckResult(hr, "래스터 상태 생성 실패.");
	}
}

void RenderResourceManager::CreateSamplerStates()
{
	HRESULT hr = S_OK;

	for (size_t i = 0; i < SSCount; ++i)
	{
		hr = m_device->CreateSamplerState(&SAMPLER_DESC_TEMPLATES[i], m_samplerStates[i].GetAddressOf());
		CheckResult(hr, "샘플러 상태 생성 실패.");
	}
}