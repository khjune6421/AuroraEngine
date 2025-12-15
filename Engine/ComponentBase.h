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
	ComponentBase(const ComponentBase&) = delete; // 복사 금지
	ComponentBase& operator=(const ComponentBase&) = delete; // 복사 대입 금지
	ComponentBase(ComponentBase&&) = delete; // 이동 금지
	ComponentBase& operator=(ComponentBase&&) = delete; // 이동 대입 금지

protected:
	// 컴포넌트 초기화 // ComponentBase의 Initialize에서 호출
	virtual void Begin() {};
	virtual void End() {};

private:
	void Initialize(GameObjectBase* owner) { m_owner = owner; Begin(); }
	void Finalize() { End(); }
};