#pragma once

class IBase
{
public:
	IBase() = default;
	virtual ~IBase() = default;
	IBase(const IBase&) = default;
	IBase& operator=(const IBase&) = default;
	IBase(IBase&&) = default;
	IBase& operator=(IBase&&) = default;

	virtual void BaseInitialize() = 0;
	virtual void BaseUpdate(float deltaTime) = 0;
	virtual void BaseRender() = 0;
	virtual void BaseRenderImGui() = 0;
	virtual void BaseFinalize() = 0;

protected:
	virtual void Initialize() {}
	virtual void Update(float deltaTime) {}
	virtual void Render() {}
	virtual void RenderImGui() {}
	virtual void Finalize() {}
};