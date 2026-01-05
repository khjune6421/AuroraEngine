/// ModelComponent.h의 시작
#pragma once
#include "ComponentBase.h"
#include "Resource.h"

class ModelComponent : public ComponentBase
{
	const struct Model* m_model = nullptr;

	com_ptr<ID3D11DeviceContext> m_deviceContext = nullptr; // 디바이스 컨텍스트

	std::pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>> m_vertexShaderAndInputLayout = {}; // 정점 셰이더 및 입력 레이아웃
	com_ptr<ID3D11PixelShader> m_pixelShader = nullptr; // 픽셀 셰이더

	com_ptr<ID3D11Buffer> m_materialConstantBuffer = nullptr; // 재질 상수 버퍼

	std::string m_modelFileName = "box.fbx"; // 기본 모델 파일 이름

	std::string m_vsShaderName = "VSModel.hlsl"; // 기본 모델 정점 셰이더
	std::string m_psShaderName = "PSModel.hlsl"; // 기본 모델 픽셀 셰이더

	// 입력 요소 배열 // 위치, UV, 법선, 접선
	std::vector<InputElement> m_inputElements =
	{
		InputElement::Position,
		InputElement::UV,
		InputElement::Normal,
		InputElement::Tangent
	};

public:
	ModelComponent() = default;
	~ModelComponent() override = default;
	ModelComponent(const ModelComponent&) = default;
	ModelComponent& operator=(const ModelComponent&) = default;
	ModelComponent(ModelComponent&&) = default;
	ModelComponent& operator=(ModelComponent&&) = default;

	const std::string& GetModelFileName() const { return m_modelFileName; }
	void SetModelFileName(const std::string& modelFileName) { m_modelFileName = modelFileName; }

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

	nlohmann::json Serialize() override;
	void Deserialize(const nlohmann::json& jsonData) override;

	// 셰이더 생성
	void CreateShaders();
};
/// ModelComponent.h의 끝