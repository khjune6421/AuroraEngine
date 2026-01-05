#pragma once

// STL 헤더
#include <array>
#include <filesystem>
#include <fstream>
#include <functional>
#ifdef _DEBUG
#include <iostream>
#endif
#include <typeindex>
#include <unordered_map>

// 윈도우 헤더
#include <wrl/client.h>
#pragma comment(lib, "winmm.lib") // timeGetTime함수 사용을 위한 라이브러리

// DirectX 헤더
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi1_2.h>
#include <DirectXMath.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

// DirectXTK 헤더
#include <directxtk/SimpleMath.h>
#include <directxtk/WICTextureLoader.h>
#include <directxtk/DDSTextureLoader.h>

// Assimp 헤더
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

// ImGui 헤더
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

// JSON 헤더
#include <nlohmann/json.hpp>

// using 정의
#ifdef _DEBUG
using std::cout;
using std::endl;
#endif

// 메크로 정의
// com_ptr 매크로
#define com_ptr Microsoft::WRL::ComPtr

// 각도 변환 상수 및 함수
constexpr float DEG_TO_RAD = DirectX::XM_PI / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / DirectX::XM_PI;
inline DirectX::XMVECTOR ToRadians(const DirectX::XMVECTOR& degrees) { return DirectX::XMVectorScale(degrees, DEG_TO_RAD); }
inline DirectX::XMVECTOR ToDegrees(const DirectX::XMVECTOR& radians) { return DirectX::XMVectorScale(radians, RAD_TO_DEG); }

// 타입 이름 얻기 매크로
template<typename T>
// 템플릿 기반 타입 이름 얻기
inline std::string GetTypeName()
{
	std::string typeName = typeid(T).name();
	constexpr std::array<const char*, 4> prefixes = { "class ", "struct ", "union ", "enum " };
	for (const char* prefix : prefixes)
	{
		if (typeName.starts_with(prefix))
		{
			typeName = typeName.substr(std::strlen(prefix));
			break;
		}
	}
	return typeName;
}
template<typename T>
// 객체 기반 타입 이름 얻기
constexpr std::string GetTypeName(T& obj)
{
	std::string typeName = typeid(obj).name();
	constexpr std::array<const char*, 4> prefixes = { "class ", "struct ", "union ", "enum " };
	for (const char* prefix : prefixes)
	{
		if (typeName.starts_with(prefix))
		{
			typeName = typeName.substr(std::strlen(prefix));
			break;
		}
	}
	return typeName;
}