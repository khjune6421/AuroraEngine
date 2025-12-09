#pragma once

class Renderer;

class GameObject // TODO: 부모-자식 관계 구현
{
	UINT m_id = 0; // 디버깅용 고유 ID

	// TODO: 컴포넌트 기반으로 수정

	// 변환 관련 멤버 뱐수
	DirectX::XMMATRIX m_worldMatrix = DirectX::XMMatrixIdentity(); // 월드 행렬
	DirectX::XMMATRIX m_positionMatrix = DirectX::XMMatrixIdentity(); // 위치 행렬
	DirectX::XMMATRIX m_rotationMatrix = DirectX::XMMatrixIdentity(); // 회전 행렬
	DirectX::XMMATRIX m_scaleMatrix = DirectX::XMMatrixIdentity(); // 스케일 행렬
	bool m_isDirty = true; // 위치 갱신 필요 여부

	// 렌더링 관련 멤버 변수
	Renderer* m_renderer = nullptr; // 렌더러 포인터
	com_ptr<ID3D11Device> m_device = nullptr; // 디바이스
	com_ptr<ID3D11DeviceContext> m_deviceContext = nullptr; // 디바이스 컨텍스트
	com_ptr<ID3D11Buffer> m_constantBuffer = nullptr; // 상수 버퍼
	com_ptr<ID3D11Buffer> m_vertexBuffer = nullptr; // 버텍스 버퍼
	com_ptr<ID3D11Buffer> m_indexBuffer = nullptr; // 인덱스 버퍼
	UINT m_indexCount = 0; // 인덱스 개수
	com_ptr<ID3D11InputLayout> m_inputLayout = nullptr; // 입력 레이아웃
	com_ptr<ID3D11VertexShader> m_vertexShader = nullptr; // 버텍스 셰이더
	com_ptr<ID3D11PixelShader> m_pixelShader = nullptr; // 픽셀 셰이더
	bool m_isActive = false; // 활성화 여부

protected:
	DirectX::XMVECTOR m_position = DirectX::XMVectorZero(); // 위치
	DirectX::XMVECTOR m_rotation = DirectX::XMVectorZero(); // 회전 (라디안 단위)
	DirectX::XMFLOAT3 m_scale = { 1.0f, 1.0f, 1.0f }; // 크기

public:
	GameObject();
	~GameObject() = default;
	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;
	GameObject(GameObject&&) = default;
	GameObject& operator=(GameObject&&) = delete;

	struct Vertex // 정점 구조체
	{
		DirectX::XMFLOAT4 position = {}; // 위치
		DirectX::XMFLOAT4 color = {}; // 색상
	};

	UINT GetID() const { return m_id; }

	void Initialize(Renderer* renderer); // 렌더러로부터 디바이스 및 컨텍스트 얻기

	void SetPosition(const DirectX::XMVECTOR& position) { m_position = position; m_isDirty = true; }
	void SetRotation(const DirectX::XMVECTOR& rotation) { m_rotation = rotation; m_isDirty = true; } // 라디안 단위
	void SetScale(const DirectX::XMFLOAT3& scale) { m_scale = scale; m_isDirty = true; }

	DirectX::XMVECTOR GetPosition() const { return m_position; }
	DirectX::XMVECTOR GetRotation() const { return m_rotation; } // 라디안 단위
	DirectX::XMFLOAT3 GetScale() const { return m_scale; }

	DirectX::XMMATRIX GetWorldMatrix() const { return m_worldMatrix; }

	// 임시로만 사용 // 나중에 더 좋은 방법으로 변경 필요
	void CreateConstantBuffer(); // 상수 버퍼 생성
	void CreateVertexBuffer(const std::vector<Vertex>& vertices); // 버텍스 버퍼 생성
	void CreateIndexBuffer(const std::vector<UINT>& indices); // 인덱스 버퍼 생성
	void CreateShaders(const std::wstring& vsShaderName, const std::wstring& psShaderName, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElementDescs); // 셰이더 생성
	void SetActive(bool isActive) { m_isActive = isActive; } // 활성화 설정

	virtual void Update(float deltaTime) {} // 매 프레임 업데이트
	void UpdateWorldMatrix(); // 월드 행렬 갱신
	void Render(DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix); // 렌더링
};