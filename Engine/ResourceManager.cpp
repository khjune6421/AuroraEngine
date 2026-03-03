#include "stdafx.h"
#include "ResourceManager.h"

using namespace std;
using namespace DirectX;

namespace
{
	string ToLowerAscii(string text)
	{
		for (char& c : text)
		{
			c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
		}
		return text;
	}

	bool IsRootMotionChannel(const string& boneName, const string& rootNodeName)
	{
		if (boneName == rootNodeName) return true;

		const string lower = ToLowerAscii(boneName);
		if (lower.find("root") != string::npos) return true;
		if (lower == "hips" || lower.find("hip") != string::npos) return true;

		return false;
	}

	void RemoveHorizontalRootMotion(BoneAnimationChannel& channel)
	{
		for (auto& key : channel.position_keys)
		{
			key.value.x = 0.0f;
			key.value.z = 0.0f;
		}
	}
}

XMFLOAT4X4 ResourceManager::ToXMFLOAT4X4(const aiMatrix4x4& matrix)
{
	return XMFLOAT4X4(
		matrix.a1, matrix.b1, matrix.c1, matrix.d1,
		matrix.a2, matrix.b2, matrix.c2, matrix.d2,
		matrix.a3, matrix.b3, matrix.c3, matrix.d3,
		matrix.a4, matrix.b4, matrix.c4, matrix.d4
	);
}

XMFLOAT3 ResourceManager::ToXMFLOAT3(const aiVector3D& vec3)
{
	return XMFLOAT3(vec3.x, vec3.y, vec3.z);
}

XMFLOAT4 ResourceManager::ToXMFLOAT4(const aiQuaternion& quar)
{
	return XMFLOAT4(quar.x, quar.y, quar.z, quar.w);
}

bool ResourceManager::SceneHasBones(const aiScene* scene)
{
	if (!scene) return false;
	for (UINT i = 0; i < scene->mNumMeshes; ++i)
	{
		if (scene->mMeshes[i]->HasBones()) return true;
	}
	return false;
}


void ResourceManager::Initialize(com_ptr<ID3D11Device> device, com_ptr<ID3D11DeviceContext> deviceContext)
{
	m_device = device;
	m_deviceContext = deviceContext;
	m_spriteBatch = make_unique<SpriteBatch>(m_deviceContext.Get());

	CreateDepthStencilStates();
	CreateBlendStates();
	CreateRasterStates();

	CreateConstantBuffers();
	SetAllConstantBuffers();
	CreateSamplerStates();
	SetAllSamplerStates();

	CacheAllTexture();
}

void ResourceManager::SetDepthStencilState(DepthStencilState state)
{
	if (m_currentDepthStencilState == state) return;

	m_deviceContext->OMSetDepthStencilState(m_depthStencilStates[static_cast<size_t>(state)].Get(), 0);
	m_currentDepthStencilState = state;
}

void ResourceManager::SetBlendState(BlendState state)
{
	if (m_currentBlendState == state) return;

	constexpr array<FLOAT, 4> blendFactor = { 1.0f, 1.0f, 1.0f, 1.0f }; // 나중에 따로 받도록 수정?
	m_deviceContext->OMSetBlendState(m_blendStates[static_cast<size_t>(state)].Get(), blendFactor.data(), 0xFFFFFFFF);
	m_currentBlendState = state;
}

void ResourceManager::SetRasterState(RasterState state)
{
	if (m_currentRasterState == state) return;

	m_deviceContext->RSSetState(m_rasterStates[static_cast<size_t>(state)].Get());
	m_currentRasterState = state;
}

void ResourceManager::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	if (m_currentTopology == topology) return;

	m_deviceContext->IASetPrimitiveTopology(topology);
	m_currentTopology = topology;
}

void ResourceManager::SetAllConstantBuffers()
{
	// 정점 셰이더용 상수 버퍼 설정
	for (size_t i = 0; i < static_cast<size_t>(VSConstBuffers::Count); ++i) m_deviceContext->VSSetConstantBuffers(static_cast<UINT>(i), 1, m_vsConstantBuffers[i].GetAddressOf());

	// 지오메트리 셰이더용 상수 버퍼 설정
	for (size_t i = 0; i < static_cast<size_t>(GSConstBuffers::Count); ++i) m_deviceContext->GSSetConstantBuffers(static_cast<UINT>(i), 1, m_gsConstantBuffers[i].GetAddressOf());

	// 픽셀 셰이더용 상수 버퍼 설정
	for (size_t i = 0; i < static_cast<size_t>(PSConstBuffers::Count); ++i) m_deviceContext->PSSetConstantBuffers(static_cast<UINT>(i), 1, m_psConstantBuffers[i].GetAddressOf());
}

void ResourceManager::SetAllSamplerStates()
{
	for (size_t i = 0; i < static_cast<size_t>(SamplerState::Count); ++i) m_deviceContext->PSSetSamplers(static_cast<UINT>(i), 1, m_samplerStates[i].GetAddressOf());
}

