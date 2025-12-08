#include "stdafx.h"
#include "Renderer.h"

using namespace std;
using namespace DirectX;

Renderer::Renderer(HWND hWnd) : m_hWnd(hWnd) {}

Renderer::~Renderer()
{
}

void Renderer::Initialize()
{
	CreateDeviceAndContext();
	CreateSwapChain();
	CreateBackBufferRenderTarget();
	CreateBackBufferVertexBufferAndShaders();
	CreateSceneRenderTarget();
	SetViewport();
	CreateRasterStates();
	CreateSamplerStates();
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

void Renderer::CreateSwapChain()
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
		m_hWnd,
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

	struct BackBufferVertex
	{
		XMFLOAT4 position = {};
		XMFLOAT2 UV = {};
	};
	constexpr array<BackBufferVertex, 3> backBufferVertices = // 전체 화면 삼각형 버텍스 데이터
	{
		BackBufferVertex{ .position = { -1.0f, -1.0f, 0.0f, 0.0f }, .UV = { 0.0f, 1.0f } },
		BackBufferVertex{ .position = { -1.0f, 3.0f, 0.0f, 0.0f }, .UV = { 0.0f, -1.0f } },
		BackBufferVertex{ .position = { 3.0f, -1.0f, 0.0f, 0.0f }, .UV = { 2.0f, 1.0f } },
	};
	constexpr D3D11_BUFFER_DESC bufferDesc =
	{
		.ByteWidth = sizeof(backBufferVertices),
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
	};
	const D3D11_SUBRESOURCE_DATA initialData = { .pSysMem = backBufferVertices.data() };

	hr = m_device->CreateBuffer(&bufferDesc, &initialData, m_backBufferVertexBuffer.GetAddressOf());
	CheckResult(hr, "백 버퍼 버텍스 버퍼 생성 실패.");

	// 버텍스 셰이더 컴파일 및 생성
	com_ptr<ID3DBlob> VSCode = nullptr;
	hr = CompileShader("VSPostProcessing.hlsl", VSCode.GetAddressOf(), "vs_5_0");
	CheckResult(hr, "백 버퍼 버텍스 셰이더 컴파일 실패.");

	hr = m_device->CreateVertexShader
	(
		VSCode->GetBufferPointer(),
		VSCode->GetBufferSize(),
		nullptr,
		m_backBufferVertexShader.GetAddressOf()
	);
	CheckResult(hr, "백 버퍼 버텍스 셰이더 생성 실패.");

	// 입력 레이아웃 생성
	constexpr array<D3D11_INPUT_ELEMENT_DESC, 2> inputElementDescs = // 입력 레이아웃 정의
	{
		D3D11_INPUT_ELEMENT_DESC
		{
			.SemanticName = "POSITION",
			.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
		},
		D3D11_INPUT_ELEMENT_DESC
		{
			.SemanticName = "TEXCOORD",
			.Format = DXGI_FORMAT_R32G32_FLOAT,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
		}
	};
	hr = m_device->CreateInputLayout
	(
		inputElementDescs.data(),
		static_cast<UINT>(inputElementDescs.size()),
		VSCode->GetBufferPointer(),
		VSCode->GetBufferSize(),
		m_backBufferInputLayout.GetAddressOf()
	);
	CheckResult(hr, "백 버퍼 입력 레이아웃 생성 실패.");

	// 픽셀 셰이더 컴파일 및 생성
	com_ptr<ID3DBlob> PSCode = nullptr;
	hr = CompileShader("PSPostProcessing.hlsl", PSCode.GetAddressOf(), "ps_5_0");
	CheckResult(hr, "백 버퍼 픽셀 셰이더 컴파일 실패.");

	hr = m_device->CreatePixelShader
	(
		PSCode->GetBufferPointer(),
		PSCode->GetBufferSize(),
		nullptr,
		m_backBufferPixelShader.GetAddressOf()
	);
	CheckResult(hr, "백 버퍼 픽셀 셰이더 생성 실패.");
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
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM, // 감마 보정 안함
		.SampleDesc = { .Count = 4, .Quality = 0 },
		.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
	};
	hr = m_device->CreateTexture2D(&textureDesc, nullptr, m_sceneBuffer.renderTarget.GetAddressOf());
	CheckResult(hr, "씬 렌더 타겟 텍스처 생성 실패.");

	// 렌더 타겟 뷰 생성
	const D3D11_RENDER_TARGET_VIEW_DESC rtvDesc =
	{
		.Format = textureDesc.Format,
		.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D
	};
	hr = m_device->CreateRenderTargetView(m_sceneBuffer.renderTarget.Get(), &rtvDesc, m_sceneBuffer.renderTargetView.GetAddressOf());
	CheckResult(hr, "씬 렌더 타겟 뷰 생성 실패.");

	// 깊이-스텐실 텍스처 및 뷰 생성
	const D3D11_TEXTURE2D_DESC depthStencilDesc =
	{
		.Width = m_swapChainDesc.Width,
		.Height = m_swapChainDesc.Height,
		.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT, // 깊이: 32비트 실수, 스텐실: 8비트 정수
		.BindFlags = D3D11_BIND_DEPTH_STENCIL,
	};
	hr = m_device->CreateTexture2D(&depthStencilDesc, nullptr, m_sceneBuffer.depthStencilTexture.GetAddressOf());
	CheckResult(hr, "씬 깊이-스텐실 텍스처 생성 실패.");

	// 깊이-스텐실 뷰 생성
	const D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc =
	{
		.Format = depthStencilDesc.Format,
		.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D
	};
	hr = m_device->CreateDepthStencilView(m_sceneBuffer.depthStencilTexture.Get(), &dsvDesc, m_sceneBuffer.depthStencilView.GetAddressOf());
	CheckResult(hr, "씬 깊이-스텐실 뷰 생성 실패.");

	// 씬 렌더 타겟의 셰이더 리소스 뷰 생성
	const D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc =
	{
		.Format = textureDesc.Format,
		.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
		.Texture2D = { .MostDetailedMip = 0, .MipLevels = 1 }
	};
	hr = m_device->CreateShaderResourceView(m_sceneBuffer.renderTarget.Get(), &srvDesc, m_sceneShaderResourceView.GetAddressOf());
	CheckResult(hr, "씬 셰이더 리소스 뷰 생성 실패.");
}

