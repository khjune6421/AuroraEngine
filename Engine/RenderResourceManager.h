#pragma once

enum RasterState
{
	RSBackBuffer, // 백 버퍼 전용 래스터 상태 // AA 없음
	RSSolid,
	RSWireframe,

	RSCount
};
constexpr std::array<D3D11_RASTERIZER_DESC, RSCount> RASTERIZER_DESC_TEMPLATES =
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

enum SamplerState
{
	SSBackBuffer, // 백 버퍼 전용 샘플러 상태
	SSScene,

	SSCount
};
constexpr std::array<D3D11_SAMPLER_DESC, SSCount> SAMPLER_DESC_TEMPLATES =
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

class RenderResourceManager : public SingletonBase<RenderResourceManager>
{
	friend class SingletonBase<RenderResourceManager>;

	com_ptr<ID3D11Device> m_device = nullptr; // 디바이스

	std::array<com_ptr<ID3D11RasterizerState>, RSCount> m_rasterStates = {}; // 래스터 상태 배열
	std::array<com_ptr<ID3D11SamplerState>, SSCount> m_samplerStates = {}; // 샘플러 상태 배열

	std::unordered_map<UINT, com_ptr<ID3D11Buffer>> m_constantBuffers = {}; // 상수 버퍼 맵 // 키: 버퍼 크기

	std::unordered_map<std::wstring, std::pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>>> m_vertexShadersAndInputLayouts = {}; // 버텍스 셰이더 및 입력 레이아웃 맵 // 키: 셰이더 파일 이름
	std::unordered_map<std::wstring, com_ptr<ID3D11PixelShader>> m_pixelShaders = {}; // 픽셀 셰이더 맵 // 키: 셰이더 파일 이름

public:
	RenderResourceManager() = default;
	~RenderResourceManager() = default;
	RenderResourceManager(const RenderResourceManager&) = delete;
	RenderResourceManager& operator=(const RenderResourceManager&) = delete;
	RenderResourceManager(RenderResourceManager&&) = delete;
	RenderResourceManager& operator=(RenderResourceManager&&) = delete;

	// 생성자 Renderer에서 호출
	void Initialize(com_ptr<ID3D11Device> device);

	// 자원 획득 함수들
	// 래스터 상태 얻기
	com_ptr<ID3D11RasterizerState> GetRasterState(RasterState state) { return m_rasterStates[state]; }
	// 샘플러 상태 얻기
	com_ptr<ID3D11SamplerState> GetSamplerState(SamplerState state) { return m_samplerStates[state]; }
	// 상수 버퍼 얻기 // 이미 생성된 버퍼가 있으면 재사용 // 없으면 새로 생성
	com_ptr<ID3D11Buffer> GetConstantBuffer(UINT bufferSize);
	// 버텍스 셰이더 및 입력 레이아웃 얻기
	std::pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>> GetVertexShaderAndInputLayout(std::wstring shaderName, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElementDescs);
	// 픽셀 셰이더 얻기
	com_ptr<ID3D11PixelShader> GetPixelShader(std::wstring shaderName);

private:
	// 래스터 상태 및 샘플러 상태 생성 함수
	void CreateRasterStates();
	void CreateSamplerStates();

	// 셰이더 컴파일 함수
	com_ptr<ID3DBlob> CompileShader(std::filesystem::path shaderName, const char* shaderModel);
};