com_ptr<ID3D11Buffer> ResourceManager::CreateVertexBuffer(const void* data, UINT stride, UINT count, bool isDynamic)
{
	HRESULT hr = S_OK;
	com_ptr<ID3D11Buffer> vertexBuffer = nullptr;

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = stride * count;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	if (isDynamic)
	{
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC; 
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		bufferDesc.Usage = D3D11_USAGE_DEFAULT; 
		bufferDesc.CPUAccessFlags = 0;
	}

	if (data != nullptr)
	{
		D3D11_SUBRESOURCE_DATA initialData = {};
		initialData.pSysMem = data;
		initialData.SysMemPitch = 0;
		initialData.SysMemSlicePitch = 0;

		hr = m_device->CreateBuffer(&bufferDesc, &initialData, vertexBuffer.GetAddressOf());
	}
	else
	{
		hr = m_device->CreateBuffer(&bufferDesc, nullptr, vertexBuffer.GetAddressOf());
	}

	CheckResult(hr, "범용 정점 버퍼 생성 실패.");

	return vertexBuffer;
}

pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>> ResourceManager::GetVertexShaderAndInputLayout(const string& shaderName, const vector<InputElement>& inputElements)
{
	// 기존에 생성된 셰이더 및 입력 레이아웃이 있으면 재사용
	auto it = m_vertexShadersAndInputLayouts.find(shaderName);
	if (it != m_vertexShadersAndInputLayouts.end()) return it->second;

	HRESULT hr = S_OK;

	// 정점 셰이더 컴파일
	com_ptr<ID3DBlob> VSCode = CompileShader(shaderName, "vs_5_0");
	hr = m_device->CreateVertexShader
	(
		VSCode->GetBufferPointer(),
		VSCode->GetBufferSize(),
		nullptr,
		m_vertexShadersAndInputLayouts[shaderName].first.GetAddressOf()
	);
	CheckResult(hr, "정점 셰이더 생성 실패.");

	// 입력 레이아웃 생성
	if (!inputElements.empty())
	{
		vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescs;
		for (const auto& element : inputElements) inputElementDescs.push_back(INPUT_ELEMENT_DESC_TEMPLATES[static_cast<size_t>(element)]);

		hr = m_device->CreateInputLayout
		(
			inputElementDescs.data(),
			static_cast<UINT>(inputElementDescs.size()),
			VSCode->GetBufferPointer(),
			VSCode->GetBufferSize(),
			m_vertexShadersAndInputLayouts[shaderName].second.GetAddressOf()
		);
		CheckResult(hr, "입력 레이아웃 생성 실패.");
	}

	return m_vertexShadersAndInputLayouts[shaderName];
}

com_ptr<ID3D11GeometryShader> ResourceManager::GetGeometryShader(const string& shaderName)
{
	// 기존에 생성된 지오메트리 셰이더가 있으면 재사용
	auto it = m_geometryShaders.find(shaderName);
	if (it != m_geometryShaders.end()) return it->second;

	HRESULT hr = S_OK;

	// 지오메트리 셰이더 컴파일
	com_ptr<ID3DBlob> GSCode = CompileShader(shaderName, "gs_5_0");
	hr = m_device->CreateGeometryShader
	(
		GSCode->GetBufferPointer(),
		GSCode->GetBufferSize(),
		nullptr,
		m_geometryShaders[shaderName].GetAddressOf()
	);
	CheckResult(hr, "지오메트리 셰이더 생성 실패.");

	return m_geometryShaders[shaderName];
}

com_ptr<ID3D11PixelShader> ResourceManager::GetPixelShader(const string& shaderName)
{
	// 기존에 생성된 픽셀 셰이더가 있으면 재사용
	auto it = m_pixelShaders.find(shaderName);
	if (it != m_pixelShaders.end()) return it->second;

	HRESULT hr = S_OK;

	// 픽셀 셰이더 컴파일
	com_ptr<ID3DBlob> PSCode = CompileShader(shaderName, "ps_5_0");
	hr = m_device->CreatePixelShader
	(
		PSCode->GetBufferPointer(),
		PSCode->GetBufferSize(),
		nullptr,
		m_pixelShaders[shaderName].GetAddressOf()
	);
	CheckResult(hr, "픽셀 셰이더 생성 실패.");

	return m_pixelShaders[shaderName];
}

