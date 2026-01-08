///ResourceManager.cpp의 시작
#include "stdafx.h"
#include "ResourceManager.h"

using namespace std;
using namespace DirectX;

void ResourceManager::Initialize(com_ptr<ID3D11Device> device, com_ptr<ID3D11DeviceContext> deviceContext)
{
	m_device = device;
	m_deviceContext = deviceContext;

	CreateDepthStencilStates();
	CreateBlendStates();
	CreateRasterStates();

	CreateAndSetConstantBuffers();
	CreateAndSetSamplerStates();

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

com_ptr<ID3D11ShaderResourceView> ResourceManager::GetTexture(const string& fileName)
{
	// 기존에 생성된 텍스처가 있으면 재사용
	auto it = m_textures.find(fileName);
	if (it != m_textures.end()) return it->second;

	HRESULT hr = S_OK;

	// 캐시된 텍스처 데이터 사용
	const auto cacheIt = m_textureCaches.find(fileName);
	if (cacheIt == m_textureCaches.end())
	{
		#ifdef _DEBUG
		cerr << "텍스처 데이터 캐시 없음: " << fileName << endl;
		#else
		MessageBoxA(nullptr, ("텍스처 데이터 캐시 없음: " + fileName).c_str(), "오류", MB_OK | MB_ICONERROR);
		#endif
		exit(EXIT_FAILURE);
	}

	// 파일 확장자 확인
	const string extension = fileName.substr(fileName.find_last_of('.') + 1);
	const bool isDDS = (extension == "dds" || extension == "DDS");

	if (isDDS)
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
			0, // dds 는 mipmap 자동 생성 안함
			DDS_LOADER_DEFAULT,
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
			WIC_LOADER_DEFAULT, // 나중에 감마 보정 옵션도 넣기
			nullptr,
			m_textures[fileName].GetAddressOf()
		);
		CheckResult(hr, "텍스처 생성 실패.");
	}

	return m_textures[fileName];
}

const Model* ResourceManager::LoadModel(const string& fileName)
{
	auto it = m_models.find(fileName);
	if (it != m_models.end()) return &it->second;

	Assimp::Importer importer;

	const string fullPath = "../Asset/Model/" + fileName;

	const aiScene* scene = importer.ReadFile
	(
		fullPath,
		aiProcess_CalcTangentSpace | // 접선 공간 계산
		aiProcess_JoinIdenticalVertices | // 동일한 정점 결합 // 메모리 절약 // 좀 위험함
		aiProcess_Triangulate | // 삼각형화
		aiProcess_GenSmoothNormals | // 부드러운 법선 생성 // 조금 느릴 수 있다고 하니까 유의
		aiProcess_SplitLargeMeshes | // 큰 메쉬 분할 // 드로우 콜 최대치를 넘는 메쉬 방지 // 이 옵션이 쓸일이 생기면 뭔가 크게 잘못된거임
		aiProcess_ValidateDataStructure | // 데이터 구조 검증 // 큰 문제가 아니여도 경고는 남김
		aiProcess_ImproveCacheLocality | // 정점 캐시 지역성 향상
		aiProcess_RemoveRedundantMaterials | // 사용되지 않는 재질 제거
		aiProcess_FixInfacingNormals | // 뒤집힌 법선(내부를 향한 법선) 수정 // 만약 의도한 것이라면 이 옵션을 빼야함
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
		aiProcess_DropNormals | // aiProcess_JoinIdenticalVertices 와 같이 사용 // 정점 노말 제거
		aiProcess_GenBoundingBoxes | // 바운딩 박스 생성
		aiProcess_ConvertToLeftHanded // DirectX 좌표계(왼손 좌표계)로 변환
	);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		#ifdef _DEBUG
		cerr << "모델 로드 실패: " << importer.GetErrorString() << endl;
		#else
		MessageBoxA(nullptr, ("모델 로드 실패: " + string(importer.GetErrorString())).c_str(), "오류", MB_OK | MB_ICONERROR);
		#endif
		exit(EXIT_FAILURE);
	}

	ProcessNode(scene->mRootNode, scene, m_models[fileName]);

	return &m_models[fileName];
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

