#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

// STL 헤더
#include <array>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iostream>
#include <typeindex>
#include <unordered_map>
#include <queue>
#include <optional>
#include <cstdint>
#include <algorithm>
#include <random>
#include <memory>

// 윈도우 헤더
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <mmsystem.h>
#include <wrl/client.h>
#pragma comment(lib, "winmm.lib") // timeGetTime

// DirectX 헤더
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi1_2.h>
#include <DirectXMath.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

// Time
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

// DirectXTK 헤더
#include <directxtk/SimpleMath.h>
#include <directxtk/WICTextureLoader.h>
#include <directxtk/DDSTextureLoader.h>
#include <directxtk/SpriteBatch.h>
#include <directxtk/SpriteFont.h>

// Assimp 헤더
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/config.h>

// ImGui 헤더
#ifdef _DEBUG
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

// ImGuizmo 헤더
#include <ImGuizmo.h>
#endif

// JSON 헤더
#include <nlohmann/json.hpp>

//fmod 헤더
#ifdef _DEBUG
#pragma comment(lib, "fmodL_vc.lib")
#else
#pragma comment(lib, "fmod_vc.lib")
#endif
#include <fmod/fmod.hpp>
#include <fmod/fmod_error_wrap.h>
#include <fftw3.h>

// 메크로 정의
// com_ptr 매크로
#define com_ptr Microsoft::WRL::ComPtr

// 각도 변환 상수 및 함수
constexpr float DEG_TO_RAD = DirectX::XM_PI / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / DirectX::XM_PI;
inline DirectX::XMVECTOR ToRadians(const DirectX::XMVECTOR& degrees) { return DirectX::XMVectorScale(degrees, DEG_TO_RAD); }
inline DirectX::XMVECTOR ToDegrees(const DirectX::XMVECTOR& radians) { return DirectX::XMVectorScale(radians, RAD_TO_DEG); }

// 타입 이름 얻기 매크로

// 템플릿 기반 타입 이름 얻기
template<typename T>
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
// 객체 기반 타입 이름 얻기
template<typename T>
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

// use this define when release GAME + 추가로 진입점 변경까지 필수임
#define GAMERELEASE

#ifdef _DEBUG
	#define LOG(msg) do { std::cout << msg << std::endl; } while (0)
	#define LOG_ERROR(msg) do { std::cerr << msg << std::endl; } while (0)
#else
	#ifdef GAMERELEASE
		#define LOG(msg) do { } while (0)
		#define LOG_ERROR(msg) do { } while (0)
	#else
		#define LOG(msg) do { std::cout << msg << std::endl; } while (0)
		#define LOG_ERROR(msg) do { std::cerr << msg << std::endl; } while (0)
	#endif
#endif

// HRESULT result check
constexpr void CheckResult(HRESULT hr, const char* msg)
{
	if (FAILED(hr))
	{
		LOG_ERROR(msg << " 에러 코드: " << std::hex << hr);
		exit(EXIT_FAILURE);
	}
}

// common definitions
#include "Singleton.h"