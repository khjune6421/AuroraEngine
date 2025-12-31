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

	// 타입 이름 가져오기
	std::string GetTypeName() const;

protected:
	virtual void Initialize() {}
	virtual void Update() {}
	virtual void Render() {}
	virtual void RenderImGui() {}
	virtual void Finalize() {}

	template<typename T> requires std::derived_from<T, Base>
	static void Register(const std::string& typeName) { s_registry[typeName] = []() -> std::unique_ptr<Base> { return std::make_unique<T>(); }; }
	static std::unique_ptr<Base> Create(const std::string& typeName);

	virtual nlohmann::json Serialize() { return nlohmann::json(); }
	virtual void Deserialize(const nlohmann::json& jsonData) {}

private:
	static std::unordered_map<std::string, std::function<std::unique_ptr<Base>()>> s_registry;
};