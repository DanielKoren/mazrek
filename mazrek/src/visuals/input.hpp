#pragma once

#include <Windows.h>

namespace visuals
{
	LRESULT CALLBACK window_procedure
	(
		_In_ HWND   hwnd,
		_In_ UINT   msg,
		_In_ WPARAM wParam,
		_In_ LPARAM lParam
	);
}