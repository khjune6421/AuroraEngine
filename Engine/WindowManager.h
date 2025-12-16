#pragma once

class WindowManager
{
	HWND m_hWnd = nullptr; // 윈도우 핸들
	HINSTANCE m_hInstance = GetModuleHandle(nullptr); // 윈도우 인스턴스 핸들

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
	// 윈도우 초기화
	void Initialize(const wchar_t* windowTitle, int width = 1280, int height = 720, const wchar_t* className = L"EngineWindowClass");
	// 메시지 처리
	bool ProcessMessages();
	// 윈도우 종료
	void Finalize();
};