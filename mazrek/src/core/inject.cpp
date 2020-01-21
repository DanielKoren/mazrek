#include "inject.hpp"
#include "process.hpp"
#include "pe_headers.hpp"
#include "logger.hpp"
#include "..\misc\helpers.hpp" // read_binary_file

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <fstream>

namespace core
{
	namespace options
	{
		injection inject = injection::loadlibrary;
		execution execute = execution::createremotethread;
	}

	bool inject(const DWORD& process_id, const std::string& dll_path)
	{
		const auto dll_buffer = misc::read_binary_file(dll_path);
		const auto dll_size = dll_buffer.size();
		if (!dll_size || dll_size < 0x1000)
		{
			logger::log_error("invalid dll size");
			return false;
		}

		auto dll_headers = pe_headers(dll_buffer);
		if (!dll_headers.initialise())
		{
			logger::log_error("invalid dll pe headers");
			return false;
		}

		auto proc = process(process_id);
		if (!proc.attached())
		{
			logger::log_error("unable to attach process, try running as admin");
			return false;
		}

		/*validate both architecture match*/
		if (proc.is_wow64())
		{
			if (dll_headers.get_file_header()->Machine != IMAGE_FILE_MACHINE_I386) 
			{
				logger::log_error("cannot load 64-bit dll into 32-bit process");
				return false;
			}
		}
		else 
		{
			if (dll_headers.get_file_header()->Machine != IMAGE_FILE_MACHINE_AMD64) 
			{
				logger::log_error("cannot load 32-bit dll into 64-bit process");
				return false;
			}
		}
	
		uintptr_t start_address = 0x0;
		uintptr_t args_address = 0x0;

		if (options::inject == injection::loadlibrary)
		{
			const auto size_allocation = strlen(dll_path.c_str()) + 1;

			if (proc.is_wow64())
			{
				const auto addr = misc::remote_wow64_procedure(process_id, "kernel32", "LoadLibraryA");
				if (!addr)
				{
					logger::log_error("failed to obtain wow64 procedure remotely");
					return false;
				}

				start_address = addr;
			}
			else
			{
				const auto kernel_module = GetModuleHandle("kernel32");
				const auto addr = reinterpret_cast<uintptr_t>(GetProcAddress(kernel_module, "LoadLibraryA"));
				if (!addr)
				{
					logger::log_error("failed to obtain native procedure");
					return false;
				}

				start_address = addr;
			}

			args_address = proc.allocate_memory(0, size_allocation);
			proc.write_memory(args_address, dll_path.c_str(), size_allocation);
		}

		if (options::execute == execution::createremotethread)
		{
			if (!proc.create_thread(start_address, args_address))
			{
				logger::log_error("failed to create remote thread");
				return false;
			}
		}

		if (options::execute == execution::ntcreatethreadex)
		{
			if (!proc.nt_create_thread(start_address, args_address))
			{
				logger::log_error("failed to create remote thread");
				return false;
			}
		}

		if (options::execute == execution::hijackthread)
		{
			if (!proc.hijack_thread(start_address, args_address))
			{
				logger::log_error("failed to hijack remote thread");
			}
		}

		logger::log_success("injected successfully");
		return true;
	}
}