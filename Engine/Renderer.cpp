#include "stdafx.h"
#include "Renderer.h"

using namespace std;
using namespace DirectX;

Renderer::~Renderer()
{
}

void Renderer::Initialize(HWND hWnd)
{
	CreateDeviceAndContext();
	CreateSwapChain(hWnd);
	CreateBackBufferRenderTarget();
	CreateBackBufferVertexBufferAndShaders();
	CreateSceneRenderTarget();
	SetViewport();
	CreateRasterStates();
	CreateSamplerStates();
}

void Renderer::BeginFrame(const array<FLOAT, 4>& clearColor)
{
	HRESULT hr = S_OK;

	// 래스터 상태 변경
	hr = SetRasterState(RSSolid);
	CheckResult(hr, "래스터 상태 설정 실패.");

	// 씬 렌더 타겟으로 설정
	m_deviceContext->OMSetRenderTargets(1, m_sceneBuffer.renderTargetView.GetAddressOf(), m_sceneBuffer.depthStencilView.Get());

	// 씬 렌더 타겟 클리어
	ClearRenderTarget(m_sceneBuffer, clearColor);
}

void Renderer::EndFrame()
{
	HRESULT hr = S_OK;

	// 씬 렌더 타겟 MSAA 해제 및 결과 텍스처 복사
	ResolveSceneMSAA();

	// 래스터 상태 변경
	hr = SetRasterState(RSBackBuffer);

	// 백 버퍼로 씬 렌더링
	RenderSceneToBackBuffer();

	// 스왑 체인 프레젠트
	hr = m_swapChain->Present(1, 0);
	CheckResult(hr, "스왑 체인 프레젠트 실패.");
}

HRESULT Renderer::Resize(UINT width, UINT height)
{
	if (width <= 0 || height <= 0) return E_INVALIDARG;

	HRESULT hr = S_OK;

	// 렌더 타겟 해제 및 플러시
	constexpr ID3D11RenderTargetView* nullRTV = nullptr;
	m_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
	m_deviceContext->Flush();

	m_swapChainDesc.Width = width;
	m_swapChainDesc.Height = height;

	// 스왑 체인 크기 조정
	hr = m_swapChain->ResizeBuffers
	(
		m_swapChainDesc.BufferCount,
		m_swapChainDesc.Width,
		m_swapChainDesc.Height,
		m_swapChainDesc.Format,
		m_swapChainDesc.Flags
	);
	CheckResult(hr, "스왑 체인 버퍼 크기 조정 실패.");

	// 새로운 백 버퍼, 씬 렌더 타겟 생성
	CreateBackBufferRenderTarget();
	CreateSceneRenderTarget();

	// 뷰포트 설정
	SetViewport();

	return hr;
}

HRESULT Renderer::SetRasterState(RasterState state)
{
	if (state >= RSCount) return E_INVALIDARG;
	if (m_rasterState == state) return S_OK;

	m_rasterState = state;
	m_deviceContext->RSSetState(m_rasterStates[state].Get());

	return S_OK;
}

com_ptr<ID3D11Buffer> Renderer::GetConstantBuffer(UINT bufferSize)
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

pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>> Renderer::GetVertexShaderAndInputLayout(wstring shaderName, const vector<D3D11_INPUT_ELEMENT_DESC>& inputElementDescs)
{
	// 기존에 생성된 셰이더 및 입력 레이아웃이 있으면 재사용
	auto it = m_vertexShadersAndInputLayouts.find(shaderName);
	if (it != m_vertexShadersAndInputLayouts.end()) return it->second;

	HRESULT hr = S_OK;

	// 버텍스 셰이더 컴파일
	com_ptr<ID3DBlob> VSCode = CompileShader(filesystem::path(shaderName), "vs_5_0");
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

com_ptr<ID3D11PixelShader> Renderer::GetPixelShader(wstring shaderName)
{
	// 기존에 생성된 픽셀 셰이더가 있으면 재사용
	auto it = m_pixelShaders.find(shaderName);
	if (it != m_pixelShaders.end()) return it->second;

	HRESULT hr = S_OK;

	// 픽셀 셰이더 컴파일
	com_ptr<ID3DBlob> PSCode = CompileShader(filesystem::path(shaderName), "ps_5_0");
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

com_ptr<ID3DBlob> Renderer::CompileShader(filesystem::path shaderName, const char* shaderModel)
{
	HRESULT hr = S_OK;

	const filesystem::path shaderPath = L"../Asset/Shader/" / shaderName;
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
	if (errorBlob) cerr << shaderName.string() << " 셰이더 컴파일 오류: " << static_cast<const char*>(errorBlob->GetBufferPointer()) << endl;

	return shaderCode;
}

void Renderer::CheckResult(HRESULT hr, const char* msg) const
{
	if (FAILED(hr))
	{
		#ifdef _DEBUG
		cerr << msg << " 렌더러 에러 코드: " << hex << hr << endl;
		#else
		MessageBoxA(nullptr, msg, "렌더러 오류", MB_OK | MB_ICONERROR);
		#endif
		exit(EXIT_FAILURE);
	}
}

void Renderer::CreateDeviceAndContext()
{
	HRESULT hr = S_OK;

	hr = D3D11CreateDevice
	(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		#ifdef _DEBUG
		D3D11_CREATE_DEVICE_DEBUG,
		#else
		0,
		#endif
		m_featureLevels.data(),
		static_cast<UINT>(m_featureLevels.size()),
		D3D11_SDK_VERSION,
		m_device.GetAddressOf(),
		nullptr,
		m_deviceContext.GetAddressOf()
	);
	CheckResult(hr, "디바이스 및 디바이스 컨텍스트 생성 실패.");
}

void Renderer::CreateSwapChain(HWND hWnd)
{
	HRESULT hr = S_OK;

	com_ptr<IDXGIDevice> dxgiDevice = nullptr;
	com_ptr<IDXGIAdapter> dxgiAdapter = nullptr;
	com_ptr<IDXGIFactory2> dxgiFactory = nullptr;

	hr = m_device.As(&dxgiDevice);
	CheckResult(hr, "ID3D11Device 로부터 IDXGIDevice 얻기 실패.");
	hr = dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
	CheckResult(hr, "IDXGIDevice 로부터 IDXGIAdapter 얻기 실패.");
	hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
	CheckResult(hr, "IDXGIAdapter 로부터 IDXGIFactory2 얻기 실패.");

	// 스왑 체인 생성
	hr = dxgiFactory->CreateSwapChainForHwnd
	(
		dxgiDevice.Get(),
		hWnd,
		&m_swapChainDesc,
		nullptr,
		nullptr,
		m_swapChain.GetAddressOf()
	);
	CheckResult(hr, "스왑 체인 생성 실패.");
}

void Renderer::CreateBackBufferRenderTarget()
{
	HRESULT hr = S_OK;

	// 기존 렌더 타겟 해제
	m_backBuffer.renderTarget.Reset();
	m_backBuffer.renderTargetView.Reset();

	hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&m_backBuffer.renderTarget));
	CheckResult(hr, "스왑 체인 버퍼 얻기 실패.");

	// 렌더 타겟 뷰 생성
	const D3D11_RENDER_TARGET_VIEW_DESC rtvDesc =
	{
		.Format = m_swapChainDesc.Format,
		.ViewDimension = m_swapChainDesc.SampleDesc.Count > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D
	};
	hr = m_device->CreateRenderTargetView(m_backBuffer.renderTarget.Get(), &rtvDesc, m_backBuffer.renderTargetView.GetAddressOf());
	CheckResult(hr, "렌더 타겟 뷰 생성 실패.");
}

