#pragma once

class GameObjectBase;

class ComponentBase
{
	std::string m_typeName = "ComponentBase"; // 컴포넌트 타입 이름

protected:
	GameObjectBase* m_owner = nullptr; // 소유 게임 오브젝트 포인터

public:
	ComponentBase() = default;
	virtual ~ComponentBase() = default;
	ComponentBase(const ComponentBase&) = default; // 복사
	ComponentBase& operator=(const ComponentBase&) = default; // 복사 대입
	ComponentBase(ComponentBase&&) = default; // 이동
	ComponentBase& operator=(ComponentBase&&) = default; // 이동 대입

	void Initialize(GameObjectBase* owner);
	void Update(float deltaTime) { UpdateComponent(deltaTime); }
	// ImGui 렌더링 // GameObjectBase의 RenderImGui에서 호출
	void RenderImGui();
	void Finalize() { FinalizeComponent(); }

protected:
	// 컴포넌트 초기화 // ComponentBase의 Initialize에서 호출
	virtual void InitializeComponent() {};
	// 컴포넌트 업데이트 // Update에서 호출
	virtual void UpdateComponent(float deltaTime) {};
	// 컴포넌트 ImGui 렌더링 // RenderImGui에서 호출
	virtual void RenderImGuiComponent() {};
	// 컴포넌트 Finalize에서 호출
	virtual void FinalizeComponent() {};
};