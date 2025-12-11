#pragma once
#include "ComponentBase.h"

class Renderer;

class GameObjectBase // TODO: 부모-자식 관계 구현
{
	UINT m_id = 0; // 디버깅용 고유 ID

	// 변환 관련 멤버 뱐수
	DirectX::XMMATRIX m_worldMatrix = DirectX::XMMatrixIdentity(); // 월드 행렬
	DirectX::XMMATRIX m_positionMatrix = DirectX::XMMatrixIdentity(); // 위치 행렬
	DirectX::XMMATRIX m_rotationMatrix = DirectX::XMMatrixIdentity(); // 회전 행렬
	DirectX::XMMATRIX m_scaleMatrix = DirectX::XMMatrixIdentity(); // 스케일 행렬
	DirectX::XMVECTOR m_position = DirectX::XMVectorZero(); // 위치
	DirectX::XMVECTOR m_rotation = DirectX::XMVectorZero(); // 회전 (라디안 단위)
	DirectX::XMFLOAT3 m_scale = { 1.0f, 1.0f, 1.0f }; // 크기
	bool m_isDirty = true; // 위치 갱신 필요 여부

	struct WorldWVPBuffer // 월드 및 WVP 행렬 상수 버퍼 구조체
	{
		DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity(); // 월드 행렬
		DirectX::XMMATRIX WVPMatrix = DirectX::XMMatrixIdentity(); // WVP 행렬
	};
	WorldWVPBuffer m_worldWVPData = {}; // 월드 및 WVP 행렬 상수 버퍼 데이터
	com_ptr<ID3D11Buffer> m_worldWVPConstantBuffer = nullptr; // 월드, WVP 행렬 상수 버퍼

	std::unordered_map<std::type_index, std::unique_ptr<class ComponentBase>> m_components = {}; // 컴포넌트 맵

public:
	GameObjectBase();
	virtual ~GameObjectBase() { End(); }
	GameObjectBase(const GameObjectBase&) = delete;
	GameObjectBase& operator=(const GameObjectBase&) = delete;
	GameObjectBase(GameObjectBase&&) = default;
	GameObjectBase& operator=(GameObjectBase&&) = delete;

	UINT GetID() const { return m_id; }

	// 변환 관련 함수
	// 위치 지정
	void SetPosition(const DirectX::XMVECTOR& position) { m_position = position; m_isDirty = true; }
	// 회전 지정 (라디안 단위)
	void SetRotation(const DirectX::XMVECTOR& rotation) { m_rotation = rotation; m_isDirty = true; }
	// 크기 지정
	void SetScale(const DirectX::XMFLOAT3& scale) { m_scale = scale; m_isDirty = true; }

	DirectX::XMVECTOR GetPosition() const { return m_position; }
	DirectX::XMVECTOR GetRotation() const { return m_rotation; }
	DirectX::XMFLOAT3 GetScale() const { return m_scale; }

	DirectX::XMMATRIX GetWorldMatrix() const { return m_worldMatrix; }

	template<typename T, typename... Args>
	T* AddComponent(Args&&... args); // 컴포넌트 추가
	template<typename T>
	T* GetComponent(); // 컴포넌트 가져오기 // 없으면 nullptr 반환
	template<typename T>
	void RemoveComponent(); // 컴포넌트 제거

protected:
	// 게임 오브젝트 Initialize에서 호출
	virtual void Begin() {}
	// 게임 오브젝트 소멸자가 호출
	virtual void End() {}

	friend class SceneBase; // 씬이 게임 오브젝트의 Initialize, UpdateWorldMatrix, Render를 호출할 수 있도록 함
	// 매 프레임 씬 Render에서 호출
	virtual void Update(float deltaTime) {}

private:
	// 게임 오브젝트 초기화 // 씬이 AddGameObject에서 호출
	void Initialize();
	// 월드 행렬 갱신 // 씬이 TransformGameObjects에서 호출
	void UpdateWorldMatrix();
	// 렌더링 // 씬이 Render에서 호출
	void Render(DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix);
};

template<typename T, typename... Args>
T* GameObjectBase::AddComponent(Args&&... args)
{
	auto component = std::make_unique<T>(std::forward<Args>(args)...);
	component->Initialize(this);
	T* componentPtr = component.get();
	m_components[std::type_index(typeid(T))] = std::move(component);
	return componentPtr;
}

template<typename T>
T* GameObjectBase::GetComponent()
{
	auto it = m_components.find(std::type_index(typeid(T)));
	if (it != m_components.end()) return static_cast<T*>(it->second.get());
	return nullptr;
}

template<typename T>
void GameObjectBase::RemoveComponent()
{
	auto it = m_components.find(std::type_index(typeid(T)));
	if (it != m_components.end()) m_components.erase(it);
}