void Renderer::CreateBackBufferVertexBufferAndShaders()
{
	HRESULT hr = S_OK;

	constexpr array<BackBufferVertex, 3> backBufferVertices = // 전체 화면 삼각형 버텍스 데이터
	{
		BackBufferVertex{ .position = { -1.0f, -1.0f, 0.0f, 1.0f }, .UV = { 0.0f, 1.0f } },
		BackBufferVertex{ .position = { -1.0f, 3.0f, 0.0f, 1.0f }, .UV = { 0.0f, -1.0f } },
		BackBufferVertex{ .position = { 3.0f, -1.0f, 0.0f, 1.0f }, .UV = { 2.0f, 1.0f } }
	};
	constexpr D3D11_BUFFER_DESC bufferDesc =
	{
		.ByteWidth = sizeof(backBufferVertices),
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};
	const D3D11_SUBRESOURCE_DATA initialData = { .pSysMem = backBufferVertices.data() };

	hr = m_device->CreateBuffer(&bufferDesc, &initialData, m_backBufferVertexBuffer.GetAddressOf());
	CheckResult(hr, "백 버퍼 버텍스 버퍼 생성 실패.");

	const vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescs =
	{
		D3D11_INPUT_ELEMENT_DESC
		{
			.SemanticName = "POSITION", // 이름
			.SemanticIndex = 0, // 인덱스 // 같은 이름의 여러 요소 구분용 // 일반적으로 쓸일 없음
			.Format = DXGI_FORMAT_R32G32B32A32_FLOAT, // 형식 // float4
			.InputSlot = 0, // 입력 슬롯 // 여러 버텍스 버퍼 사용할 때 구분용
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT, // 오프셋 자동 계산
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA, // 입력 슬롯 클래스
			.InstanceDataStepRate = 0 // D3D11_INPUT_PER_VERTEX_DATA 일시 무조건 0
		},
		D3D11_INPUT_ELEMENT_DESC
		{
			.SemanticName = "TEXCOORD",
			.SemanticIndex = 0,
			.Format = DXGI_FORMAT_R32G32_FLOAT, // float2
			.InputSlot = 0,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		}
	};
	// 버텍스 셰이더 및 입력 레이아웃 생성
	m_backBufferVertexShaderAndInputLayout = GetVertexShaderAndInputLayout(L"VSPostProcessing.hlsl", inputElementDescs);

	// 픽셀 셰이더 컴파일 및 생성
	m_backBufferPixelShader = GetPixelShader(L"PSPostProcessing.hlsl");
}

void Renderer::CreateSceneRenderTarget()
{
	HRESULT hr = S_OK;

	m_sceneBuffer.renderTarget.Reset();
	m_sceneBuffer.renderTargetView.Reset();
	m_sceneBuffer.depthStencilTexture.Reset();
	m_sceneBuffer.depthStencilView.Reset();

	// 렌더 타겟 텍스처 생성
	const D3D11_TEXTURE2D_DESC textureDesc =
	{
		.Width = m_swapChainDesc.Width,
		.Height = m_swapChainDesc.Height,
		.MipLevels = 1, // 단일 밉맵
		.ArraySize = 1, // 단일 텍스처
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM, // 감마 보정 안함
		.SampleDesc = m_sceneBufferSampleDesc,
		.Usage = D3D11_USAGE_DEFAULT, // GPU 읽기/쓰기
		.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, // 렌더 타겟 및 셰이더 리소스
		.CPUAccessFlags = 0, // CPU 접근 없음
		.MiscFlags = 0 // 기타 플래그 없음
	};
	hr = m_device->CreateTexture2D(&textureDesc, nullptr, m_sceneBuffer.renderTarget.GetAddressOf());
	CheckResult(hr, "씬 렌더 타겟 텍스처 생성 실패.");

	// 렌더 타겟 뷰 생성
	const D3D11_RENDER_TARGET_VIEW_DESC rtvDesc =
	{
		.Format = textureDesc.Format,
		.ViewDimension = textureDesc.SampleDesc.Count > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D // 멀티샘플링 여부에 따른 뷰 차원
	};
	hr = m_device->CreateRenderTargetView(m_sceneBuffer.renderTarget.Get(), &rtvDesc, m_sceneBuffer.renderTargetView.GetAddressOf());
	CheckResult(hr, "씬 렌더 타겟 뷰 생성 실패.");

	// 깊이-스텐실 텍스처 및 뷰 생성
	const D3D11_TEXTURE2D_DESC depthStencilDesc =
	{
		.Width = m_swapChainDesc.Width,
		.Height = m_swapChainDesc.Height,
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT, // 깊이: 32비트 실수, 스텐실: 8비트 정수
		.SampleDesc = textureDesc.SampleDesc,
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_DEPTH_STENCIL,
		.CPUAccessFlags = 0,
		.MiscFlags = 0
	};
	hr = m_device->CreateTexture2D(&depthStencilDesc, nullptr, m_sceneBuffer.depthStencilTexture.GetAddressOf());
	CheckResult(hr, "씬 깊이-스텐실 텍스처 생성 실패.");

	// 깊이-스텐실 뷰 생성
	const D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc =
	{
		.Format = depthStencilDesc.Format,
		.ViewDimension = depthStencilDesc.SampleDesc.Count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D,
		.Flags = 0
	};
	hr = m_device->CreateDepthStencilView(m_sceneBuffer.depthStencilTexture.Get(), &dsvDesc, m_sceneBuffer.depthStencilView.GetAddressOf());
	CheckResult(hr, "씬 깊이-스텐실 뷰 생성 실패.");

	const D3D11_TEXTURE2D_DESC sceneResultDesc =
	{
		.Width = m_swapChainDesc.Width,
		.Height = m_swapChainDesc.Height,
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = textureDesc.Format,
		.SampleDesc = { 1, 0 }, // 결과 텍스처는 단일 샘플링
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_SHADER_RESOURCE, // 셰이더 리소스 용도
		.CPUAccessFlags = 0,
		.MiscFlags = 0
	};
	hr = m_device->CreateTexture2D(&sceneResultDesc, nullptr, m_sceneResultTexture.GetAddressOf());
	CheckResult(hr, "씬 결과 텍스처 생성 실패.");

	// 씬 렌더 타겟의 셰이더 리소스 뷰 생성
	const D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc =
	{
		.Format = textureDesc.Format,
		.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
		.Texture2D = { .MostDetailedMip = 0, .MipLevels = 1 }
	};
	hr = m_device->CreateShaderResourceView(m_sceneResultTexture.Get(), &srvDesc, m_sceneShaderResourceView.GetAddressOf());
	CheckResult(hr, "씬 셰이더 리소스 뷰 생성 실패.");
}

