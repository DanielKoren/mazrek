#include "window.hpp"
#include "input.hpp"
#include <random>

namespace visuals
{
	window::window(HINSTANCE& instance)
		: m_instance(instance)
	{
		//todo; create seperate utils/misc cpp files for this kind of functionality
		static const char possible_chars[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";

		std::random_device rand;
		std::mt19937 rand_engine;
		std::uniform_int_distribution<> dist(0, strlen(possible_chars) - 1);

		for (int i = 0; i < 0x10; i++)
		{
			const auto rnd_index = dist(rand_engine);
			m_class_name[i] = possible_chars[rnd_index];
		}

		m_icon_handle = LoadIcon(LoadLibrary("SHELL32"), MAKEINTRESOURCE(5));
	}

	window::~window()
	{
		UnregisterClass(m_class_name, m_instance);
	}

	bool window::initialise()
	{
		auto wndclass = WNDCLASSEX{};
		ZeroMemory(&wndclass, sizeof(WNDCLASSEX));
		wndclass.cbSize = sizeof(WNDCLASSEX);
		wndclass.style = CS_CLASSDC;
		wndclass.lpfnWndProc = window_procedure;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = m_instance;
		wndclass.hIcon = m_icon_handle;
		wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wndclass.hbrBackground = NULL;
		wndclass.lpszMenuName = nullptr;
		wndclass.lpszClassName = m_class_name;
		wndclass.hIconSm = NULL;

		if (!RegisterClassEx(&wndclass))
		{
			MessageBox(nullptr, "Failed to register a window class", "error", MB_ICONERROR | MB_OK);
			return false;
		}

		auto centerX = (GetSystemMetrics(SM_CXSCREEN) / 2) - (m_width / 2);
		auto centerY = (GetSystemMetrics(SM_CYSCREEN) / 2) - (m_height / 2);

		m_wnd_handle = CreateWindow(m_class_name, m_class_name,
			WS_POPUP | WS_VISIBLE | WS_SYSMENU,
			centerX, centerY, m_width, m_height,
			nullptr, nullptr, m_instance, nullptr);
		if (!m_wnd_handle)
		{
			MessageBox(nullptr, "Failed to create window", "error", MB_ICONERROR | MB_OK);
			return false;
		}

		ShowWindow(m_wnd_handle, SW_SHOW);
		UpdateWindow(m_wnd_handle);

		return true;
	}

	bool window::is_running()
	{
		if (m_msg.message == WM_QUIT)
		{
			return false;
		}

		return true;
	}

	bool window::message_loop()
	{
		if (PeekMessage(&m_msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&m_msg);
			DispatchMessage(&m_msg);
			return true;
		}

		return false;
	}
}
