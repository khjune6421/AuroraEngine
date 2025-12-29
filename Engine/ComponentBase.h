#pragma once
#include "IBase.h"

class ComponentBase : public IBase
{
	std::string m_typeName = "ComponentBase"; // 컴포넌트 타입 이름

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

	virtual bool NeedsUpdate() const = 0;
	virtual bool NeedsRender() const = 0;

private:
	void BaseInitialize() override;
	void BaseUpdate(float deltaTime) override { Update(deltaTime); }
	void BaseRender() override { Render(); }
	// ImGui 렌더링 // GameObjectBase의 RenderImGui에서 호출
	void BaseRenderImGui() override;
	void BaseFinalize() override { Finalize(); }
};