void ResourceManager::CreateAndSetConstantBuffers()
{
	HRESULT hr = S_OK;

	// 정점 셰이더용 상수 버퍼 생성 및 설정
	// 뷰-투영 상수 버퍼
	hr = m_device->CreateBuffer(&VS_CONST_BUFFER_DESCS[static_cast<size_t>(VSConstBuffers::ViewProjection)], nullptr, m_vsConstantBuffers[static_cast<size_t>(VSConstBuffers::ViewProjection)].GetAddressOf());
	CheckResult(hr, "ViewProjection 상수 버퍼 생성 실패.");
	m_deviceContext->VSSetConstantBuffers(static_cast<UINT>(VSConstBuffers::ViewProjection), 1, m_vsConstantBuffers[static_cast<size_t>(VSConstBuffers::ViewProjection)].GetAddressOf());
	// 스카이박스 뷰-투영 역행렬 상수 버퍼
	hr = m_device->CreateBuffer(&VS_CONST_BUFFER_DESCS[static_cast<size_t>(VSConstBuffers::SkyboxViewProjection)], nullptr, m_vsConstantBuffers[static_cast<size_t>(VSConstBuffers::SkyboxViewProjection)].GetAddressOf());
	CheckResult(hr, "Object 상수 버퍼 생성 실패.");
	m_deviceContext->VSSetConstantBuffers(static_cast<UINT>(VSConstBuffers::SkyboxViewProjection), 1, m_vsConstantBuffers[static_cast<size_t>(VSConstBuffers::SkyboxViewProjection)].GetAddressOf());
	// 객체 월드, 스케일 역행렬 적용한 월드 상수 버퍼
	hr = m_device->CreateBuffer(&VS_CONST_BUFFER_DESCS[static_cast<size_t>(VSConstBuffers::WorldNormal)], nullptr, m_vsConstantBuffers[static_cast<size_t>(VSConstBuffers::WorldNormal)].GetAddressOf());
	CheckResult(hr, "WorldNormal 상수 버퍼 생성 실패.");
	m_deviceContext->VSSetConstantBuffers(static_cast<UINT>(VSConstBuffers::WorldNormal), 1, m_vsConstantBuffers[static_cast<size_t>(VSConstBuffers::WorldNormal)].GetAddressOf());

	// 픽셀 셰이더용 상수 버퍼 생성 및 설정
	// 카메라 위치 상수 버퍼
	hr = m_device->CreateBuffer(&PS_CONST_BUFFER_DESCS[static_cast<size_t>(PSConstBuffers::CameraPosition)], nullptr, m_psConstantBuffers[static_cast<size_t>(PSConstBuffers::CameraPosition)].GetAddressOf());
	CheckResult(hr, "CameraPosition 상수 버퍼 생성 실패.");
	m_deviceContext->PSSetConstantBuffers(static_cast<UINT>(PSConstBuffers::CameraPosition), 1, m_psConstantBuffers[static_cast<size_t>(PSConstBuffers::CameraPosition)].GetAddressOf());
	// 방향광 상수 버퍼
	hr = m_device->CreateBuffer(&PS_CONST_BUFFER_DESCS[static_cast<size_t>(PSConstBuffers::GlobalLight)], nullptr, m_psConstantBuffers[static_cast<size_t>(PSConstBuffers::GlobalLight)].GetAddressOf());
	CheckResult(hr, "DirectionalLight 상수 버퍼 생성 실패.");
	m_deviceContext->PSSetConstantBuffers(static_cast<UINT>(PSConstBuffers::GlobalLight), 1, m_psConstantBuffers[static_cast<size_t>(PSConstBuffers::GlobalLight)].GetAddressOf());
	// 재질 팩터 상수 버퍼
	hr = m_device->CreateBuffer(&PS_CONST_BUFFER_DESCS[static_cast<size_t>(PSConstBuffers::MaterialFactor)], nullptr, m_psConstantBuffers[static_cast<size_t>(PSConstBuffers::MaterialFactor)].GetAddressOf());
	CheckResult(hr, "MaterialFactor 상수 버퍼 생성 실패.");
	m_deviceContext->PSSetConstantBuffers(static_cast<UINT>(PSConstBuffers::MaterialFactor), 1, m_psConstantBuffers[static_cast<size_t>(PSConstBuffers::MaterialFactor)].GetAddressOf());
}

