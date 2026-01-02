#include "stdafx.h"
#include "TypeRegistry.h"

#include "SceneBase.h"
#include "GameObjectBase.h"
#include "ComponentBase.h"

using namespace std;

//unique_ptr<Base> TypeRegistry::Create(const string& typeName)
//{
//	auto it = m_registry.find(typeName);
//	if (it != m_registry.end()) return it->second();
//
//	#ifdef _DEBUG
//	cerr << "오류: 등록되지 않은 타입 이름 '" << typeName << "'입니다." << endl;
//	#else
//	MessageBoxA(nullptr, ("오류: 등록되지 않은 타입 이름 '" + typeName + "'입니다.").c_str(), "TypeRegistry Error", MB_OK | MB_ICONERROR);
//	#endif
//
//	exit(EXIT_FAILURE);
//	return nullptr;
//}

unique_ptr<SceneBase> TypeRegistry::CreateScene(const string& typeName)
{
	auto it = m_sceneRegistry.find(typeName);
	if (it != m_sceneRegistry.end()) return it->second();

	#ifdef _DEBUG
	cerr << "오류: 등록되지 않은 씬 타입 이름 '" << typeName << "'입니다." << endl;
	#else
	MessageBoxA(nullptr, ("오류: 등록되지 않은 씬 타입 이름 '" + typeName + "'입니다.").c_str(), "TypeRegistry Error", MB_OK | MB_ICONERROR);
	#endif

	exit(EXIT_FAILURE);
	return nullptr;
}

unique_ptr<GameObjectBase> TypeRegistry::CreateGameObject(const string& typeName)
{
	auto it = m_gameObjectRegistry.find(typeName);
	if (it != m_gameObjectRegistry.end()) return it->second();

	#ifdef _DEBUG
	cerr << "오류: 등록되지 않은 게임 오브젝트 타입 이름 '" << typeName << "'입니다." << endl;
	#else
	MessageBoxA(nullptr, ("오류: 등록되지 않은 게임 오브젝트 타입 이름 '" + typeName + "'입니다.").c_str(), "TypeRegistry Error", MB_OK | MB_ICONERROR);
	#endif

	exit(EXIT_FAILURE);
	return nullptr;
}

pair<unique_ptr<ComponentBase>, type_index> TypeRegistry::CreateComponent(const string& typeName)
{
	auto it = m_componentRegistry.find(typeName);
	if (it != m_componentRegistry.end()) return { it->second.first(), it->second.second };

	#ifdef _DEBUG
	cerr << "오류: 등록되지 않은 컴포넌트 타입 이름 '" << typeName << "'입니다." << endl;
	#else
	MessageBoxA(nullptr, ("오류: 등록되지 않은 컴포넌트 타입 이름 '" + typeName + "'입니다.").c_str(), "TypeRegistry Error", MB_OK | MB_ICONERROR);
	#endif

	exit(EXIT_FAILURE);
	return { nullptr, type_index(typeid(void)) };
}