com_ptr<ID3D11ShaderResourceView> ResourceManager::GetTexture(const string& fileName, TextureType type)
{
	// 기존에 생성된 텍스처가 있으면 재사용
	auto it = m_textures.find(fileName);
	if (it != m_textures.end()) return it->second;

	HRESULT hr = S_OK;

	const auto cacheIt = m_textureCaches.find(fileName);
	if (cacheIt == m_textureCaches.end())
	{
		LOG_ERROR("텍스처 캐시에서 파일을 찾을 수 없습니다: " << fileName);
		switch (type)
		{
		case TextureType::BaseColor:
			return GetTexture("Fallback_BaseColor.png", TextureType::BaseColor);
		case TextureType::ORM:
			return GetTexture("Fallback_OcclusionRoughnessMetallic.png", TextureType::ORM);
		case TextureType::Normal:
			return GetTexture("Fallback_Normal.png", TextureType::Normal);
		case TextureType::Emissive:
			return GetTexture("Fallback_Emissive.png", TextureType::Emissive);
		case TextureType::LUT:
			return GetTexture("LUT\\0_IDENTITY.png", TextureType::LUT);
		default:
			return nullptr;
		}
	}

	bool isSRGB = (type == TextureType::BaseColor || type == TextureType::Emissive);

	// 파일 확장자 확인
	const string extension = fileName.substr(fileName.find_last_of('.') + 1);
	if (extension == "dds" || extension == "DDS")
	{
		// DDS 파일 (큐브맵 등)
		hr = CreateDDSTextureFromMemoryEx
		(
			m_device.Get(),
			m_deviceContext.Get(),
			cacheIt->second.data(),
			cacheIt->second.size(),
			0,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE,
			0,
			0, // dds 는 mipmap 자동 생성 못함
			isSRGB ? DDS_LOADER_FORCE_SRGB : DDS_LOADER_IGNORE_SRGB,
			nullptr,
			m_textures[fileName].GetAddressOf()
		);
		CheckResult(hr, "DDS 텍스처 생성 실패.");
	}
	else
	{
		// WIC 지원 이미지 (jpg, png 등)
		hr = CreateWICTextureFromMemoryEx
		(
			m_device.Get(),
			m_deviceContext.Get(),
			cacheIt->second.data(),
			cacheIt->second.size(),
			0,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE,
			0,
			D3D11_RESOURCE_MISC_GENERATE_MIPS, // mipmap 자동 생성
			isSRGB ? WIC_LOADER_FORCE_SRGB : WIC_LOADER_IGNORE_SRGB,
			nullptr,
			m_textures[fileName].GetAddressOf()
		);
		CheckResult(hr, "텍스처 생성 실패.");
	}

	return m_textures[fileName];
}

std::pair<com_ptr<ID3D11ShaderResourceView>, DirectX::XMFLOAT2> ResourceManager::GetTextureAndOffset(const std::string& fileName)
{
	com_ptr<ID3D11ShaderResourceView> textureSRV = GetTexture(fileName);
	XMFLOAT2 offset = {};

	// srv에서 크기 정보 얻기
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	textureSRV->GetDesc(&srvDesc);
	if (srvDesc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURE2D)
	{
		D3D11_TEX2D_SRV tex2DSRV = srvDesc.Texture2D;
		com_ptr<ID3D11Resource> resource = nullptr;
		textureSRV->GetResource(resource.GetAddressOf());
		com_ptr<ID3D11Texture2D> texture2D = nullptr;
		resource.As(&texture2D);
		D3D11_TEXTURE2D_DESC textureDesc = {};
		texture2D->GetDesc(&textureDesc);
		offset.x = static_cast<float>(textureDesc.Width) * 0.5f;
		offset.y = static_cast<float>(textureDesc.Height) * 0.5f;
	}

	return { textureSRV, offset };
}

void ResourceManager::CacheAllModel()
{
	const filesystem::path modelDir = "../Asset/Model/";
	if (!filesystem::exists(modelDir) || !filesystem::is_directory(modelDir))
	{
		LOG_ERROR("모델 디렉토리가 존재하지 않거나 디렉토리가 아닙니다: " << modelDir.string());
		return;
	}

	for (const auto& entry : filesystem::directory_iterator(modelDir))
	{
		if (entry.is_regular_file())
		{
			const string fileName = entry.path().filename().string();
			LoadModel(fileName);
		}
	}
}

