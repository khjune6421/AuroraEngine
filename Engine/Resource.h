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

enum InputElement
{
	Position,
	Normal,
	Tangent,
	UV,

	InputElementCount
};
constexpr std::array<D3D11_INPUT_ELEMENT_DESC, InputElementCount> INPUT_ELEMENT_DESC_TEMPLATES =
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
	}
};

struct Vertex
{
	DirectX::XMFLOAT4 position = {};
	DirectX::XMFLOAT3 normal = {};
	DirectX::XMFLOAT3 tangent = {};
	DirectX::XMFLOAT2 UV = {};
};

struct MeshBoundingBox
{
	DirectX::XMFLOAT3 min = {};
	DirectX::XMFLOAT3 max = {};
};

struct Mesh
{
	std::vector<Vertex> vertices = {};
	std::vector<UINT> indices = {};
	UINT indexCount = 0;

	MeshBoundingBox boundingBox = {};

	com_ptr<ID3D11Buffer> vertexBuffer = nullptr;
	com_ptr<ID3D11Buffer> indexBuffer = nullptr;
};

struct Model
{
	std::vector<Mesh> meshes = {};
};