#pragma once
#include "Singleton.h"

class WindowManager : public Singleton<WindowManager>
{
	friend class Singleton<WindowManager>;

	HWND m_hWnd = nullptr; // 윈도우 핸들
	HINSTANCE m_hInstance = GetModuleHandle(nullptr); // 윈도우 인스턴스 핸들

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
	~WindowManager() = default;
	WindowManager(const WindowManager&) = delete;
	WindowManager& operator=(const WindowManager&) = delete;
	WindowManager(WindowManager&&) = delete;
	WindowManager& operator=(WindowManager&&) = delete;

	// 윈도우 초기화
	void Initialize(const wchar_t* windowTitle, int width = 1280, int height = 720, const wchar_t* className = L"EngineWindowClass");
	// 메시지 처리
	bool ProcessMessages();
	// 윈도우 종료
	void Finalize();

	// 윈도우 핸들 얻기
	HWND GetHWnd() const { return m_hWnd; }

private:
	WindowManager() = default;
};