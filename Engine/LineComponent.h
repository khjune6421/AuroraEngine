/// LineComponent.h의 시작
#pragma once
#include "ComponentBase.h"


class LineComponent : public ComponentBase
{
	com_ptr<ID3D11DeviceContext> m_deviceContext = nullptr; 

	std::pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>> m_vertexShaderAndInputLayout = {}; // 정점 셰이더 및 입력 레이아웃
	com_ptr<ID3D11PixelShader> m_pixelShader = nullptr; // 픽셀 셰이더
	com_ptr<ID3D11Buffer> m_vertexBuffer = nullptr; //정점 버퍼

	std::string m_vsShaderName = "VSLine.hlsl"; // 기본 라인 정점 셰이더
	std::string m_psShaderName = "PSLine.hlsl"; // 기본 라인 픽셀 셰이더
	
	// 입력 요소 배열 // 위치, COLOR
	std::vector<InputElement> m_inputElements =
	{
		InputElement::Position,
	};

public:
	LineComponent() = default;
	~LineComponent() override = default;
	LineComponent(const LineComponent&) = default;
	LineComponent& operator=(const LineComponent&) = default;
	LineComponent(LineComponent&&) = default;
	LineComponent& operator=(LineComponent&&) = default;


	const std::string& GetVertexShaderName() const { return m_vsShaderName; }
	void SetVertexShaderName(const std::string& vsShaderName) { m_vsShaderName = vsShaderName; }
	const std::string& GetPixelShaderName() const { return m_psShaderName; }
	void SetPixelShaderName(const std::string& psShaderName) { m_psShaderName = psShaderName; }

	bool NeedsUpdate() const override { return false; }
	bool NeedsRender() const override { return true; }

private:
	void Initialize() override;
	void Render() override;
	void RenderImGui() override;
	// 셰이더 생성
	void CreateShaders();
};
/// LineComponent.h의 끝