void Renderer::SetViewport()
{
	const D3D11_VIEWPORT viewport =
	{
		.TopLeftX = 0.0f,
		.TopLeftY = 0.0f,
		.Width = static_cast<FLOAT>(m_swapChainDesc.Width),
		.Height = static_cast<FLOAT>(m_swapChainDesc.Height),
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f
	};
	m_deviceContext->RSSetViewports(1, &viewport);
}

void Renderer::CreateRasterStates()
{
	HRESULT hr = S_OK;

	constexpr array<D3D11_RASTERIZER_DESC, RSCount> rasterDescTemplates =
	{
		// RSBackBuffer
		D3D11_RASTERIZER_DESC
		{
			.FillMode = D3D11_FILL_SOLID, // 실선 채우기
			.CullMode = D3D11_CULL_NONE, // 면 컬링 없음
			.FrontCounterClockwise = FALSE, // 시계방향이 앞면
			.DepthBias = 0, // 깊이 바이어스 없음
			.DepthBiasClamp = 0.0f, // 깊이 바이어스 클램프 없음
			.SlopeScaledDepthBias = 0.0f, // 기울기 기반 깊이 바이어스 없음
			.DepthClipEnable = TRUE, // 깊이 클리핑 활성화
			.ScissorEnable = FALSE, // 가위 영역 비활성화
			.MultisampleEnable = FALSE, // 멀티샘플링 비활성화
			.AntialiasedLineEnable = FALSE // 앤티앨리어싱 선 비활성화
		},

		// RSSolid
		D3D11_RASTERIZER_DESC
		{
			.FillMode = D3D11_FILL_SOLID,
			.CullMode = D3D11_CULL_BACK,
			.FrontCounterClockwise = FALSE,
			.DepthBias = 0,
			.DepthBiasClamp = 0.0f,
			.SlopeScaledDepthBias = 0.0f,
			.DepthClipEnable = TRUE,
			.ScissorEnable = FALSE,
			.MultisampleEnable = TRUE,
			.AntialiasedLineEnable = TRUE
		},

		// RSWireframe
		D3D11_RASTERIZER_DESC
		{
			.FillMode = D3D11_FILL_WIREFRAME,
			.CullMode = D3D11_CULL_BACK,
			.FrontCounterClockwise = FALSE,
			.DepthBias = 0,
			.DepthBiasClamp = 0.0f,
			.SlopeScaledDepthBias = 0.0f,
			.DepthClipEnable = TRUE,
			.ScissorEnable = FALSE,
			.MultisampleEnable = TRUE,
			.AntialiasedLineEnable = TRUE
		}
	};

	for (size_t i = 0; i < rasterDescTemplates.size(); ++i)
	{
		hr = m_device->CreateRasterizerState(&rasterDescTemplates[i], m_rasterStates[i].GetAddressOf());
		CheckResult(hr, "래스터 상태 생성 실패.");
	}

	hr = SetRasterState(m_rasterState);
	CheckResult(hr, "초기 래스터 상태 설정 실패.");
}

