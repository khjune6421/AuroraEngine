#include "stdafx.h"
#include "TypeRegistry.h"

using namespace std;

unique_ptr<Base> TypeRegistry::Create(const string& typeName)
{
	auto it = m_registry.find(typeName);
	if (it != m_registry.end()) return it->second();

	#ifdef _DEBUG
	std::cerr << "오류: 등록되지 않은 타입 이름 '" << typeName << "'입니다." << std::endl;
	#else
	MessageBoxA(nullptr, ("오류: 등록되지 않은 타입 이름 '" + typeName + "'입니다.").c_str(), "TypeRegistry Error", MB_OK | MB_ICONERROR);
	#endif

	exit(EXIT_FAILURE);
	return nullptr;
}