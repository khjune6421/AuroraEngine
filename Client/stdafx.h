#pragma once

// STL 헤더
#include <array>
#include <filesystem>
#include <iostream>
#include <typeindex>

// 윈도우 헤더
#include <wrl/client.h>

// DirectX 헤더
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi1_2.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

// DirectX 툴킷 헤더
#include <SimpleMath.h>
#pragma comment(lib, "DirectXTK.lib")

// 기타 사용자 정의 헤더
#include "SingletonBase.h"

// 메크로 정의
#define com_ptr Microsoft::WRL::ComPtr