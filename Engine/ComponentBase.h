#pragma once

class GameObjectBase;

class ComponentBase
{
protected:
	GameObjectBase* m_owner = nullptr; // 소유 게임 오브젝트 포인터

public:
	ComponentBase() = default;
	virtual ~ComponentBase() { End(); }
	ComponentBase(const ComponentBase&) = delete;
	ComponentBase& operator=(const ComponentBase&) = delete;
	ComponentBase(ComponentBase&&) = delete;
	ComponentBase& operator=(ComponentBase&&) = delete;

	void Initialize(GameObjectBase* owner) { m_owner = owner; Begin(); }

protected:
	// 컴포넌트 초기화 // ComponentBase의 Initialize에서 호출
	virtual void Begin() {};
	virtual void End() {};
};