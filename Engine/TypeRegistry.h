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

	template<typename T> requires std::is_base_of_v<T, Base>
	std::unique_ptr<T> Create(const std::string& typeName);

private:
	TypeRegistry() = default;
};

template<typename T> requires std::is_base_of_v<T, Base>
inline std::unique_ptr<T> TypeRegistry::Create(const std::string& typeName)
{
	auto it = m_registry.find(typeName);
	if (it != m_registry.end()) return std::unique_ptr<T>(static_cast<T*>(it->second().release()));

	#ifdef _DEBUG
	std::cerr << "Error: 타입 '" << typeName << "' 을(를) 찾을 수 없습니다." << std::endl;
	#else
	MessageBoxA(nullptr, ("Error: 타입 '" + typeName + "' 을(를) 찾을 수 없습니다.").c_str(), "Fatal Error", MB_OK | MB_ICONERROR);
	#endif

	exit(EXIT_FAILURE);
	return nullptr;
}

#define REGISTER_TYPE(Type) namespace { bool s_##Type##_registered = []() { TypeRegistry::GetInstance().Register<Type>(#Type); return true;	}(); }