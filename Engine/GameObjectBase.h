#pragma once
#include "ComponentBase.h"

class Renderer;

class GameObjectBase // TODO: 부모-자식 관계 구현
{
	UINT m_id = 0; // 디버깅용 고유 ID

	std::unordered_map<std::type_index, std::unique_ptr<class ComponentBase>> m_components = {}; // 컴포넌트 맵

	// 변환 관련 멤버 뱐수
	DirectX::XMMATRIX m_worldMatrix = DirectX::XMMatrixIdentity(); // 월드 행렬
	DirectX::XMMATRIX m_positionMatrix = DirectX::XMMatrixIdentity(); // 위치 행렬
	DirectX::XMMATRIX m_rotationMatrix = DirectX::XMMatrixIdentity(); // 회전 행렬
	DirectX::XMMATRIX m_scaleMatrix = DirectX::XMMatrixIdentity(); // 스케일 행렬
	DirectX::XMVECTOR m_position = DirectX::XMVectorZero(); // 위치
	DirectX::XMVECTOR m_rotation = DirectX::XMVectorZero(); // 회전 (라디안 단위)
	DirectX::XMFLOAT3 m_scale = { 1.0f, 1.0f, 1.0f }; // 크기
	bool m_isDirty = true; // 위치 갱신 필요 여부

	// 렌더링 관련 멤버 변수
	com_ptr<ID3D11Buffer> m_constantBuffer = nullptr; // 상수 버퍼
	com_ptr<ID3D11Buffer> m_vertexBuffer = nullptr; // 버텍스 버퍼
	com_ptr<ID3D11Buffer> m_indexBuffer = nullptr; // 인덱스 버퍼
	UINT m_indexCount = 0; // 인덱스 개수
	com_ptr<ID3D11VertexShader> m_vertexShader = nullptr; // 버텍스 셰이더
	com_ptr<ID3D11InputLayout> m_inputLayout = nullptr; // 입력 레이아웃
	com_ptr<ID3D11PixelShader> m_pixelShader = nullptr; // 픽셀 셰이더
	bool m_isActive = false; // 활성화 여부

protected:
	struct RenderData // 렌더 정보 구조체
	{
		struct Vertex // 정점 구조체
		{
			DirectX::XMFLOAT4 position = {}; // 위치
			DirectX::XMFLOAT4 color = {}; // 색상
		};
		std::vector<Vertex> vertices = // 정점 테이터 // 기본값: 정육면체
		{
			{ .position = { -1.0f,  1.0f, -1.0f, 1.0f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f } }, // 앞면 왼쪽 위
			{ .position = {  1.0f,  1.0f, -1.0f, 1.0f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f } }, // 앞면 오른쪽 위
			{ .position = {  1.0f, -1.0f, -1.0f, 1.0f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f } }, // 앞면 오른쪽 아래
			{ .position = { -1.0f, -1.0f, -1.0f, 1.0f }, .color = { 1.0f, 1.0f, 0.0f, 1.0f } }, // 앞면 왼쪽 아래
			{ .position = { -1.0f,  1.0f,  1.0f, 1.0f }, .color = { 1.0f, 0.0f, 1.0f, 1.0f } }, // 뒷면 왼쪽 위
			{ .position = {  1.0f,  1.0f,  1.0f, 1.0f }, .color = { 0.0f, 1.0f, 1.0f, 1.0f } }, // 뒷면 오른쪽 위
			{ .position = {  1.0f, -1.0f,  1.0f, 1.0f }, .color = { 1.0f, 1.0f, 1.0f, 1.0f } }, // 뒷면 오른쪽 아래
			{ .position = { -1.0f, -1.0f,  1.0f, 1.0f }, .color = { 0.0f, 0.0f, 0.0f, 1.0f } }  // 뒷면 왼쪽 아래
		};
		std::vector<UINT> indices = // 인덱스 데이터 // 기본값: 정육면체
		{
			0, 1, 2, 0, 2, 3,       // 앞면
			4, 6, 5, 4, 7, 6,       // 뒷면
			4, 5, 1, 4, 1, 0,       // 윗면
			3, 2, 6, 3, 6, 7,       // 아랫면
			1, 5, 6, 1, 6, 2,       // 오른쪽 면
			4, 0, 3, 4, 3, 7        // 왼쪽 면
		};
		std::wstring vsShaderName = L"VSVertexColor.hlsl"; // 기본 버텍스 셰이더
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
				.SemanticName = "COLOR",
				.SemanticIndex = 0,
				.Format = DXGI_FORMAT_R32G32B32A32_FLOAT, // float4
				.InputSlot = 0,
				.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
				.InstanceDataStepRate = 0
			}
		};
		std::wstring psShaderName = L"PSVertexColor.hlsl"; // 기본 픽셀 셰이더
	};
	RenderData m_renderData = {}; // 렌더 정보

