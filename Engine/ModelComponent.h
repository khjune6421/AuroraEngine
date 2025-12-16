#pragma once
#include "ComponentBase.h"

class ModelComponent : public ComponentBase
{
	const struct Model* m_model = nullptr;

	std::pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>> m_vertexShaderAndInputLayout = {}; // 정점 셰이더 및 입력 레이아웃
	com_ptr<ID3D11PixelShader> m_pixelShader = nullptr; // 픽셀 셰이더

	std::string m_vsShaderName = "VSVertexColor.hlsl"; // 기본 정점 셰이더
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
			.SemanticName = "NORMAL",
			.SemanticIndex = 0,
			.Format = DXGI_FORMAT_R32G32B32_FLOAT, // float3
			.InputSlot = 0,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		},
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
	std::string m_psShaderName = "PSVertexColor.hlsl"; // 기본 픽셀 셰이더

public:
	ModelComponent() = default;
	~ModelComponent() override = default;
	ModelComponent(const ModelComponent&) = default;
	ModelComponent& operator=(const ModelComponent&) = default;
	ModelComponent(ModelComponent&&) = default;
	ModelComponent& operator=(ModelComponent&&) = default;

	void Render();

protected:
	void Begin() override;

private:
	// 셰이더 생성
	void CreateShaders();
};