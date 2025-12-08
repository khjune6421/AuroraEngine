#include "stdafx.h"
#include "WindowManager.h"

using namespace std;

void WindowManager::Initialize(const wchar_t* windowTitle, int width, int height, const wchar_t* className)
{
	const WNDCLASS wc = 
	{
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = DefWindowProc, // TODO: 나중에 커스텀 윈도우 프로시저로 변경
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

	m_hWnd = CreateWindow
	(
		className,
		windowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
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

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);
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

	return true;
}