const Model* ResourceManager::LoadModel(const string& fileName)
{
	auto it = m_models.find(fileName);
	if (it != m_models.end()) return &it->second;

	Assimp::Importer importer;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	const string fullPath = "../Asset/Model/" + fileName;

	const aiScene* scene = importer.ReadFile
	(
		fullPath,
		aiProcess_CalcTangentSpace | // 접선 공간 계산
		aiProcess_JoinIdenticalVertices | // 동일한 정점 결합 // 메모리 절약 // 좀 위험함
		aiProcess_Triangulate | // 삼각형화
		//aiProcess_GenSmoothNormals | // 부드러운 법선 생성 // 조금 느릴 수 있다고 하니까 유의
		aiProcess_SplitLargeMeshes | // 큰 메쉬 분할 // 드로우 콜 최대치를 넘는 메쉬 방지 // 이 옵션이 쓸일이 생기면 뭔가 크게 잘못된거임
		aiProcess_ValidateDataStructure | // 데이터 구조 검증 // 큰 문제가 아니여도 경고는 남김
		aiProcess_ImproveCacheLocality | // 정점 캐시 지역성 향상
		aiProcess_RemoveRedundantMaterials | // 사용되지 않는 재질 제거
		//aiProcess_FixInfacingNormals | // 뒤집힌 법선(내부를 향한 법선) 수정 // 만약 의도한 것이라면 이 옵션을 빼야함
		aiProcess_PopulateArmatureData | // 본 정보 채우기 // 애니메이션이 있는 모델에 필요 // 사실 뭐하는건지 잘 모르겠음
		aiProcess_SortByPType | // 프리미티브 타입별로 메쉬 정렬 // 삼각형, 선, 점 등으로 나눔 // 삼각형만 필요하면 나머지는 무시 가능
		aiProcess_FindDegenerates | // 엄청 작은(사실상 안보이는) 삼각형 제거
		aiProcess_FindInvalidData | // 잘못된 데이터(노말 값 = 0 같은거) 찾기 및 수정
		aiProcess_GenUVCoords | // 비UV 맵핑(구면, 원통 등)을 UV 좌표 채널로 변환
		aiProcess_TransformUVCoords | // UV 좌표 변환 적용 // 뭐하는건지 모르겠음
		aiProcess_FindInstances | // 중복 메쉬 찾기
		aiProcess_OptimizeMeshes | // 메쉬 최적화
		aiProcess_OptimizeGraph | // 씬 그래프 최적화 // 애니메이션이나 본이 없는 노드 병합 // 좀 위험할 수 있으니 유의
		aiProcess_SplitByBoneCount | // 본 개수로 메쉬 분할 // 한 메쉬에 본이 너무 많으면 여러 메쉬로 나눔 // 뭐하는건지 모르겠음
		aiProcess_Debone | // 사용하지 않는 더미 본 제거
		//aiProcess_DropNormals | // aiProcess_JoinIdenticalVertices 와 같이 사용 // 정점 노말 제거
		aiProcess_GenBoundingBoxes | // 바운딩 박스 생성
		aiProcess_LimitBoneWeights |
		aiProcess_GlobalScale |
		aiProcess_ConvertToLeftHanded // DirectX 좌표계(왼손 좌표계)로 변환
	);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG_ERROR("모델 " << fullPath << " 로드 실패 : " << importer.GetErrorString());
		exit(EXIT_FAILURE);
	}

	Model& model = m_models[fileName];

	//1. 모델 타입 결정 로직 변경
	bool hasBones = SceneHasBones(scene);
    bool hasAnims = scene->HasAnimations();

	if (hasBones) { model.type = ModelType::Skinned; }
	else if (hasAnims) {
		model.type = ModelType::Rigid;
		BuildRigidSkeleton(scene->mRootNode, model.skeleton);

		aiMatrix4x4 inverse_root_transform = scene->mRootNode->mTransformation;
		inverse_root_transform.Inverse();
		model.skeleton.globalInverseTransform = ToXMFLOAT4X4(inverse_root_transform);
	} 
	else { model.type = ModelType::Static; }



	// 2. 메쉬 및 노드 처리
	ProcessNode(scene->mRootNode, scene, model);

	// 3. 이름
	const string modelName = filesystem::path(fileName).stem().string();
	
	// 4. 본 정보가 있다면 스켈레톤 구축
	if (model.type == ModelType::Skinned){
		aiMatrix4x4 inverse_root_transform = scene->mRootNode->mTransformation;
		inverse_root_transform.Inverse();
		model.skeleton.globalInverseTransform = ToXMFLOAT4X4(inverse_root_transform);
		model.skeleton.root = BuildSkeletonNode(scene->mRootNode, model.skeleton);
	}
	else if (model.type == ModelType::Rigid){
		model.skeleton.root = BuildSkeletonNode(scene->mRootNode, model.skeleton);
	}

	// 5. 애니메이션 로드
	if (scene->HasAnimations()) LoadAnimations(scene, model);

	return &m_models[fileName];
}

Material ResourceManager::LoadMaterial(const string& materialName)
{
	Material material = {};

	material.baseColorTextureSRV = GetTexture(materialName + "_BaseColor.png", TextureType::BaseColor);
	material.ORMTextureSRV = GetTexture(materialName + "_OcclusionRoughnessMetallic.png", TextureType::ORM);
	material.normalTextureSRV = GetTexture(materialName + "_Normal.png", TextureType::Normal);
	material.emissionTextureSRV = GetTexture(materialName + "_Emissive.png", TextureType::Emissive);

	return material;
}

SpriteFont* ResourceManager::GetSpriteFont(const wstring& fontName)
{
	// 기존에 생성된 스프라이트 폰트가 있으면 재사용
	auto it = m_spriteFonts.find(fontName);
	if (it != m_spriteFonts.end()) return it->second.get();

	// 새로 생성
	m_spriteFonts[fontName] = make_unique<SpriteFont>(m_device.Get(), (L"../Asset/Font/" + fontName + L".spritefont").c_str());

	return m_spriteFonts[fontName].get();
}

