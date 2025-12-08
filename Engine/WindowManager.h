#pragma once

class WindowManager
{
	HWND m_hWnd = nullptr; // 윈도우 핸들
	HINSTANCE m_hInstance = GetModuleHandle(nullptr); // 윈도우 인스턴스 핸들

public:
	void Initialize(const wchar_t* windowTitle, int width = 1280, int height = 720, const wchar_t* className = L"EngineWindowClass");
	bool ProcessMessages();

	HWND GetHWnd() const { return m_hWnd; }
};