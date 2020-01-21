#pragma once

#include <Windows.h>
#include <string>

namespace visuals
{
	class window
	{
	public:
		window(HINSTANCE& instance);
		~window();

		bool initialise();
		bool is_running();
		bool message_loop();

		inline DWORD get_width() { return m_width; }
		inline DWORD get_height() { return m_height; }
		inline HWND get_wnd_handle() { return m_wnd_handle; }

	private:
		char		m_class_name[MAX_PATH]{};
		HINSTANCE	m_instance = nullptr;
		DWORD		m_width = 310;
		DWORD		m_height = 300;
		HICON		m_icon_handle = nullptr;
		HWND		m_wnd_handle = nullptr;
		MSG			m_msg{};

	};
}