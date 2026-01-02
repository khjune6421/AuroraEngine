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
	virtual void BaseUpdate() = 0;
	virtual void BaseRender() = 0;
	virtual void BaseRenderImGui() = 0;
	virtual void BaseFinalize() = 0;

	virtual nlohmann::json BaseSerialize() { return nlohmann::json(); } // = 0;
	virtual void BaseDeserialize(const nlohmann::json& jsonData) {} // = 0;
};

class Base : public IBase
{
protected:
	std::string m_type = "Base"; // 타입 이름

public:
	Base() = default;
	virtual ~Base() = default;
	Base(const Base&) = default;
	Base& operator=(const Base&) = default;
	Base(Base&&) = default;
	Base& operator=(Base&&) = default;

	std::string GetType() const { return m_type; }

protected:
	// 파생 클래스의 초기화
	virtual void Initialize() {}
	// 파생 클래스의 업데이트
	virtual void Update() {}
	// 파생 클래스의 렌더링
	virtual void Render() {}
	// 파생 클래스의 ImGui 렌더링
	virtual void RenderImGui() {}
	// 파생 클래스의 종료
	virtual void Finalize() {}

	virtual nlohmann::json Serialize() { return nlohmann::json(); }
	virtual void Deserialize(const nlohmann::json& jsonData) {}
};