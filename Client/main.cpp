#include "stdafx.h"

#include "WindowManager.h"
#include "Renderer.h"

int main()
{
	WindowManager windowManager;
	windowManager.Initialize(L"Window");
	Renderer renderer;
	renderer.Initialize(windowManager.GetHWnd());

	while (windowManager.ProcessMessages())
	{
		renderer.BeginFrame({ 0.1f, 0.1f, 0.1f, 1.0f });
		renderer.EndFrame();
	};
}