void ResourceManager::CreateAndSetSamplerStates()
{
	HRESULT hr = S_OK;

	for (size_t i = 0; i < static_cast<size_t>(SamplerState::Count); ++i)
	{
		hr = m_device->CreateSamplerState(&SAMPLER_DESC_TEMPLATES[i], m_samplerStates[i].GetAddressOf());
		CheckResult(hr, "샘플러 상태 생성 실패.");

		m_deviceContext->PSSetSamplers(static_cast<UINT>(i), 1, m_samplerStates[i].GetAddressOf());
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
				else cerr << "텍스처 파일 읽기 실패: " << fileName << endl;
			}
			else cerr << "텍스처 파일 열기 실패: " << fileName << endl;
		}
	}
}

void ResourceManager::ProcessNode(const aiNode* node, const aiScene* scene, Model& model)
{
	// 노드의 메쉬 처리
	for (UINT i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		model.meshes.push_back(ProcessMesh(mesh, scene));
	}

	// 자식 노드 재귀 처리
	for (UINT i = 0; i < node->mNumChildren; ++i) ProcessNode(node->mChildren[i], scene, model);
}

Mesh ResourceManager::ProcessMesh(const aiMesh* mesh, const aiScene* scene)
{
	Mesh resultMesh;

	// 정점 처리
	for (UINT i = 0; i < mesh->mNumVertices; ++i)
	{
		Vertex vertex;
		// 위치
		vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f };

		// UV // 첫 번째 UV 채널만 사용
		if (mesh->mTextureCoords[0]) vertex.UV = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

		// 법선
		if (mesh->HasNormals()) vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

		// 접선
		if (mesh->HasTangentsAndBitangents()) vertex.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };

		resultMesh.vertices.push_back(vertex);
	}

	// 인덱스 처리
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

	// 재질 처리
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		resultMesh.materialFactor = ProcessMaterialFactor(material);

		resultMesh.materialTexture.albedoTextureSRV = GetTexture("SampleAlbedo.dds");
		resultMesh.materialTexture.ORMTextureSRV = GetTexture("SampleORM.dds");
		resultMesh.materialTexture.normalTextureSRV = GetTexture("SampleNormal.dds");
	}

	CreateMeshBuffers(resultMesh);

	return resultMesh;
}

MaterialFactorBuffer ResourceManager::ProcessMaterialFactor(aiMaterial* material)
{
	MaterialFactorBuffer resultMaterialFactor;

	// Albedo/Diffuse 색상 팩터
	aiColor4D albedoColor;
	if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, albedoColor)) resultMaterialFactor.albedoFactor = { albedoColor.r, albedoColor.g, albedoColor.b, albedoColor.a };

	// PBR 메탈릭 팩터
	float metallicFactor = 1.0f;
	if (AI_SUCCESS == material->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor)) resultMaterialFactor.metallicFactor = metallicFactor;

	// PBR 러프니스 팩터
	float roughnessFactor = 1.0f;
	if (AI_SUCCESS == material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor)) resultMaterialFactor.roughnessFactor = roughnessFactor;

	// 굴절률 팩터
	float iorFactor = 1.5f;
	if (AI_SUCCESS == material->Get(AI_MATKEY_REFRACTI, iorFactor)) resultMaterialFactor.iorFactor = iorFactor;

	return resultMaterialFactor;
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
	if (errorBlob) cerr << shaderName << " 셰이더 컴파일 오류: " << static_cast<const char*>(errorBlob->GetBufferPointer()) << endl;

	return shaderCode;
}
///ResourceManager.cpp의 끝