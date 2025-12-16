#pragma once
#include "ComponentBase.h"
#include "Resource.h"

class ModelComponent : public ComponentBase
{
	const struct Model* m_model = nullptr;

	std::pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>> m_vertexShaderAndInputLayout = {}; // 정점 셰이더 및 입력 레이아웃
	com_ptr<ID3D11PixelShader> m_pixelShader = nullptr; // 픽셀 셰이더

	std::string m_modelFileName = "box.fbx"; // 기본 모델 파일 이름

	std::string m_vsShaderName = "VSModel.hlsl"; // 기본 모델 정점 셰이더
	std::string m_psShaderName = "PSModel.hlsl"; // 기본 모델 픽셀 셰이더

	// 입력 레이아웃 목록 // 위치, 노말, 접선, UV
	std::vector<InputElement> m_inputElements =
	{
		InputElement::Position,
		InputElement::Normal,
		InputElement::Tangent,
		InputElement::UV
	};

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