void ResourceManager::CreateDepthStencilStates()
{
	HRESULT hr = S_OK;
	for (size_t i = 0; i < static_cast<size_t>(DepthStencilState::Count); ++i)
	{
		hr = m_device->CreateDepthStencilState(&DEPTH_STENCIL_DESC_TEMPLATES[i], m_depthStencilStates[i].GetAddressOf());
		CheckResult(hr, "깊이버퍼 상태 생성 실패.");
	}
}

void ResourceManager::CreateBlendStates()
{
	HRESULT hr = S_OK;
	for (size_t i = 0; i < static_cast<size_t>(BlendState::Count); ++i)
	{
		hr = m_device->CreateBlendState(&BLEND_DESC_TEMPLATES[i], m_blendStates[i].GetAddressOf());
		CheckResult(hr, "블렌드 상태 생성 실패.");
	}
}

void ResourceManager::CreateRasterStates()
{
	HRESULT hr = S_OK;

	for (size_t i = 0; i < static_cast<size_t>(RasterState::Count); ++i)
	{
		hr = m_device->CreateRasterizerState(&RASTERIZER_DESC_TEMPLATES[i], m_rasterStates[i].GetAddressOf());
		CheckResult(hr, "래스터 상태 생성 실패.");
	}
}

void ResourceManager::CreateConstantBuffers()
{
	HRESULT hr = S_OK;

	// 정점 셰이더용 상수 버퍼 생성
	for (size_t i = 0; i < static_cast<size_t>(VSConstBuffers::Count); ++i)
	{
		hr = m_device->CreateBuffer(&VS_CONST_BUFFER_DESCS[i], nullptr, m_vsConstantBuffers[i].GetAddressOf());
		CheckResult(hr, "정점 셰이더용 상수 버퍼 생성 실패.");
	}

	// 지오메트리 셰이더용 상수 버퍼 생성
	for (size_t i = 0; i < static_cast<size_t>(GSConstBuffers::Count); ++i)
	{
		hr = m_device->CreateBuffer(&GS_CONST_BUFFER_DESCS[i], nullptr, m_gsConstantBuffers[i].GetAddressOf());
		CheckResult(hr, "지오메트리 셰이더용 상수 버퍼 생성 실패.");
	}

	// 픽셀 셰이더용 상수 버퍼 생성
	for (size_t i = 0; i < static_cast<size_t>(PSConstBuffers::Count); ++i)
	{
		hr = m_device->CreateBuffer(&PS_CONST_BUFFER_DESCS[i], nullptr, m_psConstantBuffers[i].GetAddressOf());
		CheckResult(hr, "픽셀 셰이더용 상수 버퍼 생성 실패.");
	}
}

void ResourceManager::CreateSamplerStates()
{
	HRESULT hr = S_OK;

	for (size_t i = 0; i < static_cast<size_t>(SamplerState::Count); ++i)
	{
		hr = m_device->CreateSamplerState(&SAMPLER_DESC_TEMPLATES[i], m_samplerStates[i].GetAddressOf());
		CheckResult(hr, "샘플러 상태 생성 실패.");
	}
}

void ResourceManager::CacheAllTexture()
{
	const filesystem::path textureDirectory = "../Asset/Texture/";

	for (const auto& dirEntry : filesystem::recursive_directory_iterator(textureDirectory))
	{
		if (dirEntry.is_regular_file())
		{
			const string fileName = filesystem::relative(dirEntry.path(), textureDirectory).string();

			// 텍스처 파일 읽기
			ifstream fileStream(dirEntry.path(), ios::binary | ios::ate);

			if (fileStream)
			{
				const streamsize fileSize = fileStream.tellg();
				fileStream.seekg(0, ios::beg);
				vector<char> fileData(static_cast<size_t>(fileSize));

				if (fileStream.read(fileData.data(), fileSize)) m_textureCaches[fileName] = vector<uint8_t>(fileData.begin(), fileData.end());
				else LOG_ERROR("텍스처 파일 읽기 실패: " << fileName);
			}
			else LOG_ERROR("텍스처 파일 열기 실패: " << fileName);
		}
	}
}

void ResourceManager::ProcessNode(const aiNode* node, const aiScene* scene, Model& model)
{
	// 노드의 메쉬 처리
	for (UINT i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		model.meshes.push_back(ProcessMesh(mesh, scene, model, node));
	}

	// 자식 노드 재귀 처리
	for (UINT i = 0; i < node->mNumChildren; ++i) ProcessNode(node->mChildren[i], scene, model);
}

