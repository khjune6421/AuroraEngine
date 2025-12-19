#include "stdafx.h"
#include "ResourceManager.h"

using namespace std;
using namespace DirectX;

void ResourceManager::Initialize(com_ptr<ID3D11Device> device)
{
	m_device = device;

	CreateRasterStates();
	CreateSamplerStates();
}

com_ptr<ID3D11Buffer> ResourceManager::GetConstantBuffer(UINT bufferSize)
{
	// 기존에 생성된 상수 버퍼가 있으면 재사용
	auto it = m_constantBuffers.find(bufferSize);
	if (it != m_constantBuffers.end()) return it->second;

	HRESULT hr = S_OK;

	com_ptr<ID3D11Buffer> constantBuffer = nullptr;
	const D3D11_BUFFER_DESC bufferDesc =
	{
		.ByteWidth = bufferSize,
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, constantBuffer.GetAddressOf());
	CheckResult(hr, "상수 버퍼 생성 실패.");

	m_constantBuffers[bufferSize] = constantBuffer;

	return constantBuffer;
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
		cerr << "모델 로드 실패: " << importer.GetErrorString() << endl;
		return nullptr;
	}

	ProcessNode(scene->mRootNode, scene, m_models[fileName]);

	return &m_models[fileName];
}

com_ptr<ID3D11ShaderResourceView> ResourceManager::LoadTexture(const std::string& fileName)
{
	// 기존에 생성된 텍스처가 있으면 재사용
	auto it = m_textures.find(fileName);
	if (it != m_textures.end()) return it->second;

	HRESULT hr = S_OK;

	const string fullPath = "../Asset/Texture/" + fileName;

	com_ptr<ID3D11ShaderResourceView> textureSRV = nullptr;
	hr = CreateWICTextureFromFile
	(
		m_device.Get(),
		wstring(fullPath.begin(), fullPath.end()).c_str(),
		nullptr,
		textureSRV.GetAddressOf()
	);
	CheckResult(hr, "텍스처 로드 실패.");

	m_textures[fileName] = textureSRV;

	return textureSRV;
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

void ResourceManager::CreateSamplerStates()
{
	HRESULT hr = S_OK;

	for (size_t i = 0; i < static_cast<size_t>(SamplerState::Count); ++i)
	{
		hr = m_device->CreateSamplerState(&SAMPLER_DESC_TEMPLATES[i], m_samplerStates[i].GetAddressOf());
		CheckResult(hr, "샘플러 상태 생성 실패.");
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

		// 쓰면 터짐 // 일단은 텍스처는 따로 불러오게
		//resultMesh.materialTexture.albedoTextureSRV = LoadMaterialTexture(material, aiTextureType_DIFFUSE);
		//resultMesh.materialTexture.normalTextureSRV = LoadMaterialTexture(material, aiTextureType_NORMALS);
		//resultMesh.materialTexture.metallicTextureSRV = LoadMaterialTexture(material, aiTextureType_METALNESS);
		//resultMesh.materialTexture.roughnessTextureSRV = LoadMaterialTexture(material, aiTextureType_DIFFUSE_ROUGHNESS);

		resultMesh.materialTexture.albedoTextureSRV = LoadTexture("SampleAlbedo.jpg");
		resultMesh.materialTexture.normalTextureSRV = LoadTexture("SampleNormal.jpg");
		resultMesh.materialTexture.metallicTextureSRV = LoadTexture("SampleMetallic.jpg");
		resultMesh.materialTexture.roughnessTextureSRV = LoadTexture("SampleRoughness.jpg");
	}

	CreateMeshBuffers(resultMesh);

	return resultMesh;
}

MaterialFactor ResourceManager::ProcessMaterialFactor(aiMaterial* material)
{
	MaterialFactor resultMaterialFactor;

	// Albedo/Diffuse 색상 팩터
	aiColor4D albedoColor;
	if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, albedoColor)) resultMaterialFactor.albedoFactor = { albedoColor.r, albedoColor.g, albedoColor.b, albedoColor.a };

	// PBR 메탈릭 팩터
	float metallicFactor = 1.0f;
	if (AI_SUCCESS == material->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor)) resultMaterialFactor.metallicFactor = metallicFactor;

	// PBR 러프니스 팩터
	float roughnessFactor = 1.0f;
	if (AI_SUCCESS == material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor)) resultMaterialFactor.roughnessFactor = roughnessFactor;

	return resultMaterialFactor;
}

com_ptr<ID3D11ShaderResourceView> ResourceManager::LoadMaterialTexture(aiMaterial* material, aiTextureType type)
{
	if (material->GetTextureCount(type) > 0)
	{
		aiString texturePath;
		material->GetTexture(type, 0, &texturePath);

		// 외부 파일 텍스처 처리
		string fileName = texturePath.C_Str();

		// 파일 경로에서 파일 이름만 추출 (경로 구분자 처리)
		size_t lastSlash = fileName.find_last_of("/\\");
		if (lastSlash != string::npos) fileName = fileName.substr(lastSlash + 1);

		return LoadTexture(fileName);
	}
	return nullptr;
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