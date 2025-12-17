#pragma once

// STL 헤더
#include <array>
#include <filesystem>
#include <iostream>
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

// DirectX 툴킷 헤더
#include <directxtk/SimpleMath.h>

// Assimp 헤더
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

// ImGui 헤더
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

// 기타 사용자 정의 헤더
#include "SingletonBase.h"

// 메크로 정의
#define com_ptr Microsoft::WRL::ComPtr

// HRESULT 결과 확인
constexpr void CheckResult(HRESULT hr, const char* msg)
{
	if (FAILED(hr))
	{
		#ifdef _DEBUG
		std::cerr << msg << " 에러 코드: " << std::hex << hr << std::endl;
		#else
		MessageBoxA(nullptr, msg, "오류", MB_OK | MB_ICONERROR);
		#endif
		exit(EXIT_FAILURE);
	}
}