Mesh ResourceManager::ProcessMesh(const aiMesh* mesh, const aiScene* scene, Model& model, const aiNode* node)
{
	Mesh resultMesh;

	switch (mesh->mPrimitiveTypes) {
	case aiPrimitiveType_POINT:		resultMesh.topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;		break;
	case aiPrimitiveType_LINE:		resultMesh.topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;		break;
	case aiPrimitiveType_TRIANGLE:	resultMesh.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;	break;
	default:						resultMesh.topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;		break;
	}

	uint32_t ownerNodeIndex = 0;
	const bool isRigid = (model.type == ModelType::Rigid);
	const bool isSkinned = (model.type == ModelType::Skinned);
	const bool hasBones = mesh->HasBones();

	if (isRigid) {
		string nodeName = node->mName.C_Str();
		if(model.skeleton.boneMapping.find(nodeName) != model.skeleton.boneMapping.end()){
			ownerNodeIndex = model.skeleton.boneMapping[nodeName];
		}
	}

	// 정점 처리
	resultMesh.vertices.reserve(mesh->mNumVertices);
	for (UINT i = 0; i < mesh->mNumVertices; ++i)
	{
		Vertex vertex = {};
		vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f };

		if (mesh->mTextureCoords[0]) vertex.UV = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		if (mesh->HasNormals()) vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
		if (mesh->HasTangentsAndBitangents()){
			vertex.bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
			vertex.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
		}

		// 본 데이터 초기화
		vertex.boneIndex = { 0, 0, 0, 0 };
		vertex.boneWeight = { 0.f, 0.f, 0.f, 0.f };

		if (isRigid) {
			vertex.boneIndex[0] = ownerNodeIndex;
			vertex.boneWeight.x = 1.f;
		} else if (isSkinned && !hasBones) {
			vertex.boneIndex[0] = 0;
			vertex.boneWeight.x = 1.f;
		}

		resultMesh.vertices.push_back(vertex);
	}

	if (hasBones && isSkinned){
		auto addBoneData = [](Vertex& vertex, uint32_t boneIndex, float weight){
				float* weights = &vertex.boneWeight.x;
				uint32_t* indices = vertex.boneIndex.data();

				for (int i = 0; i < 4; ++i) {
					if (weights[i] == 0.0f) {
						indices[i] = boneIndex;
						weights[i] = weight;
						return;
					}
				}
			};

		for (UINT i = 0; i < mesh->mNumBones; ++i)
		{
			const aiBone* bone = mesh->mBones[i];
			const string boneName = bone->mName.C_Str();
			uint32_t boneIndex = 0;

			auto mappingIt = model.skeleton.boneMapping.find(boneName);
			if (mappingIt == model.skeleton.boneMapping.end()) {
				boneIndex = static_cast<uint32_t>(model.skeleton.bones.size());
				model.skeleton.boneMapping[boneName] = boneIndex;
				BoneInfo info = {};
				info.id = boneIndex;
				info.offset_matrix = ToXMFLOAT4X4(bone->mOffsetMatrix);
				model.skeleton.bones.push_back(info);
			} else {
				boneIndex = mappingIt->second;
			}

			for (UINT weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
				const aiVertexWeight& weight = bone->mWeights[weightIndex];
				if (weight.mVertexId < resultMesh.vertices.size()) {
					addBoneData(resultMesh.vertices[weight.mVertexId], boneIndex, weight.mWeight);
				}
			}
		}

		for (auto& vertex : resultMesh.vertices) {
			float* weights = &vertex.boneWeight.x;
			float sum = weights[0] + weights[1] + weights[2] + weights[3];
			if (sum > 0.0f) { for (int i = 0; i < 4; ++i) weights[i] /= sum;}
		}
	}

	for (UINT i = 0; i < mesh->mNumFaces; ++i)
	{
		const aiFace& face = mesh->mFaces[i];
		for (UINT j = 0; j < face.mNumIndices; ++j) resultMesh.indices.push_back(face.mIndices[j]);
	}
	resultMesh.indexCount = static_cast<UINT>(resultMesh.indices.size());

	// 바운딩 박스 처리
	resultMesh.boundingBox =
	{
		// 중심
		{
			(mesh->mAABB.mMin.x + mesh->mAABB.mMax.x) * 0.5f,
			(mesh->mAABB.mMin.y + mesh->mAABB.mMax.y) * 0.5f,
			(mesh->mAABB.mMin.z + mesh->mAABB.mMax.z) * 0.5f
		},
		// 꼭짓점까지의 거리
		{
			(mesh->mAABB.mMax.x - mesh->mAABB.mMin.x) * 0.5f,
			(mesh->mAABB.mMax.y - mesh->mAABB.mMin.y) * 0.5f,
			(mesh->mAABB.mMax.z - mesh->mAABB.mMin.z) * 0.5f
		}
	};
	// 모델 전체 바운딩 박스 갱신
	BoundingBox::CreateMerged(model.boundingBox, model.boundingBox, resultMesh.boundingBox);

	CreateMeshBuffers(resultMesh);

	return resultMesh;
}

