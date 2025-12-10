#pragma once
#include "MainCamera.h"

class SceneBase
{
	Renderer* m_renderer = nullptr; // 렌더러 포인터
	std::vector<std::unique_ptr<class GameObject>> m_gameObjects = {}; // 게임 오브젝트 배열

protected:
	MainCamera m_mainCamera; // 테스트용 메인 카메라
	std::array<FLOAT, 4> m_clearColor = { 0.5f, 0.5f, 0.5f, 1.0f }; // 씬 클리어 색상

public:
	SceneBase() = default;
	virtual ~SceneBase() = default;
	SceneBase(const SceneBase&) = delete;
	SceneBase& operator=(const SceneBase&) = delete;
	SceneBase(SceneBase&&) = delete;
	SceneBase& operator=(SceneBase&&) = delete;

	// 씬 초기화 // 씬 사용 전 반드시 호출해야 함
	void Initialize(Renderer* renderer) { m_renderer = renderer; Begin(); }
	virtual void Begin() = 0;

	void Update(float deltaTime);
	void TransformGameObjects();
	void Render();

protected:
	void AddGameObject(std::unique_ptr<class GameObject> gameObject);
};