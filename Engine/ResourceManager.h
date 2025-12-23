#pragma once
#include "Resource.h"

class ResourceManager : public SingletonBase<ResourceManager>
{
	friend class SingletonBase<ResourceManager>;

	com_ptr<ID3D11Device> m_device = nullptr; // 디바이스
	com_ptr<ID3D11DeviceContext> m_deviceContext = nullptr; // 디바이스 컨텍스트

	std::array<com_ptr<ID3D11RasterizerState>, static_cast<size_t>(RasterState::Count)> m_rasterStates = {}; // 래스터 상태 배열
	std::array<com_ptr<ID3D11SamplerState>, static_cast<size_t>(SamplerState::Count)> m_samplerStates = {}; // 샘플러 상태 배열

	std::unordered_map<std::string, std::pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>>> m_vertexShadersAndInputLayouts = {}; // 정점 셰이더 및 입력 레이아웃 맵 // 키: 셰이더 파일 이름
	std::unordered_map<std::string, com_ptr<ID3D11PixelShader>> m_pixelShaders = {}; // 픽셀 셰이더 맵 // 키: 셰이더 파일 이름

	std::unordered_map<std::string, std::vector<uint8_t>> m_textureCaches = {}; // 텍스처 데이터 캐시 // 키: 텍스처 파일 이름
	std::unordered_map<std::string, com_ptr<ID3D11ShaderResourceView>> m_textures = {}; // 텍스처 맵 // 키: 텍스처 파일 이름

	std::unordered_map<std::string, Model> m_models = {}; // 모델 맵 // 키: 모델 파일 경로

public:
	ResourceManager() = default;
	~ResourceManager() = default;
	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;
	ResourceManager(ResourceManager&&) = delete;
	ResourceManager& operator=(ResourceManager&&) = delete;

	// 생성자 Renderer에서 호출
	void Initialize(com_ptr<ID3D11Device> device, com_ptr<ID3D11DeviceContext> deviceContext);

	// 자원 획득 함수들
	// 래스터 상태 얻기
	com_ptr<ID3D11RasterizerState> GetRasterState(RasterState state) { return m_rasterStates[static_cast<size_t>(state)]; }
	// 샘플러 상태 얻기
	com_ptr<ID3D11SamplerState> GetSamplerState(SamplerState state) { return m_samplerStates[static_cast<size_t>(state)]; }
	// 상수 버퍼 얻기 // 이미 생성된 버퍼가 있으면 재사용 // 없으면 새로 생성
	com_ptr<ID3D11Buffer> GetConstantBuffer(UINT bufferSize);
	// 정점 셰이더 및 입력 레이아웃 얻기
	std::pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>> GetVertexShaderAndInputLayout(const std::string& shaderName, const std::vector<InputElement>& inputElements);
	// 픽셀 셰이더 얻기
	com_ptr<ID3D11PixelShader> GetPixelShader(const std::string& shaderName);
	// 텍스처 파일로부터 텍스처 로드
	com_ptr<ID3D11ShaderResourceView> GetTexture(const std::string& fileName);
	// 모델 파일로부터 모델 로드
	const Model* LoadModel(const std::string& fileName);

private:
	// 래스터 상태 생성 함수
	void CreateRasterStates();
	// 샘플러 상태 생성 함수
	void CreateSamplerStates();
	// 텍스처 데이터 캐싱 함수
	void CacheAllTexture();

	// FBX 파일 로드 함수
	// 노드 처리 함수
	void ProcessNode(const aiNode* node, const aiScene* scene, Model& model);
	// 메쉬 처리 함수
	Mesh ProcessMesh(const aiMesh* mesh, const aiScene* scene);
	// 재질 처리 함수
	MaterialFactor ProcessMaterialFactor(aiMaterial* material);
	// 메쉬 버퍼(GPU) 생성 함수
	void CreateMeshBuffers(Mesh& mesh);

	// 셰이더 컴파일 함수
	com_ptr<ID3DBlob> CompileShader(const std::string& shaderName, const char* shaderModel);
};