void Renderer::CreateSamplerStates()
{
	HRESULT hr = S_OK;

	constexpr array<D3D11_SAMPLER_DESC, SSCount> samplerDescTemplates =
	{
		// SSBackBuffer
		D3D11_SAMPLER_DESC
		{
			.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, // 선형 필터링
			.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP, // U 좌표 클램핑
			.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP, // V 좌표 클램핑
			.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP, // W 좌표 클램핑
			.MipLODBias = 0.0f, // 밉 LOD 바이어스 없음
			.MaxAnisotropy = 1, // 이방성 필터링 없음
			.ComparisonFunc = D3D11_COMPARISON_NEVER, // 비교 함수 없음
			.BorderColor = { 0.0f, 0.0f, 0.0f, 0.0f }, // 테두리 색상 (사용 안 함)
			.MinLOD = 0, // 최소 LOD
			.MaxLOD = D3D11_FLOAT32_MAX // 최대 LOD
		},

		// SSScene
		D3D11_SAMPLER_DESC
		{
			.Filter = D3D11_FILTER_ANISOTROPIC, // 이방성 필터링
			.AddressU = D3D11_TEXTURE_ADDRESS_WRAP, // U 좌표 래핑
			.AddressV = D3D11_TEXTURE_ADDRESS_WRAP, // V 좌표 래핑
			.AddressW = D3D11_TEXTURE_ADDRESS_WRAP, // W 좌표 래핑
			.MipLODBias = 0.0f,
			.MaxAnisotropy = 8, // 최대 이방성 필터링
			.ComparisonFunc = D3D11_COMPARISON_NEVER,
			.BorderColor = { 0.0f, 0.0f, 0.0f, 0.0f },
			.MinLOD = 0,
			.MaxLOD = D3D11_FLOAT32_MAX
		}
	};

	for (size_t i = 0; i < samplerDescTemplates.size(); ++i)
	{
		hr = m_device->CreateSamplerState(&samplerDescTemplates[i], m_samplerStates[i].GetAddressOf());
		CheckResult(hr, "샘플러 상태 생성 실패.");
	}
}

void Renderer::ClearRenderTarget(RenderTarget& target, const array<FLOAT, 4>& clearColor)
{
	m_deviceContext->ClearRenderTargetView(target.renderTargetView.Get(), clearColor.data());
	if (target.depthStencilView) m_deviceContext->ClearDepthStencilView(target.depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::ResolveSceneMSAA()
{
	if (m_sceneBufferSampleDesc.Count > 1) m_deviceContext->ResolveSubresource(m_sceneResultTexture.Get(), 0, m_sceneBuffer.renderTarget.Get(), 0, m_swapChainDesc.Format);
	else m_deviceContext->CopyResource(m_sceneResultTexture.Get(), m_sceneBuffer.renderTarget.Get());
}

void Renderer::RenderSceneToBackBuffer()
{
	// 픽셀 셰이더의 셰이더 리소스 뷰 해제
	constexpr ID3D11ShaderResourceView* nullSRV = nullptr;
	m_deviceContext->PSSetShaderResources(0, 1, &nullSRV);

	// 백 버퍼 렌더 타겟으로 설정
	m_deviceContext->OMSetRenderTargets(1, m_backBuffer.renderTargetView.GetAddressOf(), m_backBuffer.depthStencilView.Get());

	// 백 버퍼 클리어
	constexpr array<FLOAT, 4> clearColor = { 1.0f, 0.0f, 0.0f, 1.0f };
	ClearRenderTarget(m_backBuffer, clearColor);

	// 전체 화면 삼각형 렌더링
	constexpr UINT stride = sizeof(BackBufferVertex);
	constexpr UINT offset = 0;

	m_deviceContext->IASetVertexBuffers(0, 1, m_backBufferVertexBuffer.GetAddressOf(), &stride, &offset);
	m_deviceContext->IASetInputLayout(m_backBufferVertexShaderAndInputLayout.second.Get());
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_deviceContext->VSSetShader(m_backBufferVertexShaderAndInputLayout.first.Get(), nullptr, 0);
	m_deviceContext->PSSetShader(m_backBufferPixelShader.Get(), nullptr, 0);

	m_deviceContext->PSSetShaderResources(0, 1, m_sceneShaderResourceView.GetAddressOf());
	m_deviceContext->PSSetSamplers(0, 1, m_samplerStates[SSBackBuffer].GetAddressOf());

	m_deviceContext->Draw(3, 0);
}