public:
	GameObjectBase();
	virtual ~GameObjectBase() { End(); }
	GameObjectBase(const GameObjectBase&) = delete;
	GameObjectBase& operator=(const GameObjectBase&) = delete;
	GameObjectBase(GameObjectBase&&) = default;
	GameObjectBase& operator=(GameObjectBase&&) = delete;

	UINT GetID() const { return m_id; }

	void Initialize() { Begin(); } // 게임 오브젝트 초기화
	virtual void Update(float deltaTime) {} // 매 프레임 업데이트
	void UpdateWorldMatrix(); // 월드 행렬 갱신
	void Render(DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix); // 렌더링

	template<typename T, typename... Args>
	T* AddComponent(Args&&... args)
	{
		auto component = std::make_unique<T>(std::forward<Args>(args)...);
		component->Initialize(this);

		T* componentPtr = component.get();
		m_components[std::type_index(typeid(T))] = std::move(component);

		return componentPtr;
	}
	template<typename T>
	T* GetComponent()
	{
		auto it = m_components.find(std::type_index(typeid(T)));
		if (it != m_components.end()) return static_cast<T*>(it->second.get());

		return nullptr;
	}
	template<typename T>
	void RemoveComponent()
	{
		auto it = m_components.find(std::type_index(typeid(T)));
		if (it != m_components.end()) m_components.erase(it);
	}

	void SetPosition(const DirectX::XMVECTOR& position) { m_position = position; m_isDirty = true; }
	void SetRotation(const DirectX::XMVECTOR& rotation) { m_rotation = rotation; m_isDirty = true; } // 라디안 단위
	void SetScale(const DirectX::XMFLOAT3& scale) { m_scale = scale; m_isDirty = true; }

	DirectX::XMVECTOR GetPosition() const { return m_position; }
	DirectX::XMVECTOR GetRotation() const { return m_rotation; } // 라디안 단위
	DirectX::XMFLOAT3 GetScale() const { return m_scale; }

	DirectX::XMMATRIX GetWorldMatrix() const { return m_worldMatrix; }

	void SetRenderData(const RenderData& renderData) { m_renderData = renderData; } // 렌더 정보 설정
	void GetRenderData(RenderData& outRenderData) const { outRenderData = m_renderData; } // 렌더 정보 조회
	void CreateRenderResources(); // 렌더 리소스 생성

protected:
	virtual void Begin() = 0; // 게임 오브젝트 시작 시 호출
	virtual void End() {} // 게임 오브젝트 종료 시 호출

private:
	// 임시로만 사용 // 나중에 더 좋은 방법으로 변경 필요
	void CreateConstantBuffer(); // 상수 버퍼 생성
	void CreateVertexBuffer(); // 버텍스 버퍼 생성
	void CreateIndexBuffer(); // 인덱스 버퍼 생성
	void CreateShaders(); // 셰이더 생성
	void SetActive(bool isActive) { m_isActive = isActive; } // 활성화 설정
};