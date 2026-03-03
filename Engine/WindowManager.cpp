#include "stdafx.h"
#include "WindowManager.h"

#include "Renderer.h"
#include "InputManager.h"
#include "ResourceManager.h"

#include "..\\Client\\resource.h"

using namespace std;

#ifdef _DEBUG
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

LRESULT CALLBACK WindowManager::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	#ifdef _DEBUG
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) return true;
	#endif

	InputManager::GetInstance().HandleMessage(message, wParam, lParam);

	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);

		return 0;

	case WM_SIZE:
		Renderer::GetInstance().Resize(static_cast<UINT>(LOWORD(lParam)), static_cast<UINT>(HIWORD(lParam)));

		#ifdef NDEBUG
		if (GetForegroundWindow() == hWnd)
		{
			RECT clipRect;
			GetClientRect(hWnd, &clipRect);
			MapWindowPoints(hWnd, nullptr, reinterpret_cast<POINT*>(&clipRect), 2);
			ClipCursor(&clipRect);
		}
		#endif

		return 0;

	case WM_ACTIVATE:
		#ifdef NDEBUG
		if (LOWORD(wParam) != WA_INACTIVE)
		{
			RECT clipRect;
			GetClientRect(hWnd, &clipRect);
			MapWindowPoints(hWnd, nullptr, reinterpret_cast<POINT*>(&clipRect), 2);
			ClipCursor(&clipRect);
		}
		else ClipCursor(nullptr);
		#endif


		return 0;

	case WM_SYSKEYDOWN:
		if (wParam == VK_RETURN)
		{
			WindowManager::GetInstance().ToggleFullscreen();
			return 0;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

void WindowManager::Initialize(const wchar_t* windowTitle, int width, int height, const wchar_t* className)
{
	// 한글 출력 설정
	SetConsoleOutputCP(CP_UTF8);

	const WNDCLASS wc =
	{
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = WindowProc,
		.hInstance = m_hInstance,
		.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_ICON1)),
		.lpszClassName = className
	};
	if (!RegisterClass(&wc))
	{
		LOG_ERROR("윈도우 클래스 등록 실패. 에러 코드: " << hex << GetLastError());
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
		LOG_ERROR("윈도우 생성 실패. 에러 코드: " << hex << GetLastError());
		exit(EXIT_FAILURE);
	}

	#ifdef _DEBUG
	// ImGui Win32 초기화
	ImGui_ImplWin32_Init(m_hWnd);
	#endif

	// 렌더러 초기화
	Renderer::GetInstance().Initialize(m_hWnd);

	// 인풋매니저 초기화
	InputManager::GetInstance().Initialize(m_hWnd);

	ShowWindow(m_hWnd, SW_SHOW);

	ResourceManager& rm = ResourceManager::GetInstance();
	rm.LoadLUTTexture();
	rm.LoadNoiseTexture();

	#ifdef NDEBUG
	rm.CacheAllModel();
	#endif
}

bool WindowManager::ProcessMessages()
{
	InputManager::GetInstance().ClearInput();

	MSG msg = {};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT) return false;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	#ifdef _DEBUG
	// ImGui Win32 새 프레임 시작
	ImGui_ImplWin32_NewFrame();
	#endif

	return true;
}

void WindowManager::Finalize()
{
	// 렌더러 종료
	Renderer::GetInstance().Finalize();
	
	#ifdef _DEBUG
	// ImGui Win32 종료
	ImGui_ImplWin32_Shutdown();
	#endif

	// 윈도우 파괴 및 클래스 등록 해제
	DestroyWindow(m_hWnd);
	m_hWnd = nullptr;
	UnregisterClass(L"EngineWindowClass", m_hInstance);
}

RECT WindowManager::GetClientPosRect() const
{
	RECT rect;

	GetClientRect(m_hWnd, &rect);
	ClientToScreen(m_hWnd, reinterpret_cast<POINT*>(&rect.left));

	return rect;
}

void WindowManager::ToggleFullscreen()
{
	if (!m_hWnd) return;

	bool targetFullscreen = !m_isFullscreen;
	Renderer& renderer = Renderer::GetInstance();
	renderer.SetFullscreen(targetFullscreen);

	if (targetFullscreen)
	{
		/*GetWindowRect(m_hWnd, &m_windowedRect);

		LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
		style &= ~WS_OVERLAPPEDWINDOW;
		SetWindowLong(m_hWnd, GWL_STYLE, style);*/

		/*HMONITOR hMon = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hMon, &mi);*/

// 		SetWindowPos
// 		(
// 			m_hWnd, HWND_TOP,
// 			mi.rcMonitor.left, mi.rcMonitor.top,
// 			mi.rcMonitor.right - mi.rcMonitor.left,
// 			mi.rcMonitor.bottom - mi.rcMonitor.top,
// 			SWP_NOOWNERZORDER | SWP_FRAMECHANGED
// 		);
		//ShowWindow(m_hWnd, SW_MAXIMIZE);
		
		m_isFullscreen = true;
	}
	else
	{
		/*LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
		style |= WS_OVERLAPPEDWINDOW;
		SetWindowLong(m_hWnd, GWL_STYLE, style);

		SetWindowPos
		(
			m_hWnd, nullptr,
			m_windowedRect.left, m_windowedRect.top,
			m_windowedRect.right - m_windowedRect.left,
			m_windowedRect.bottom - m_windowedRect.top,
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED
		);*/

		//ShowWindow(m_hWnd, SW_NORMAL);
		m_isFullscreen = false;
	}
}