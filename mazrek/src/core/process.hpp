#pragma once

#include <Windows.h>
#include <stdint.h>

namespace core
{
	class process
	{
	public:
		process(const DWORD& process_id);
		~process();

		bool attached();	
		bool is_wow64();

		bool read_memory(const uintptr_t address, const void* buffer, const size_t size);
		bool write_memory(const uintptr_t address, const void* buffer, const size_t size);
		uintptr_t allocate_memory(const uintptr_t address, const size_t size);
		bool free_memory(const uintptr_t address);

		template <typename T>
		T read_memory(const uintptr_t address)
		{
			T buffer{};
			read_memory(address, &buffer, sizeof(T));
			return buffer;
		}

		template <typename T>
		bool write_memory(const uintptr_t address, T buffer)
		{
			return write_memory(address, &buffer, sizeof(T));
		}

		//remote thread execution
		bool create_thread(const uintptr_t address, const uintptr_t args);
		bool nt_create_thread(const uintptr_t address, const uintptr_t args);

	private:
		DWORD m_process_id = 0;
		HANDLE m_process_handle = nullptr;

	};
}
