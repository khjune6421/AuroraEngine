/// GameObjectBase.h의 시작
#pragma once
#include "Base.h"
#include "ComponentBase.h"

enum class Direction // 방향 열거형
{
	Left, Right,
	Up, Down,
	Forward, Backward,

	Count
};

class GameObjectBase : public Base
{
	UINT m_id = 0; // 고유 ID
	std::string m_name = ""; // 이름

	// 변환 관련 멤버 변수
	DirectX::XMMATRIX m_worldMatrix = DirectX::XMMatrixIdentity(); // 월드 행렬
	DirectX::XMMATRIX m_positionMatrix = DirectX::XMMatrixIdentity(); // 위치 행렬
	DirectX::XMMATRIX m_rotationMatrix = DirectX::XMMatrixIdentity(); // 회전 행렬
	DirectX::XMMATRIX m_scaleMatrix = DirectX::XMMatrixIdentity(); // 스케일 행렬
	DirectX::XMMATRIX m_inverseScaleSquareMatrix = DirectX::XMMatrixIdentity(); // 스케일 역행렬 제곱 (법선 행렬 계산용)

	DirectX::XMVECTOR m_position = DirectX::XMVectorZero(); // 위치
	DirectX::XMVECTOR m_quaternion = DirectX::XMQuaternionIdentity(); // 쿼터니언
	DirectX::XMVECTOR m_euler = DirectX::XMVectorZero(); // 오일러
	DirectX::XMVECTOR m_scale = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f); // 크기
	bool m_isDirty = true; // 위치 갱신 필요 여부

	com_ptr<ID3D11DeviceContext> m_deviceContext = nullptr; // 디바이스 컨텍스트 포인터

	WorldNormalBuffer m_worldData = {}; // 월드 및 WVP 행렬 상수 버퍼 데이터
	com_ptr<ID3D11Buffer> m_worldMatrixConstantBuffer = nullptr; // 월드, WVP 행렬 상수 버퍼

	std::unordered_map<std::type_index, std::unique_ptr<Base>> m_components = {}; // 컴포넌트 맵
	std::vector<Base*> m_updateComponents = {}; // 업데이트할 컴포넌트 배열
	std::vector<Base*> m_renderComponents = {}; // 렌더링할 컴포넌트 배열

protected:
	GameObjectBase* m_parent = nullptr; // 부모 게임 오브젝트 포인터
	std::vector<std::unique_ptr<GameObjectBase>> m_childrens = {}; // 소유한 자식 게임 오브젝트 배열

public:
	GameObjectBase(); // 무조건 CreateGameObject로 생성
	virtual ~GameObjectBase() = default;
	GameObjectBase(const GameObjectBase&) = default; // 복사
	GameObjectBase& operator=(const GameObjectBase&) = default; // 복사 대입
	GameObjectBase(GameObjectBase&&) = default; // 이동
	GameObjectBase& operator=(GameObjectBase&&) = default; // 이동 대입

	UINT GetID() const { return m_id; }

	// 변환 관련 함수
	// 위치 지정
	void SetPosition(const DirectX::XMVECTOR& position) { m_position = position; SetDirty(); }
	const DirectX::XMVECTOR GetPosition() { return m_position; }
	// 위치 이동
	void MovePosition(const DirectX::XMVECTOR& deltaPosition) { m_position = DirectX::XMVectorAdd(m_position, deltaPosition); SetDirty(); }
	// 방향 이동
	void MoveDirection(float distance, Direction direction);
	// 위치 가져오기
	const DirectX::XMVECTOR& GetPosition() const { return m_position; }

	// 회전 지정
	void SetRotation(const DirectX::XMVECTOR& rotation);
	// 회전 이동
	void Rotate(const DirectX::XMVECTOR& deltaRotation);
	// 특정 위치 바라보기
	void LookAt(const DirectX::XMVECTOR& targetPosition, const DirectX::XMVECTOR& upDirection = { 0.0f, 1.0f, 0.0f, 0.0f });
	// 회전 가져오기
	const DirectX::XMVECTOR& GetRotation() const { return m_euler; }
	// 정규화된 방향 벡터 가져오기
	DirectX::XMVECTOR GetDirectionVector(Direction direction) const;
	// 월드 기준 정규화된 방향 벡터 가져오기
	DirectX::XMVECTOR GetWorldDirectionVector(Direction direction);

	// 크기 지정
	void SetScale(const DirectX::XMVECTOR& scale) { m_scale = scale; SetDirty(); }
	// 크기 변경
	void Scale(const DirectX::XMVECTOR& deltaScale) { m_scale = DirectX::XMVectorMultiply(m_scale, deltaScale); SetDirty(); }
	// 크기 가져오기
	const DirectX::XMVECTOR& GetScale() const { return m_scale; }

	const DirectX::XMMATRIX& GetWorldMatrix() const { return m_worldMatrix; }

	// 컴포넌트 추가 // 컴포넌트 베이스 포인터 반환
	ComponentBase* CreateComponent(const std::string& typeName);
	template<typename T> requires std::derived_from<T, ComponentBase>
	T* CreateComponent(); // 컴포넌트 추가 // 포인터 반환

	template<typename T> requires std::derived_from<T, ComponentBase>
	T* GetComponent(); // 컴포넌트 가져오기 // 없으면 nullptr 반환

	// 자식 게임 오브젝트 생성 // 게임 오브젝트 베이스 포인터 반환
	GameObjectBase* CreateChildGameObject(const std::string& typeName);
	template<typename T> requires std::derived_from<T, GameObjectBase>
	T* CreateChildGameObject(); // 자식 게임 오브젝트 생성 // 포인터 반환
	// 자식 게임 오브젝트 제거 // 제거 배열에 추가
	template<typename T> requires std::derived_from<T, GameObjectBase>
	[[nodiscard]] T* CreateChildGameObject(const std::string& typeName); // 자식 게임 오브젝트 생성 // 포인터 반환

