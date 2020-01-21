#include "process.hpp"
#include <Windows.h>
#include <string>
#include <TlHelp32.h>
#include <Psapi.h>

inline bool check_handle(HANDLE handle)
{
	return handle && handle != INVALID_HANDLE_VALUE;
}

namespace core
{
	process::process(const DWORD& process_id) :
		m_process_id(process_id)
	{
		ACCESS_MASK desired_access = PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION |
			PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD;

		m_process_handle = OpenProcess(desired_access, FALSE, m_process_id);
	}

	process::~process()
	{
		if (check_handle(m_process_handle))
		{
			CloseHandle(m_process_handle);
		}
	}

	bool process::attached()
	{
		return check_handle(m_process_handle) ? true : false;
	}

	bool process::is_wow64()
	{
		BOOL is_process_wow64 = FALSE;
		IsWow64Process(m_process_handle, &is_process_wow64);
		return is_process_wow64;
	}

	bool process::read_memory(const uintptr_t address, const void* buffer, const size_t size)
	{
		SIZE_T bytes_read = 0;
		return ReadProcessMemory(m_process_handle, reinterpret_cast<LPCVOID>(address), const_cast<LPVOID>(buffer), size, &bytes_read);
	}

	bool process::write_memory(const uintptr_t address, const void* buffer, const size_t size)
	{
		SIZE_T bytes_read = 0;
		return WriteProcessMemory(m_process_handle, reinterpret_cast<LPVOID>(address), buffer, size, &bytes_read);
	}

	uintptr_t process::allocate_memory(const uintptr_t address, const size_t size)
	{
		return reinterpret_cast<uintptr_t>(VirtualAllocEx(m_process_handle, reinterpret_cast<LPVOID>(address), size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
	}

	bool process::free_memory(const uintptr_t address)
	{
		return VirtualFreeEx(m_process_handle, reinterpret_cast<LPVOID>(address), 0, MEM_RELEASE);
	}

	bool process::create_thread(const uintptr_t address, const uintptr_t args)
	{
		HANDLE thread_handle = nullptr;

		thread_handle = CreateRemoteThread(m_process_handle,
			nullptr,
			NULL,
			reinterpret_cast<LPTHREAD_START_ROUTINE>(address),
			reinterpret_cast<LPVOID>(args),
			NULL,
			nullptr);

		/*WaitForSingleObject(thread_handle, INFINITE);

		DWORD exit_code = 0;
		GetExitCodeThread(thread_handle, &exit_code);

		if (check_handle(thread_handle))
		{
			CloseHandle(thread_handle);
		}*/

		if (!check_handle(thread_handle))
		{
			return false;
		}

		CloseHandle(thread_handle);
		return true;
	}
	
	bool process::nt_create_thread(const uintptr_t address, const uintptr_t args)
	{
#define NT_SUCCESS(x) ((x) >= 0)
		typedef LONG(__stdcall* NtCreateThreadEx_t)(
			PHANDLE ThreadHandle, ACCESS_MASK DesiredAccess, LPVOID ObjectAttributes, HANDLE ProcessHandle, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, BOOL CreateSuspended, DWORD dwStackSize, LPVOID Unknown1, LPVOID Unknown2, LPVOID Unknown3
			);
		HANDLE thread_handle = nullptr;

		const auto NtCreateThreadEx = reinterpret_cast<NtCreateThreadEx_t>(GetProcAddress(GetModuleHandle("ntdll"), "NtCreateThreadEx"));;
		if (!NtCreateThreadEx)
			return false;

		auto status = NtCreateThreadEx(&thread_handle,
			THREAD_ALL_ACCESS,
			nullptr,
			m_process_handle,
			reinterpret_cast<LPTHREAD_START_ROUTINE>(address),
			reinterpret_cast<LPVOID>(args),
			FALSE,
			0,
			nullptr,
			nullptr,
			nullptr);

		if (!NT_SUCCESS(status) ||
			!check_handle(thread_handle))
		{
			return false;
		}

		CloseHandle(thread_handle);
		return true;
	}

	bool process::hijack_thread(const uintptr_t address, const uintptr_t args)
	{

		return true;
	}
}