#pragma once
#include "ComponentBase.h"

class Renderer;

class GameObjectBase
{
	friend class SceneBase;

	UINT m_id = 0; // 고유 ID
	std::string m_typeName = "GameObjectBase"; // 게임 오브젝트 타입 이름

	// 변환 관련 멤버 뱐수
	DirectX::XMMATRIX m_worldMatrix = DirectX::XMMatrixIdentity(); // 월드 행렬
	DirectX::XMMATRIX m_positionMatrix = DirectX::XMMatrixIdentity(); // 위치 행렬
	DirectX::XMMATRIX m_rotationMatrix = DirectX::XMMatrixIdentity(); // 회전 행렬
	DirectX::XMMATRIX m_scaleMatrix = DirectX::XMMatrixIdentity(); // 스케일 행렬
	DirectX::XMMATRIX m_inverseScaleSquareMatrix = DirectX::XMMatrixIdentity(); // 스케일 역행렬 제곱 (법선 행렬 계산용)

	DirectX::XMVECTOR m_position = DirectX::XMVectorZero(); // 위치
	DirectX::XMVECTOR m_quaternion = DirectX::XMQuaternionIdentity(); // 쿼터니언
	DirectX::XMVECTOR m_euler = DirectX::XMVectorZero(); // 오일러 각도 // 라디안 단위
	DirectX::XMVECTOR m_scale = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f); // 크기
	bool m_isDirty = true; // 위치 갱신 필요 여부

	struct WorldBuffer // 월드 및 WVP 행렬 상수 버퍼 구조체
	{
		DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity(); // 월드 행렬
		DirectX::XMMATRIX normalMatrix = DirectX::XMMatrixIdentity(); // 스케일 역행렬을 적용한 월드 행렬
		DirectX::XMMATRIX WVPMatrix = DirectX::XMMatrixIdentity(); // WVP 행렬
	};
	WorldBuffer m_worldData = {}; // 월드 및 WVP 행렬 상수 버퍼 데이터
	com_ptr<ID3D11Buffer> m_worldWVPConstantBuffer = nullptr; // 월드, WVP 행렬 상수 버퍼

	std::unordered_map<std::type_index, std::unique_ptr<ComponentBase>> m_components = {}; // 컴포넌트 맵

protected:
	SceneBase* m_scene = nullptr; // 소유 씬 포인터
	GameObjectBase* m_parent = nullptr; // 부모 게임 오브젝트 포인터
	std::vector<GameObjectBase*> m_children = {}; // 자식 게임 오브젝트 배열

public:
	GameObjectBase(); // 무조건 CreateGameObject로 생성
	virtual ~GameObjectBase(); // 컴포넌트 제거
	GameObjectBase(const GameObjectBase&) = default; // 복사
	GameObjectBase& operator=(const GameObjectBase&) = default; // 복사 대입
	GameObjectBase(GameObjectBase&&) = default; // 이동
	GameObjectBase& operator=(GameObjectBase&&) = default; // 이동 대입

	UINT GetID() const { return m_id; }

	// 변환 관련 함수
	// 위치 지정
	void SetPosition(const DirectX::XMVECTOR& position) { m_position = position; SetDirty(); }
	// 위치 가져오기
	DirectX::XMVECTOR GetPosition() const { return m_position; }
	// 위치 이동
	void MovePosition(const DirectX::XMVECTOR& deltaPosition) { m_position = DirectX::XMVectorAdd(m_position, deltaPosition); SetDirty(); }
	enum class Direction // 방향 열거형
	{
		Forward, Backward,
		Left, Right,
		Up, Down
	};
	// 방향 이동
	void MoveDirection(float distance, Direction direction);

	// 회전 지정 // 라디안 단위
	void SetRotation(const DirectX::XMVECTOR& rotation) { m_euler = rotation; SetDirty(); }
	// 회전 가져오기 // 라디안 단위
	DirectX::XMVECTOR GetRotation() const { return m_euler; }
	// 회전 이동 // 라디안 단위
	void Rotate(const DirectX::XMVECTOR& deltaRotation) { m_euler = DirectX::XMVectorAdd(m_euler, deltaRotation); SetDirty(); }
	// 특정 위치 바라보기
	void LookAt(const DirectX::XMVECTOR& targetPosition);
	// 정규화된 방향 벡터 가져오기
	DirectX::XMVECTOR GetDirectionVector(Direction direction);

	// 크기 지정
	void SetScale(const DirectX::XMVECTOR& scale) { m_scale = scale; SetDirty(); }
	// 크기 가져오기
	DirectX::XMVECTOR GetScale() const { return m_scale; }
	// 크기 변경
	void Scale(const DirectX::XMVECTOR& deltaScale) { m_scale = DirectX::XMVectorMultiply(m_scale, deltaScale); SetDirty(); }

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
	// 매 프레임 씬 Render에서 호출
	virtual void Update(float deltaTime) {}
	// 매 프레임 RenderImGui에서 호출
	virtual void SerializeImGui() {}
	// 게임 오브젝트 소멸자가 호출
	virtual void End() {}

private:
	void SetDirty() { m_isDirty = true; } // 위치 갱신 필요로 설정

	// 게임 오브젝트 초기화 // 씬이 CreateGameObject에서 호출
	void Initialize(SceneBase* scene);

	// 월드 행렬 갱신 // 씬이 TransformGameObjects에서 호출
	void UpdateWorldMatrix();
	// 렌더링 // 씬이 Render에서 호출
	void Render(DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix);
	// ImGui 렌더링 // 씬이 RenderImGui에서 호출
	void RenderImGui();

	// 게임 오브젝트 종료 // 씬이 호출
	void Finalize();
};

template<typename T, typename ...Args>
inline T* GameObjectBase::AddComponent(Args && ...args)
{
	auto component = std::make_unique<T>(std::forward<Args>(args)...);

	component->Initialize(this);
	T* componentPtr = component.get();
	m_components[std::type_index(typeid(T))] = std::move(component);

	return componentPtr;
}

template<typename T>
inline T* GameObjectBase::GetComponent()
{
	auto it = m_components.find(std::type_index(typeid(T)));
	if (it != m_components.end()) return static_cast<T*>(it->second.get());

	return nullptr;
}

template<typename T>
inline void GameObjectBase::RemoveComponent()
{
	auto it = m_components.find(std::type_index(typeid(T)));
	if (it != m_components.end())
	{
		it->second->Finalize();
		m_components.erase(it);
	}
}