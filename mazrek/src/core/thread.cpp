#include "thread.hpp"
#include "logger.hpp"
#include <TlHelp32.h>

inline bool check_handle(HANDLE handle)
{
	return handle && handle != INVALID_HANDLE_VALUE;
}

namespace core
{
	thread::thread(const DWORD& process_id)
	{
		auto snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, process_id);

		auto te32 = THREADENTRY32{};
		te32.dwSize = sizeof(THREADENTRY32);

		if (Thread32First(snapshot_handle, &te32))
		{
			do
			{
				if (te32.th32OwnerProcessID == process_id)
				{
					m_thread_id = te32.th32ThreadID;
					break;
				}
			} while (Thread32Next(snapshot_handle, &te32));
		}

		if (m_thread_id)
		{
			ACCESS_MASK desired_acess = THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME;
			m_thread_handle = OpenThread(desired_acess, FALSE, m_thread_id);
		}

		if (check_handle)
		{
			CloseHandle(snapshot_handle);
		}
	}

	thread::~thread()
	{
		if (check_handle(m_thread_handle))
		{
			CloseHandle(m_thread_handle);
		}
	}

	bool thread::suspend()
	{
		return SuspendThread(m_thread_handle) != (DWORD)-1 ? true : false;
	}

	bool thread::resume()
	{
		return ResumeThread(m_thread_handle) != (DWORD)-1 ? true : false;
	}

	bool thread::get_context(CONTEXT& context)
	{
		return GetThreadContext(m_thread_handle, &context);
	}

	bool thread::set_context(CONTEXT& context)
	{
		return SetThreadContext(m_thread_handle, &context);
	}

	bool thread::get_wow64_context(WOW64_CONTEXT& context)
	{
		return Wow64GetThreadContext(m_thread_handle, &context);
	}

	bool thread::set_wow64_context(WOW64_CONTEXT& context)
	{
		return Wow64SetThreadContext(m_thread_handle, &context);
	}
}