#pragma once

#include <Windows.h>

namespace core
{
	class thread
	{
	public:
		thread(const DWORD& process_id);
		~thread();

		bool suspend();
		bool resume();

		bool get_context(CONTEXT& context);
		bool set_context(CONTEXT& context);
		bool get_wow64_context(WOW64_CONTEXT& context);
		bool set_wow64_context(WOW64_CONTEXT& context);
			
	private:
		DWORD m_thread_id = 0;
		HANDLE m_thread_handle = nullptr;

	};
}
