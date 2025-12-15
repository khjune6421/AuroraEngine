#pragma once
#include "ComponentBase.h"

class ModelComponent : public ComponentBase
{
	struct RenderData // 렌더 정보 구조체
	{
		struct Vertex // 정점 구조체
		{
			DirectX::XMFLOAT4 position = {}; // 위치
			DirectX::XMFLOAT4 color = {}; // 색상
		};
		std::vector<Vertex> vertices = // 정점 테이터 // 기본값: 정육면체
		{
			{.position = { -1.0f,  1.0f, -1.0f, 1.0f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f } }, // 앞면 왼쪽 위
			{.position = {  1.0f,  1.0f, -1.0f, 1.0f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f } }, // 앞면 오른쪽 위
			{.position = {  1.0f, -1.0f, -1.0f, 1.0f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f } }, // 앞면 오른쪽 아래
			{.position = { -1.0f, -1.0f, -1.0f, 1.0f }, .color = { 1.0f, 1.0f, 0.0f, 1.0f } }, // 앞면 왼쪽 아래
			{.position = { -1.0f,  1.0f,  1.0f, 1.0f }, .color = { 1.0f, 0.0f, 1.0f, 1.0f } }, // 뒷면 왼쪽 위
			{.position = {  1.0f,  1.0f,  1.0f, 1.0f }, .color = { 0.0f, 1.0f, 1.0f, 1.0f } }, // 뒷면 오른쪽 위
			{.position = {  1.0f, -1.0f,  1.0f, 1.0f }, .color = { 1.0f, 1.0f, 1.0f, 1.0f } }, // 뒷면 오른쪽 아래
			{.position = { -1.0f, -1.0f,  1.0f, 1.0f }, .color = { 0.0f, 0.0f, 0.0f, 1.0f } }  // 뒷면 왼쪽 아래
		};
		std::vector<UINT> indices = // 인덱스 데이터 // 기본값: 정육면체
		{
			0, 1, 2, 0, 2, 3,       // 앞면
			4, 6, 5, 4, 7, 6,       // 뒷면
			4, 5, 1, 4, 1, 0,       // 윗면
			3, 2, 6, 3, 6, 7,       // 아랫면
			1, 5, 6, 1, 6, 2,       // 오른쪽 면
			4, 0, 3, 4, 3, 7        // 왼쪽 면
		};
		std::string vsShaderName = "VSVertexColor.hlsl"; // 기본 버텍스 셰이더
		std::vector<D3D11_INPUT_ELEMENT_DESC> m_inputElementDescs = // 입력 레이아웃 배열 // 기본값: Vertex 구조체에 맞춤
		{
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
			D3D11_INPUT_ELEMENT_DESC
			{
				.SemanticName = "COLOR",
				.SemanticIndex = 0,
				.Format = DXGI_FORMAT_R32G32B32A32_FLOAT, // float4
				.InputSlot = 0,
				.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
				.InstanceDataStepRate = 0
			}
		};
		std::string psShaderName = "PSVertexColor.hlsl"; // 기본 픽셀 셰이더
	};

	// 렌더링 관련 멤버 변수
	RenderData m_renderData = {}; // 렌더 정보 // 이걸로 리소스 생성
	com_ptr<ID3D11Buffer> m_vertexBuffer = nullptr; // 버텍스 버퍼
	com_ptr<ID3D11Buffer> m_indexBuffer = nullptr; // 인덱스 버퍼
	UINT m_indexCount = 0; // 인덱스 개수
	std::pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>> m_vertexShaderAndInputLayout = {}; // 버텍스 셰이더 및 입력 레이아웃
	com_ptr<ID3D11PixelShader> m_pixelShader = nullptr; // 픽셀 셰이더

public:
	ModelComponent() = default;
	~ModelComponent() override = default;
	ModelComponent(const ModelComponent&) = default;
	ModelComponent& operator=(const ModelComponent&) = default;
	ModelComponent(ModelComponent&&) = default;
	ModelComponent& operator=(ModelComponent&&) = default;

	void Render();

	void SetRenderData(const RenderData& renderData) { m_renderData = renderData; }
	const RenderData& GetRenderData() const { return m_renderData; }

protected:
	void Begin() override;

private:
	// 버텍스 버퍼 생성
	void CreateVertexBuffer();
	// 인덱스 버퍼 생성
	void CreateIndexBuffer();
	// 셰이더 생성
	void CreateShaders();
};