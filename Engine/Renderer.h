#pragma once

class Renderer
{
	HWND m_hWnd = nullptr;

	const std::array<D3D_FEATURE_LEVEL, 3> m_featureLevels = // 지원할 Direct3D 버전
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};

	DXGI_SWAP_CHAIN_DESC1 m_swapChainDesc = // 스왑 체인 설정
	{
		.Width = 1280,
		.Height = 720,
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, // 감마 보정
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = 2,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	};

	com_ptr<ID3D11Device> m_device = nullptr; // 디바이스
	com_ptr<ID3D11DeviceContext> m_deviceContext = nullptr; // 디바이스 컨텍스트
	com_ptr<IDXGISwapChain1> m_swapChain = nullptr; // 스왑 체인

	struct RenderTarget
	{
		com_ptr<ID3D11Texture2D> renderTarget = nullptr; // 렌더 타겟 텍스처
		com_ptr<ID3D11RenderTargetView> renderTargetView = nullptr; // 렌더 타겟 뷰

		com_ptr<ID3D11Texture2D> depthStencilTexture = nullptr; // 깊이-스텐실 텍스처
		com_ptr<ID3D11DepthStencilView> depthStencilView = nullptr; // 깊이-스텐실 뷰
	};

	RenderTarget m_backBuffer; // 백 버퍼 렌더 타겟 // 화면에 출력되는 버퍼 // UI만을 직접적으로 랜더
	com_ptr<ID3D11Buffer> m_backBufferVertexBuffer = nullptr; // 백 버퍼용 버텍스 버퍼
	com_ptr<ID3D11VertexShader> m_backBufferVertexShader = nullptr; // 백 버퍼용 버텍스 셰이더
	com_ptr<ID3D11InputLayout> m_backBufferInputLayout = nullptr; // 백 버퍼용 입력 레이아웃
	com_ptr<ID3D11PixelShader> m_backBufferPixelShader = nullptr; // 백 버퍼용 후처리 픽셀 셰이더

	RenderTarget m_sceneBuffer; // 씬 렌더 타겟 // 실제 게임 씬을 랜더링하는 버퍼
	com_ptr<ID3D11ShaderResourceView> m_sceneShaderResourceView = nullptr; // 씬 렌더 타겟의 셰이더 리소스 뷰 // 백 버퍼에 적용하면서 후처리됨

	enum RasterState
	{
		RSBackBuffer, // 백 버퍼 전용 래스터 상태 // AA 없음
		RSSolid,
		RSWireframe,

		RSCount
	};
	RasterState m_rasterState = RSSolid;
	std::array<com_ptr<ID3D11RasterizerState>, RSCount> m_rasterStates = {}; // 래스터 상태 배열

	enum SamplerState
	{
		SSBackBuffer, // 백 버퍼 전용 샘플러 상태
		SSScene,

		SSCount
	};
	std::array<com_ptr<ID3D11SamplerState>, SSCount> m_samplerStates = {}; // 샘플러 상태 배열

public:
	Renderer(HWND hwnd);
	~Renderer();
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator=(Renderer&&) = delete;

	// 렌더러 초기화 // 렌더러 사용 전 반드시 호출해야 함
	void Initialize();

	// 화면 크기 조정
	HRESULT Resize(UINT width, UINT height);

	// 스왑 체인 설정 조회 및 편집
	void GetSwapChainDesc(_Out_ DXGI_SWAP_CHAIN_DESC1& desc) const { desc = m_swapChainDesc; }

	// 레스터 상태 설정 및 조회
	HRESULT SetRasterState(RasterState state);
	RasterState GetRasterState() const { return m_rasterState; }

private:
	// 디바이스 및 디바이스 컨텍스트 생성
	void CreateDeviceAndContext();
	// 스왑 체인 생성
	void CreateSwapChain();
	// 백 버퍼 렌더 타겟 생성
	void CreateBackBufferRenderTarget();
	// 백 버퍼 셰이더 및 상수 버퍼 생성
	void CreateBackBufferVertexBufferAndShaders();
	// 씬 렌더 타겟 생성
	void CreateSceneRenderTarget();
	// 뷰포트 설정
	void SetViewport();
	// 래스터 상태 생성
	void CreateRasterStates();
	// 샘플러 상태 생성
	void CreateSamplerStates();

	// 헬퍼 함수
	// HRESULT 결과 확인
	void CheckResult(HRESULT hr, const char* msg) const;
	// 셰이더 컴파일
	HRESULT CompileShader(std::filesystem::path shaderName, _Out_ ID3DBlob** shaderCode, const char* shaderModel);
};