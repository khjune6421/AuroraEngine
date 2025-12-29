#pragma once

struct RenderTarget
{
	com_ptr<ID3D11Texture2D> renderTarget = nullptr; // 렌더 타겟 텍스처
	com_ptr<ID3D11RenderTargetView> renderTargetView = nullptr; // 렌더 타겟 뷰

	com_ptr<ID3D11Texture2D> depthStencilTexture = nullptr; // 깊이-스텐실 텍스처
	com_ptr<ID3D11DepthStencilView> depthStencilView = nullptr; // 깊이-스텐실 뷰
};

enum class DepthStencilState
{
	Skybox,
	Count
};
constexpr std::array<D3D11_DEPTH_STENCIL_DESC, static_cast<size_t>(DepthStencilState::Count)> DEPTH_STENCIL_DESC_TEMPLATES =
{
	// Skybox
	D3D11_DEPTH_STENCIL_DESC
	{
		.DepthEnable = TRUE, // 깊이 테스트 활성화
		.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO, // 깊이 쓰기 활성화
		.DepthFunc = D3D11_COMPARISON_LESS_EQUAL, // 깊이 비교 함수: 작거나 같음
		.StencilEnable = FALSE, // 스텐실 테스트 비활성화
		.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
		.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
		.FrontFace = {}, // 사용 안 함
		.BackFace = {} // 사용 안 함
	}
};

enum class RasterState
{
	BackBuffer, // 백 버퍼 전용 래스터 상태 // AA 없음
	Solid,
	Wireframe,

	Count
};
constexpr std::array<D3D11_RASTERIZER_DESC, static_cast<size_t>(RasterState::Count)> RASTERIZER_DESC_TEMPLATES =
{
	// BackBuffer
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

	// Solid
	D3D11_RASTERIZER_DESC
	{
		.FillMode = D3D11_FILL_SOLID,
		.CullMode = D3D11_CULL_BACK, // 뒷면 컬링(CCW)
		.FrontCounterClockwise = FALSE,
		.DepthBias = 0,
		.DepthBiasClamp = 0.0f,
		.SlopeScaledDepthBias = 0.0f,
		.DepthClipEnable = TRUE,
		.ScissorEnable = FALSE,
		.MultisampleEnable = TRUE,
		.AntialiasedLineEnable = TRUE
	},

	// Wireframe
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

enum class SamplerState
{
	BackBuffer, // 백 버퍼 전용 샘플러 상태
	Default,

	Count
};
constexpr std::array<D3D11_SAMPLER_DESC, static_cast<size_t>(SamplerState::Count)> SAMPLER_DESC_TEMPLATES =
{
	// BackBuffer
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

	// Default
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

enum class InputElement
{
	Position,
	UV,
	Normal,
	Tangent,

	Count
};
constexpr std::array<D3D11_INPUT_ELEMENT_DESC, static_cast<size_t>(InputElement::Count)> INPUT_ELEMENT_DESC_TEMPLATES =
{
	// Position
	D3D11_INPUT_ELEMENT_DESC
	{
		.SemanticName = "POSITION",
		.SemanticIndex = 0,
		.Format = DXGI_FORMAT_R32G32B32A32_FLOAT, // float4
		.InputSlot = 0,
		.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT, // 자동 오프셋 계산
		.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
		.InstanceDataStepRate = 0
	},

	// UV
	D3D11_INPUT_ELEMENT_DESC
	{
		.SemanticName = "TEXCOORD",
		.SemanticIndex = 0,
		.Format = DXGI_FORMAT_R32G32_FLOAT, // float2
		.InputSlot = 0,
		.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
		.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
		.InstanceDataStepRate = 0
	},

	// Normal
	D3D11_INPUT_ELEMENT_DESC
	{
		.SemanticName = "NORMAL",
		.SemanticIndex = 0,
		.Format = DXGI_FORMAT_R32G32B32_FLOAT, // float3
		.InputSlot = 0,
		.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
		.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
		.InstanceDataStepRate = 0
	},

	// Tangent
	D3D11_INPUT_ELEMENT_DESC
	{
		.SemanticName = "TANGENT",
		.SemanticIndex = 0,
		.Format = DXGI_FORMAT_R32G32B32_FLOAT, // float3
		.InputSlot = 0,
		.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
		.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
		.InstanceDataStepRate = 0
	}
};

enum class VSConstBuffers
{
	ViewProjection,
	WorldNormal
};
enum class PSConstBuffers
{
	CameraPosition,
	DirectionalLight,
	Material
};
enum class TextureSlots
{
	BackBuffer,
	Environment,
	Albedo,
	Normal,
	Metallic,
	Roughness,
	AmbientOcclusion
};

struct Vertex
{
	DirectX::XMFLOAT4 position = {};
	DirectX::XMFLOAT2 UV = {};
	DirectX::XMFLOAT3 normal = {};
	DirectX::XMFLOAT3 tangent = {};
};

struct MaterialFactor
{
	DirectX::XMFLOAT4 albedoFactor = { 1.0f, 1.0f, 1.0f, 1.0f }; // 텍스처 색상이 얼마나 적용되는지
	float metallicFactor = 1.0f; // 금속성 텍스처가 얼마나 적용되는지
	float roughnessFactor = 1.0f; // 거칠기 텍스처가 얼마나 적용되는지
	float ambientOcclusionFactor = 1.0f; // 환경광 차폐 텍스처가 얼마나 적용되는지
	float lightFactor = 1.0f; // 조명 영향도 // 일반적으로 1.0f 여야 함
	DirectX::XMFLOAT4 emissionFactor = { 0.0f, 0.0f, 0.0f, 0.0f }; // 자가 발광 색상 팩터
};

struct MaterialTexture
{
	com_ptr<ID3D11ShaderResourceView> albedoTextureSRV = nullptr;
	com_ptr<ID3D11ShaderResourceView> normalTextureSRV = nullptr;
	com_ptr<ID3D11ShaderResourceView> metallicTextureSRV = nullptr;
	com_ptr<ID3D11ShaderResourceView> roughnessTextureSRV = nullptr;
	com_ptr<ID3D11ShaderResourceView> ambientOcclusionTextureSRV = nullptr;
};

struct Mesh
{
	std::vector<Vertex> vertices = {};
	std::vector<UINT> indices = {};
	UINT indexCount = 0;

	DirectX::BoundingBox boundingBox = {};

	com_ptr<ID3D11Buffer> vertexBuffer = nullptr;
	com_ptr<ID3D11Buffer> indexBuffer = nullptr;

	MaterialFactor materialFactor = {};
	MaterialTexture materialTexture = {};
};

struct Model
{
	std::vector<Mesh> meshes = {};
};