#pragma once
#include "Base.h"

class ComponentBase : public Base
{
protected:
	class GameObjectBase* m_owner = nullptr; // 소유 게임 오브젝트 포인터

public:
	ComponentBase() = default;
	virtual ~ComponentBase() = default;
	ComponentBase(const ComponentBase&) = default; // 복사
	ComponentBase& operator=(const ComponentBase&) = default; // 복사 대입
	ComponentBase(ComponentBase&&) = default; // 이동
	ComponentBase& operator=(ComponentBase&&) = default; // 이동 대입

	void SetOwner(GameObjectBase* owner) { m_owner = owner; }
	GameObjectBase* GetOwner() const { return m_owner; }
	virtual bool NeedsUpdate() const = 0;
	virtual bool NeedsRender() const = 0;
  
private:
	void BaseInitialize() override;
	void BaseUpdate() override { Update(); }
	void BaseRender() override { Render(); }
	void BaseRenderImGui() override;
	void BaseFinalize() override { Finalize(); }

	// 컴포넌트 직렬화
	nlohmann::json BaseSerialize() override;
	// 컴포넌트 역직렬화
	void BaseDeserialize(const nlohmann::json& jsonData) override { Deserialize(jsonData); }

	// 컴포넌트는 제거 대기 처리가 필요 없으므로 빈 구현
	void RemovePending() override {};
};