#include "stdafx.h"

#include "WindowManager.h"
#include "Renderer.h"

#include "TestScene.h"

int main()
{
	WindowManager windowManager;
	windowManager.Initialize(L"Window");

	Renderer::GetInstance().Initialize(windowManager.GetHWnd());

	TestScene testScene;
	testScene.Initialize();

	while (windowManager.ProcessMessages())
	{
		testScene.Update(0.015f);
		testScene.TransformGameObjects();
		testScene.Render();
	};
}