void ResourceManager::BuildRigidSkeleton(const aiNode* node, Skeleton& skeleton)
{
	std::string nodeName = node->mName.C_Str();

	// 이미 등록된 본인지 확인 (중복 방지)
	if (skeleton.boneMapping.find(nodeName) == skeleton.boneMapping.end())
	{
		uint32_t newIndex = static_cast<uint32_t>(skeleton.bones.size());
		skeleton.boneMapping[nodeName] = newIndex;

		BoneInfo info = {};
		info.id = newIndex;

		// Rigid 애니메이션: 정점이 이미 노드 로컬 좌표계에 있으므로
		// Bind Pose 변환(Offset Matrix)은 단위 행렬(Identity)입니다.
		XMStoreFloat4x4(&info.offset_matrix, XMMatrixIdentity());

		skeleton.bones.push_back(info);
	}

	// 자식 노드 순회
	for (UINT i = 0; i < node->mNumChildren; ++i)
	{
		BuildRigidSkeleton(node->mChildren[i], skeleton);
	}
}

void ResourceManager::CreateMeshBuffers(Mesh& mesh)
{
	HRESULT hr = S_OK;

	// 정점 버퍼 생성
	if (mesh.vertices.empty()) return;
	const D3D11_BUFFER_DESC vertexBufferDesc =
	{
		.ByteWidth = static_cast<UINT>(sizeof(Vertex) * mesh.vertices.size()),
		.Usage = D3D11_USAGE_DEFAULT, // 이거 D3D11_USAGE_IMMUTABLE로 바꿀 수 있나?
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};
	const D3D11_SUBRESOURCE_DATA vertexInitialData =
	{
		.pSysMem = mesh.vertices.data(),
		.SysMemPitch = 0,
		.SysMemSlicePitch = 0
	};
	hr = m_device->CreateBuffer(&vertexBufferDesc, &vertexInitialData, mesh.vertexBuffer.GetAddressOf());
	CheckResult(hr, "메쉬 정점 버퍼 생성 실패.");

	// 인덱스 버퍼 생성
	if (mesh.indices.empty()) return;
	const D3D11_BUFFER_DESC indexBufferDesc =
	{
		.ByteWidth = static_cast<UINT>(sizeof(UINT) * mesh.indices.size()),
		.Usage = D3D11_USAGE_DEFAULT, // 이것도
		.BindFlags = D3D11_BIND_INDEX_BUFFER,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};
	const D3D11_SUBRESOURCE_DATA indexInitialData =
	{
		.pSysMem = mesh.indices.data(),
		.SysMemPitch = 0,
		.SysMemSlicePitch = 0
	};
	hr = m_device->CreateBuffer(&indexBufferDesc, &indexInitialData, mesh.indexBuffer.GetAddressOf());
	CheckResult(hr, "메쉬 인덱스 버퍼 생성 실패.");
}

unique_ptr<SkeletonNode> ResourceManager::BuildSkeletonNode(const aiNode* node, Skeleton& skeleton)
{
	auto skeletonNode = make_unique<SkeletonNode>();
	skeletonNode->name = node->mName.C_Str();
	skeletonNode->localTransform = ToXMFLOAT4X4(node->mTransformation);

	auto mappingIt = skeleton.boneMapping.find(skeletonNode->name);
	if (mappingIt != skeleton.boneMapping.end())
	{
		skeletonNode->boneIndex = static_cast<int>(mappingIt->second);
	}

	for (UINT i = 0; i < node->mNumChildren; ++i)
	{
		skeletonNode->children.push_back(BuildSkeletonNode(node->mChildren[i], skeleton));
	}

	return skeletonNode;
}

