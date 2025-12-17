#pragma once

class GameObjectBase;

class ComponentBase
{
	friend class GameObjectBase;

protected:
	GameObjectBase* m_owner = nullptr; // 소유 게임 오브젝트 포인터

public:
	ComponentBase() = default;
	virtual ~ComponentBase() = default;
	ComponentBase(const ComponentBase&) = default; // 복사
	ComponentBase& operator=(const ComponentBase&) = default; // 복사 대입
	ComponentBase(ComponentBase&&) = default; // 이동
	ComponentBase& operator=(ComponentBase&&) = default; // 이동 대입

protected:
	// 컴포넌트 초기화 // ComponentBase의 Initialize에서 호출
	virtual void Begin() {};
	// 매 프레임 RenderImGui에서 호출
	virtual void SerializeImGui() {};
	// 컴포넌트 Finalize에서 호출
	virtual void End() {};

private:
	void Initialize(GameObjectBase* owner) { m_owner = owner; Begin(); }
	// ImGui 렌더링 // GameObjectBase의 RenderImGui에서 호출
	void RenderImGui();
	void Finalize() { End(); }
};