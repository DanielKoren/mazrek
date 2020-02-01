#include "inject.hpp"
#include "logger.hpp"
#include "process.hpp"
#include "thread.hpp"
#include "pe_headers.hpp"
#include "shellcode.hpp"
#include "..\misc\helpers.hpp"

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

		/*validate both arch*/
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
			//initalise start_address
			if (proc.is_wow64())
			{
				start_address = misc::remote_wow64_procedure(process_id, "kernel32", "LoadLibraryA");
				if (!start_address)
				{
					logger::log_error("failed to obtain wow64 procedure remotely");
					return false;
				}
			}
			else
			{
				start_address = reinterpret_cast<uintptr_t>(GetProcAddress(GetModuleHandle("kernel32"), "LoadLibraryA"));
				if (!start_address)
				{
					logger::log_error("failed to obtain native procedure");
					return false;
				}
			}
			//initalise args_address
			args_address = proc.allocate_memory(0, size_allocation);
			if (!args_address)
			{
				logger::log_error("failed to allocate memory");
				return false;
			}
			if (!proc.write_memory(args_address, dll_path.c_str(), size_allocation))
			{
				logger::log_error("failed to write memory");
				return false;
			}
		}

		//remote thread creation
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
			auto proc_thread = thread(process_id);
			if (!proc_thread.suspend())
			{
				logger::log_error("failed to suspend thread");
				return false;
			}

			const auto shellcode_address = proc.allocate_memory(0, 0x100);
			if (!shellcode_address)
			{
				proc_thread.resume();
				logger::log_error("failed to allocate memory");
				return false;
			}

			//86-bit process
			if (proc.is_wow64())
			{
				WOW64_CONTEXT context{};
				context.ContextFlags = CONTEXT_FULL;

				if (!proc_thread.get_wow64_context(context))
				{
					proc_thread.resume();
					logger::log_error("failed to get thread context");
					return false;
				}

				context.Esp -= 4; //alloc space on stack for ret address
				if (!proc.write_memory(context.Esp, &context.Eip, 4)) //write current eip into esp
				{
					proc_thread.resume();
					logger::log_error("failed to write memory");
					return false;
				}
				
				*reinterpret_cast<uint32_t*>(shellcode_x86 + 3) = start_address;
				*reinterpret_cast<uint32_t*>(shellcode_x86 + 8) = args_address;

				context.Eip = shellcode_address; //point eip to our shellcode

				if (!proc.write_memory(shellcode_address, shellcode_x86, sizeof(shellcode_x86)))
				{
					proc_thread.resume();
					logger::log_error("failed to write memory");
					return false;
				}

				if (!proc_thread.set_wow64_context(context))
				{
					proc_thread.resume();
					logger::log_error("failed to set thread context");
					return false;
				}
			}
			//64-bit process
			else
			{
				CONTEXT context{};
				context.ContextFlags = CONTEXT_FULL;

				if (!proc_thread.get_context(context))
				{
					proc_thread.resume();
					logger::log_error("failed to get thread context");
					return false;
				}

				context.Rsp -= 8; //alloc space on stack for ret address
				if (!proc.write_memory(context.Rsp, &context.Rip, 8)) //write current rip into esp
				{
					proc_thread.resume();
					logger::log_error("failed to write memory");
					return false;
				}

				*reinterpret_cast<uintptr_t*>(shellcode_x64 + 14) = start_address;
				*reinterpret_cast<uintptr_t*>(shellcode_x64 + 24) = args_address;

				context.Rip = shellcode_address;

				if (!proc.write_memory(shellcode_address, shellcode_x64, sizeof(shellcode_x64)))
				{
					proc_thread.resume();
					logger::log_error("failed to write memory");
					return false;
				}

				if (!proc_thread.set_context(context))
				{
					proc_thread.resume();
					logger::log_error("failed to set thread context");
					return false;
				}
			}

			proc_thread.resume();
		}

		logger::log_success("injected successfully");
		return true;
	}
}