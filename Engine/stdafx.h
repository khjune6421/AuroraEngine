#pragma once

// STL 헤더
#include <array>
#include <filesystem>
#include <iostream>

// 윈도우 헤더
#include <wrl/client.h>

// DirectX 헤더
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi1_2.h>

// DirectX 툴킷 헤더
#include <SimpleMath.h>
#pragma comment(lib, "DirectXTK.lib")

// 메크로 정의
#define com_ptr Microsoft::WRL::ComPtr