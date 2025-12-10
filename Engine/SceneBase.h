#pragma once
#include "MainCamera.h"

class SceneBase
{
	std::vector<std::unique_ptr<class GameObjectBase>> m_gameObjects = {}; // 게임 오브젝트 배열

	struct ViewProjectionBuffer // 뷰-투영 상수 버퍼 구조체
	{
		DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixIdentity(); // 뷰 행렬
		DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixIdentity(); // 투영 행렬
	};
	ViewProjectionBuffer m_viewProjectionData = {}; // 뷰-투영 상수 버퍼 데이터
	com_ptr<ID3D11Buffer> m_viewProjectionConstantBuffer = nullptr; // 뷰-투영 상수 버퍼

protected:
	MainCamera m_mainCamera; // 테스트용 메인 카메라
	std::array<FLOAT, 4> m_clearColor = { 0.5f, 0.5f, 0.5f, 1.0f }; // 씬 클리어 색상

public:
	SceneBase() = default;
	virtual ~SceneBase() { End(); }
	SceneBase(const SceneBase&) = delete;
	SceneBase& operator=(const SceneBase&) = delete;
	SceneBase(SceneBase&&) = delete;
	SceneBase& operator=(SceneBase&&) = delete;

	// 씬 초기화 // 씬 사용 전 반드시 호출해야 함
	void Initialize();
	void Update(float deltaTime);
	void TransformGameObjects();
	void Render();

protected:
	virtual void Begin() = 0;
	virtual void End() {};

	void AddGameObject(std::unique_ptr<class GameObjectBase> gameObject);
};