#pragma once
#include "Resource.h"

class RenderResourceManager : public SingletonBase<RenderResourceManager>
{
	friend class SingletonBase<RenderResourceManager>;

	com_ptr<ID3D11Device> m_device = nullptr; // 디바이스

	std::array<com_ptr<ID3D11RasterizerState>, RSCount> m_rasterStates = {}; // 래스터 상태 배열
	std::array<com_ptr<ID3D11SamplerState>, SSCount> m_samplerStates = {}; // 샘플러 상태 배열

	std::unordered_map<UINT, com_ptr<ID3D11Buffer>> m_constantBuffers = {}; // 상수 버퍼 맵 // 키: 버퍼 크기

	std::unordered_map<std::string, std::pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>>> m_vertexShadersAndInputLayouts = {}; // 버텍스 셰이더 및 입력 레이아웃 맵 // 키: 셰이더 파일 이름
	std::unordered_map<std::string, com_ptr<ID3D11PixelShader>> m_pixelShaders = {}; // 픽셀 셰이더 맵 // 키: 셰이더 파일 이름

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
	std::pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>> GetVertexShaderAndInputLayout(std::string shaderName, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElementDescs);
	// 픽셀 셰이더 얻기
	com_ptr<ID3D11PixelShader> GetPixelShader(std::string shaderName);
	// 모델 파일로부터 모델 로드

private:
	// 래스터 상태 생성 함수
	void CreateRasterStates();
	// 샘플러 상태 생성 함수
	void CreateSamplerStates();

	// FBX 파일 로드 함수

	// 셰이더 컴파일 함수
	com_ptr<ID3DBlob> CompileShader(std::string shaderName, const char* shaderModel);
};