private:
	// 게임 오브젝트 초기화
	void BaseInitialize() override;
	// 게임 오브젝트 업데이트
	void BaseUpdate() override;
	// 게임 오브젝트 렌더링
	void BaseRender() override;
	// ImGui 렌더링
	void BaseRenderImGui() override;
	// 게임 오브젝트 종료
	void BaseFinalize() override;

	// 게임 오브젝트 직렬화
	nlohmann::json BaseSerialize() override;
	// 게임 오브젝트 역직렬화
	void BaseDeserialize(const nlohmann::json& jsonData) override;

	// 제거 대기 중인 컴포넌트 및 자식 게임 오브젝트 제거
	void RemovePending() override;

	// 위치 갱신 필요로 설정 // 자식 게임 오브젝트도 설정
	void SetDirty();
	// 월드 행렬 갱신
	const DirectX::XMMATRIX& UpdateWorldMatrix();
};

template<typename T> requires std::derived_from<T, ComponentBase>
inline T* GameObjectBase::CreateComponent()
{
	if (m_components[std::type_index(typeid(T))])
	{
		#ifdef _DEBUG
		std::cerr << "오류: 게임 오브젝트 '" << m_name << "'에 이미 컴포넌트 '" << typeid(T).name() << "'가 존재합니다." << std::endl;
		#else
		MessageBoxA(nullptr, ("오류: 게임 오브젝트 '" + m_name + "'에 이미 컴포넌트 '" + typeid(T).name() + "'가 존재합니다.").c_str(), "GameObjectBase Error", MB_OK | MB_ICONERROR);
		#endif
		return nullptr;
	}

	std::unique_ptr<T> component = std::make_unique<T>();

	component->SetOwner(this);

	if (component->NeedsUpdate()) m_updateComponents.push_back(component.get());
	if (component->NeedsRender()) m_renderComponents.push_back(component.get());

	T* componentPtr = component.get();
	static_cast<Base*>(componentPtr)->BaseInitialize();

	m_components[std::type_index(typeid(T))] = std::move(component);

	return componentPtr;
}

template<typename T> requires std::derived_from<T, ComponentBase>
inline T* GameObjectBase::GetComponent()
{
	auto it = m_components.find(std::type_index(typeid(T)));
	if (it != m_components.end()) return static_cast<T*>(it->second.get());

	return nullptr;
}

template<typename T> requires std::derived_from<T, GameObjectBase>
inline T* GameObjectBase::CreateChildGameObject()
{
	std::unique_ptr<T> child = std::make_unique<T>();

	T* childPtr = child.get();
	child->m_parent = this;
	child->BaseInitialize();
	m_childrens.push_back(std::move(child));

	return childPtr;
}

template<typename T> requires std::derived_from<T, GameObjectBase>
inline T* GameObjectBase::CreateChildGameObject(const std::string& typeName)
{
	std::unique_ptr<GameObjectBase> childGameObjectPtr = TypeRegistry::GetInstance().CreateGameObject(typeName);

	childGameObjectPtr->m_parent = this;
	childGameObjectPtr->BaseInitialize();
	T* childPtr = static_cast<T*>(childGameObjectPtr.get());
	m_childrens.push_back(std::move(childGameObjectPtr));

	return childPtr;
}
/// GameObjectBase.h의 끝