#pragma once
#include "GameObjectBase.h"

class SceneBase
{
	friend class SceneManager;

	std::vector<std::unique_ptr<GameObjectBase>> m_gameObjects = {}; // 게임 오브젝트 배열

	struct ViewProjectionBuffer // 뷰-투영 상수 버퍼 구조체
	{
		DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixIdentity(); // 뷰 행렬
		DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixIdentity(); // 투영 행렬
	};
	ViewProjectionBuffer m_viewProjectionData = {}; // 뷰-투영 상수 버퍼 데이터
	com_ptr<ID3D11Buffer> m_viewProjectionConstantBuffer = nullptr; // 뷰-투영 상수 버퍼

protected:
	std::string m_name = "Scene"; // 씬 이름
	class CameraComponent* m_mainCamera = nullptr; // 메인 카메라 컴포넌트 포인터
	std::array<FLOAT, 4> m_clearColor = { 0.5f, 0.5f, 0.5f, 1.0f }; // 씬 클리어 색상

public:
	SceneBase() = default; // 무조건 SceneManager로 생성
	virtual ~SceneBase() = default;
	SceneBase(const SceneBase&) = delete; // 복사 금지
	SceneBase& operator=(const SceneBase&) = delete; // 복사 대입 금지
	SceneBase(SceneBase&&) = delete; // 이동 금지
	SceneBase& operator=(SceneBase&&) = delete; // 이동 대입 금지

	template<typename T, typename... Args>
	T* CreateGameObject(Args&&... args);
	void RemoveGameObject(GameObjectBase* gameObject); // TODO: 뭔가 좀 부족함

protected:
	// 메인 카메라 게임 오브젝트 설정
	virtual GameObjectBase* CreateCameraObject();
	virtual void Begin() {}
	// 매 프레임 RenderImGui에서 호출
	virtual void SerializeImGui() {}
	virtual void End() {}

private:
	// 씬 초기화 // 씬 사용 전 반드시 호출해야 함
	void Initialize();

	// 씬 업데이트 // 매 프레임 씬 매니저가 호출
	void Update(float deltaTime);
	// 게임 오브젝트 변환 갱신 // 매 프레임 씬 매니저가 호출
	void TransformGameObjects();
	// 씬 렌더링 // 매 프레임 씬 매니저가 호출
	void Render();
	// ImGui 렌더링
	void RenderImGui();

	// 씬 종료 // 씬 매니저가 씬을 교체할 때 호출
	void Finalize();
};

template<typename T, typename ...Args>
inline T* SceneBase::CreateGameObject(Args && ...args)
{
	auto gameObject = std::make_unique<T>(std::forward<Args>(args)...);

	gameObject->Initialize();
	T* gameObjectPtr = gameObject.get();
	m_gameObjects.push_back(move(gameObject));

	return gameObjectPtr;
}