#include "stdafx.h"
#include "Base.h"

using namespace std;

unordered_map<string, function<unique_ptr<Base>()>> Base::s_registry = {};

string Base::GetTypeName() const
{
	string typeName = typeid(*this).name();
	if (typeName.find("class ") == 0) typeName = typeName.substr(6);

	return typeName;
}

unique_ptr<Base> Base::Create(const string& typeName)
{
	auto it = s_registry.find(typeName);
	if (it != s_registry.end()) return it->second();

	#ifdef _DEBUG
	cerr << "Error: 타입 '" << typeName << "' 을(를) 찾을 수 없습니다." << endl;
	#else
	MessageBoxA(nullptr, ("Error: 타입 '" + typeName + "' 을(를) 찾을 수 없습니다.").c_str(), "Fatal Error", MB_OK | MB_ICONERROR);
	#endif
	exit(EXIT_FAILURE);

	return nullptr;
}