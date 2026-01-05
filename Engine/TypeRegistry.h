#pragma once
#include "Singleton.h"

class TypeRegistry : public Singleton<TypeRegistry>
{
	friend class Singleton<TypeRegistry>;

public:
	~TypeRegistry() = default;
	TypeRegistry(const TypeRegistry&) = delete;
	TypeRegistry& operator=(const TypeRegistry&) = delete;
	TypeRegistry(TypeRegistry&&) = delete;
	TypeRegistry& operator=(TypeRegistry&&) = delete;

	std::unordered_map<std::string, std::function<std::unique_ptr<class SceneBase>()>> m_sceneRegistry;
	std::unordered_map<std::string, std::function<std::unique_ptr<class GameObjectBase>()>> m_gameObjectRegistry;
	std::unordered_map<std::string, std::function<std::unique_ptr<class ComponentBase>()>> m_componentRegistry;

	template<typename T> requires std::derived_from<T, SceneBase>
	void Register() { m_sceneRegistry[GetTypeName<T>()] = []() -> std::unique_ptr<SceneBase> { return std::make_unique<T>(); }; }
	template<typename T> requires std::derived_from<T, GameObjectBase>
	void Register() { m_gameObjectRegistry[GetTypeName<T>()] = []() -> std::unique_ptr<GameObjectBase> { return std::make_unique<T>(); }; }
	template<typename T> requires std::derived_from<T, ComponentBase>
	void Register() { m_componentRegistry[GetTypeName<T>()] = []() -> std::unique_ptr<ComponentBase> { return std::make_unique<T>(); }; }

	std::unique_ptr<SceneBase> CreateScene(const std::string& typeName);
	std::unique_ptr<GameObjectBase> CreateGameObject(const std::string& typeName);
	std::unique_ptr<ComponentBase> CreateComponent(const std::string& typeName);

private:
	TypeRegistry() = default;
};

#define REGISTER_TYPE(Type) namespace { bool s_##Type##_registered = []() { TypeRegistry::GetInstance().Register<Type>(); return true; }(); }