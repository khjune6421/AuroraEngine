/// WindowManager.h의 시작
#include "stdafx.h"
#include "WindowManager.h"

#include "Renderer.h"
#include "InputManager.h"

using namespace std;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowManager::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) return true;

	InputManager::GetInstance().HandleMessage(message, wParam, lParam);

	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SIZE:
		Renderer::GetInstance().Resize(static_cast<UINT>(LOWORD(lParam)), static_cast<UINT>(HIWORD(lParam)));
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

void WindowManager::Initialize(const wchar_t* windowTitle, int width, int height, const wchar_t* className)
{
	const WNDCLASS wc =
	{
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = WindowProc,
		.hInstance = m_hInstance,
		.lpszClassName = className
	};
	if (!RegisterClass(&wc))
	{
		#ifdef _DEBUG
		cerr << "윈도우 클래스 등록 실패. 에러 코드: " << hex << GetLastError() << endl;
		#else
		MessageBoxA(nullptr, "윈도우 클래스 등록 실패.", "윈도우 관리자 오류", MB_OK | MB_ICONERROR);
		#endif
		exit(EXIT_FAILURE);
	}

	RECT windowRect = { 0, 0, width, height };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
	const int adjustedWidth = windowRect.right - windowRect.left;
	const int adjustedHeight = windowRect.bottom - windowRect.top;

	m_hWnd = CreateWindow
	(
		className,
		windowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		adjustedWidth,
		adjustedHeight,
		nullptr,
		nullptr,
		m_hInstance,
		nullptr
	);
	if (!m_hWnd)
	{
		#ifdef _DEBUG
		cerr << "윈도우 생성 실패. 에러 코드: " << hex << GetLastError() << endl;
		#else
		MessageBoxA(nullptr, "윈도우 생성 실패.", "윈도우 관리자 오류", MB_OK | MB_ICONERROR);
		#endif
		exit(EXIT_FAILURE);
	}

	// ImGui Win32 초기화
	ImGui_ImplWin32_Init(m_hWnd);
	// 렌더러 초기화
	Renderer::GetInstance().Initialize();

	ShowWindow(m_hWnd, SW_SHOW);
}

bool WindowManager::ProcessMessages()
{
	MSG msg = {};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			return false;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// ImGui Win32 새 프레임 시작
	ImGui_ImplWin32_NewFrame();

	return true;
}

void WindowManager::Finalize()
{
	// 렌더러 종료
	Renderer::GetInstance().Finalize();
	
	// 인풋매니저 종료 // 따로 필요 없음

	// ImGui Win32 종료
	ImGui_ImplWin32_Shutdown();

	// 윈도우 파괴 및 클래스 등록 해제
	DestroyWindow(m_hWnd);
	m_hWnd = nullptr;
	UnregisterClass(L"EngineWindowClass", m_hInstance);
}