/// <summary>
/// Assimp 라이브러리로 읽어들인 원본 데이터(aiScene)를 우리 엔진 전용 포맷(Model)으로 변환(Parsing)하는 함수
/// </summary>
/// <param name="scene">[in] 원본 데이터(aiScene)</param>
/// <param name="model">[out] 우리 엔진 전용 포맷(Model)</param>
void ResourceManager::LoadAnimations(const aiScene* scene, Model& model)
{
	model.animations.clear();
	if (!scene || scene->mNumAnimations == 0) return;
	const string rootNodeName = scene->mRootNode ? scene->mRootNode->mName.C_Str() : "";

	model.animations.reserve(scene->mNumAnimations);

	// 1. 애니메이션 순회
	for (UINT anim_index = 0; anim_index < scene->mNumAnimations; ++anim_index) {
		const aiAnimation* animation = scene->mAnimations[anim_index];
		if (!animation) continue;

		AnimationClip clip = {};
		clip.name = animation->mName.C_Str();
		if (clip.name.empty()) clip.name = "Animation_" + to_string(anim_index);

		clip.duration = static_cast<float>(animation->mDuration);
		clip.ticks_per_second = static_cast<float>(animation->mTicksPerSecond);
		if (clip.ticks_per_second <= 0.0f) clip.ticks_per_second = AnimationClip::DEFAULT_FPS;

		// [Duration 보정용 변수]
		float last_keyframe_time = 0.0f;

		// 2. 채널 순회
		for (UINT channel_index = 0; channel_index < animation->mNumChannels; ++channel_index) {
			const aiNodeAnim* node_anim = animation->mChannels[channel_index];
			if (!node_anim) continue;

			BoneAnimationChannel channel = {};
			channel.boneName = node_anim->mNodeName.C_Str();

			// 3. 본 매핑
			auto mapping_iter = model.skeleton.boneMapping.find(channel.boneName);
			if (mapping_iter != model.skeleton.boneMapping.end()) {
				channel.boneIndex = static_cast<int>(mapping_iter->second);
			}

			// 4. 키프레임 복사 및 마지막 시간 추적
			// (1) Position
			channel.position_keys.reserve(node_anim->mNumPositionKeys);
			for (UINT i = 0; i < node_anim->mNumPositionKeys; ++i) {
				VectorKeyframe key = {};
				key.time_position = static_cast<float>(node_anim->mPositionKeys[i].mTime);
				key.value = ToXMFLOAT3(node_anim->mPositionKeys[i].mValue);
				channel.position_keys.push_back(key);
			}
			if (IsRootMotionChannel(channel.boneName, rootNodeName))
			{
				RemoveHorizontalRootMotion(channel);
			}
			if (!channel.position_keys.empty()) {
				last_keyframe_time = max(last_keyframe_time, channel.position_keys.back().time_position);
			}

			// (2) Rotation
			channel.rotation_keys.reserve(node_anim->mNumRotationKeys);
			for (UINT i = 0; i < node_anim->mNumRotationKeys; ++i) {
				QuaternionKeyframe key = {};
				key.time_position = static_cast<float>(node_anim->mRotationKeys[i].mTime);
				key.value = ToXMFLOAT4(node_anim->mRotationKeys[i].mValue);
				channel.rotation_keys.push_back(key);
			}
			if (!channel.rotation_keys.empty()) {
				last_keyframe_time = max(last_keyframe_time, channel.rotation_keys.back().time_position);
			}

			// (3) Scale
			channel.scale_keys.reserve(node_anim->mNumScalingKeys);
			for (UINT i = 0; i < node_anim->mNumScalingKeys; ++i) {
				VectorKeyframe key = {};
				key.time_position = static_cast<float>(node_anim->mScalingKeys[i].mTime);
				key.value = ToXMFLOAT3(node_anim->mScalingKeys[i].mValue);
				channel.scale_keys.push_back(key);
			}
			if (!channel.scale_keys.empty()) {
				last_keyframe_time = max(last_keyframe_time, channel.scale_keys.back().time_position);
			}

			string keyName = channel.boneName;
			clip.channels[keyName] = move(channel);
		}

		// [Duration 최종 보정] // Assimp가 제공한 Duration이 실제 키프레임보다 불필요하게 길거나(블렌더 타임라인 전체 등), 혹은 0으로 잘못 들어온 경우, 실제 키프레임의 끝 시간으로 덮어씁니다.
		if (clip.duration > last_keyframe_time || clip.duration <= 0.0f) {
			if (last_keyframe_time > 0.0f) {
				clip.duration = last_keyframe_time;
			}
		}

		#ifdef _DEBUG
		LOG("[LoadAnim] Name: " << clip.name << " | Original Duration: " << animation->mDuration<< " -> Fixed: " << clip.duration << " (FPS: " << clip.ticks_per_second << ")");
		#endif

		// 5. 저장
		model.animations.push_back(move(clip));
	}
}


com_ptr<ID3DBlob> ResourceManager::CompileShader(const string& shaderName, const char* shaderModel)
{
	HRESULT hr = S_OK;

	const filesystem::path shaderPath = "../Asset/Shader/" + shaderName;
	com_ptr<ID3DBlob> shaderCode = nullptr;
	com_ptr<ID3DBlob> errorBlob = nullptr;

	hr = D3DCompileFromFile
	(
		shaderPath.wstring().c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		shaderModel,
		#ifdef _DEBUG
		D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		#else
		D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL3,
		#endif
		0,
		shaderCode.GetAddressOf(),
		errorBlob.GetAddressOf()
	);
	if (errorBlob) LOG_ERROR(shaderName << " 셰이더 컴파일 오류: " << static_cast<const char*>(errorBlob->GetBufferPointer()));
	return shaderCode;
}


void ResourceManager::LoadLUTTexture()
{
	m_luts[0].srv =  GetTexture("LUT\\0_IDENTITY.png", TextureType::LUT);
	m_luts[1].srv =  GetTexture("LUT\\1_SEPIA.png", TextureType::LUT);
	m_luts[2].srv =  GetTexture("LUT\\2_GREENISH.png", TextureType::LUT);
	m_luts[3].srv =  GetTexture("LUT\\3_REDDISH.png", TextureType::LUT);
	m_luts[4].srv =  GetTexture("LUT\\4_ORANGE.png", TextureType::LUT);
}

void ResourceManager::LoadNoiseTexture()
{
	m_noises[0].srv = GetTexture("noise\\00_CELL.png", TextureType::Normal);
	m_noises[1].srv = GetTexture("noise\\01_JJEOJEOJEOK.png", TextureType::Normal);
	m_noises[2].srv = GetTexture("noise\\02_distortion.dds", TextureType::Normal);
}

