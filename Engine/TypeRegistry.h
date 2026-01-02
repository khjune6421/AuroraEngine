#pragma once

class TypeRegistry : public Singleton<TypeRegistry>
{
	friend class Singleton<TypeRegistry>;

	std::unordered_map<std::string, std::function<std::unique_ptr<Base>()>> m_registry;

public:
	~TypeRegistry() = default;
	TypeRegistry(const TypeRegistry&) = delete;
	TypeRegistry& operator=(const TypeRegistry&) = delete;
	TypeRegistry(TypeRegistry&&) = delete;
	TypeRegistry& operator=(TypeRegistry&&) = delete;

	template<typename T> requires std::derived_from<T, Base>
	void Register(const std::string& typeName) { m_registry[typeName] = []() -> std::unique_ptr<Base> { return std::make_unique<T>(); }; }

	std::unique_ptr<Base> Create(const std::string& typeName);

private:
	TypeRegistry() = default;
};

#define REGISTER_TYPE(Type) namespace { bool s_##Type##_registered = []() { TypeRegistry::GetInstance().Register<Type>(#Type); return true;	}(); }