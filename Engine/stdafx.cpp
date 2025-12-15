#include "stdafx.h"

using namespace std;

void CheckResult(HRESULT hr, const char* msg)
{
	if (FAILED(hr))
	{
		#ifdef _DEBUG
		cerr << msg << " 에러 코드: " << hex << hr << endl;
		#else
		MessageBoxA(nullptr, msg, "오류", MB_OK | MB_ICONERROR);
		#endif
		exit(EXIT_FAILURE);
	}
}