void Renderer::SetViewport()
{
	D3D11_VIEWPORT viewport =
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
			.FillMode = D3D11_FILL_SOLID,
			.CullMode = D3D11_CULL_NONE
			// 백 버퍼용은 멀티샘플링 비활성화
		},

		// RSSolid
		D3D11_RASTERIZER_DESC
		{
			.FillMode = D3D11_FILL_SOLID,
			.CullMode = D3D11_CULL_BACK,
			.MultisampleEnable = TRUE,
			.AntialiasedLineEnable = TRUE
		},

		// RSWireframe
		D3D11_RASTERIZER_DESC
		{
			.FillMode = D3D11_FILL_WIREFRAME,
			.CullMode = D3D11_CULL_BACK,
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
			.MinLOD = 0,
			.MaxLOD = D3D11_FLOAT32_MAX
		},

		// SSScene
		D3D11_SAMPLER_DESC
		{
			.Filter = D3D11_FILTER_ANISOTROPIC,
			.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
			.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
			.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
			.MaxAnisotropy = 8,
			.ComparisonFunc = D3D11_COMPARISON_NEVER,
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

HRESULT Renderer::CompileShader(filesystem::path shaderName, _Out_ ID3DBlob** shaderCode, const char* shaderModel)
{
	HRESULT hr = S_OK;

	const filesystem::path shaderPath = L"../Assets/Shaders/" / shaderName;
	com_ptr<ID3DBlob> errorBlob = nullptr;

	hr = D3DCompileFromFile
	(
		shaderPath.wstring().c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		shaderModel,
		#ifdef _DEBUG
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		#else
		D3DCOMPILE_OPTIMIZATION_LEVEL3,
		#endif
		0,
		shaderCode,
		errorBlob.GetAddressOf()
	);
	if (errorBlob) cerr << shaderName.string() << " 셰이더 컴파일 오류: " << static_cast<const char*>(errorBlob->GetBufferPointer()) << endl;

	return hr;
}