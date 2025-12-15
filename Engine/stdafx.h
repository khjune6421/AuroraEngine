#pragma once

// STL 헤더
#include <array>
#include <filesystem>
#include <iostream>
#include <typeindex>

// 윈도우 헤더
#include <wrl/client.h>
#pragma comment(lib, "winmm.lib") // timeGetTime함수 사용을 위한 라이브러리

// DirectX 헤더
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi1_2.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

// DirectX 툴킷 헤더
#include <directxtk/SimpleMath.h>

// Assimp 헤더
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

// 기타 사용자 정의 헤더
#include "SingletonBase.h"

// 메크로 정의
#define com_ptr Microsoft::WRL::ComPtr

// HRESULT 결과 확인
void CheckResult(HRESULT hr, const char* msg);