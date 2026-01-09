///ResourceManager.h의 시작
#pragma once
#include "Singleton.h"

class ResourceManager : public Singleton<ResourceManager>
{
	friend class Singleton<ResourceManager>;

	com_ptr<ID3D11Device> m_device = nullptr; // 디바이스
	com_ptr<ID3D11DeviceContext> m_deviceContext = nullptr; // 디바이스 컨텍스트

	std::array<com_ptr<ID3D11DepthStencilState>, static_cast<size_t>(DepthStencilState::Count)> m_depthStencilStates = {}; // 깊이버퍼 상태 배열
	DepthStencilState m_currentDepthStencilState = DepthStencilState::Count; // 현재 깊이버퍼 상태

	std::array<com_ptr<ID3D11BlendState>, static_cast<size_t>(BlendState::Count)> m_blendStates = {}; // 블렌드 상태 배열
	BlendState m_currentBlendState = BlendState::Count; // 현재 블렌드 상태

	std::array<com_ptr<ID3D11RasterizerState>, static_cast<size_t>(RasterState::Count)> m_rasterStates = {}; // 래스터 상태 배열
	RasterState m_currentRasterState = RasterState::Count; // 현재 래스터 상태

	std::array<com_ptr<ID3D11Buffer>, static_cast<size_t>(VSConstBuffers::Count)> m_vsConstantBuffers = {}; // 정점 셰이더용 상수 버퍼 배열
	std::array<com_ptr<ID3D11Buffer>, static_cast<size_t>(PSConstBuffers::Count)> m_psConstantBuffers = {}; // 픽셀 셰이더용 상수 버퍼 배열

	std::array<com_ptr<ID3D11SamplerState>, static_cast<size_t>(SamplerState::Count)> m_samplerStates = {}; // 샘플러 상태 배열

	std::unordered_map<std::string, std::pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>>> m_vertexShadersAndInputLayouts = {}; // 정점 셰이더 및 입력 레이아웃 맵 // 키: 셰이더 파일 이름
	std::unordered_map<std::string, com_ptr<ID3D11PixelShader>> m_pixelShaders = {}; // 픽셀 셰이더 맵 // 키: 셰이더 파일 이름

	std::unordered_map<std::string, std::vector<uint8_t>> m_textureCaches = {}; // 텍스처 데이터 캐시 // 키: 텍스처 파일 이름
	std::unordered_map<std::string, com_ptr<ID3D11ShaderResourceView>> m_textures = {}; // 텍스처 맵 // 키: 텍스처 파일 이름

	std::unordered_map<std::string, Model> m_models = {}; // 모델 맵 // 키: 모델 파일 경로

public:
	~ResourceManager() = default;
	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;
	ResourceManager(ResourceManager&&) = delete;
	ResourceManager& operator=(ResourceManager&&) = delete;

	// 생성자 Renderer에서 호출
	void Initialize(com_ptr<ID3D11Device> device, com_ptr<ID3D11DeviceContext> deviceContext);

	// 깊이버퍼 상태 얻기
	com_ptr<ID3D11DepthStencilState> GetDepthStencilState(DepthStencilState state) { return m_depthStencilStates[static_cast<size_t>(state)]; }
	// 깊이버퍼 상태 설정
	void SetDepthStencilState(DepthStencilState state);

	// 블렌드 상태 얻기
	com_ptr<ID3D11BlendState> GetBlendState(BlendState state) { return m_blendStates[static_cast<size_t>(state)]; }
	// 블렌드 상태 설정
	void SetBlendState(BlendState state);

	// 상수 버퍼 얻기 // UpdateSubresource만 써야함 // Set는 리소스 매니저가 함
	com_ptr<ID3D11Buffer> GetConstantBuffer(VSConstBuffers buffer) { return m_vsConstantBuffers[static_cast<size_t>(buffer)]; }
	com_ptr<ID3D11Buffer> GetConstantBuffer(PSConstBuffers buffer) { return m_psConstantBuffers[static_cast<size_t>(buffer)]; }

	// 래스터 상태 설정
	void SetRasterState(RasterState state);

	// 버텍스 버퍼를 만들어서 리턴하는 함수 : 라인을 그리기 위함임
	com_ptr<ID3D11Buffer> CreateVertexBuffer(const void* data, UINT stride, UINT count, bool isDynamic = false);
	// 정점 셰이더 및 입력 레이아웃 얻기
	std::pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>> GetVertexShaderAndInputLayout(const std::string& shaderName, const std::vector<InputElement>& inputElements = {});
	// 픽셀 셰이더 얻기
	com_ptr<ID3D11PixelShader> GetPixelShader(const std::string& shaderName);
	// 텍스처 파일로부터 텍스처 로드
	com_ptr<ID3D11ShaderResourceView> GetTexture(const std::string& fileName);
	// 모델 파일로부터 모델 로드
	const Model* LoadModel(const std::string& fileName);

private:
	ResourceManager() = default;

	// 깊이버퍼 상태 생성 함수
	void CreateDepthStencilStates();
	// 블렌드 상태 생성 함수
	void CreateBlendStates();
	// 래스터 상태 생성 함수
	void CreateRasterStates();

	// 상수 버퍼 생성 및 설정 함수
	void CreateAndSetConstantBuffers();
	// 샘플러 상태 생성 및 설정 함수
	void CreateAndSetSamplerStates();

	// 텍스처 데이터 캐싱 함수
	void CacheAllTexture();

	// FBX 파일 로드 함수
	// 노드 처리 함수
	void ProcessNode(const aiNode* node, const aiScene* scene, Model& model);
	// 메쉬 처리 함수
	Mesh ProcessMesh(const aiMesh* mesh, const aiScene* scene);
	// 메쉬 버퍼(GPU) 생성 함수
	void CreateMeshBuffers(Mesh& mesh);

	// 셰이더 컴파일 함수
	com_ptr<ID3DBlob> CompileShader(const std::string& shaderName, const char* shaderModel);
};
///ResourceManager.h의 끝