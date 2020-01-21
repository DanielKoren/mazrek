#include "helpers.hpp"
#include <fstream>
#include <TlHelp32.h>
#include <Psapi.h>

namespace misc
{
	bool file_exist(const std::string& path)
	{
		std::ifstream stream(path);
		return stream.good();
	}

	std::string open_filename(const HWND& hwnd)
	{
		char path[MAX_PATH]{};

		OPENFILENAME ofn{};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hwnd;
		ofn.lpstrFilter = "Dll Files\0*.dll\0*.*\0";
		ofn.lpstrFile = path;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = "Select the DLL";
		ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

		if (!GetOpenFileNameA(&ofn))
			return std::string();

		return std::string(path);
	}

	std::string strip_path(const std::string& directory)
	{
		std::string result(directory);

		const auto last_slash_idx = result.find_last_of("\\/");
		if (std::string::npos != last_slash_idx)
		{
			result.erase(0, last_slash_idx + 1);
		}

		return result;
	}

	std::vector<std::pair<std::string, DWORD>> enumerate_processes()
	{
		std::vector<std::pair<std::string, DWORD>> result;

		auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		auto pe32 = PROCESSENTRY32{};
		pe32.dwSize = sizeof(PROCESSENTRY32);

		if (Process32First(snapshot, &pe32))
		{
			do
			{
				const auto process_name = std::string(pe32.szExeFile);
				const auto process_id = pe32.th32ProcessID;
				result.emplace_back(std::make_pair(process_name, process_id));
			} while (Process32Next(snapshot, &pe32));
		}

		if (snapshot)
		{
			CloseHandle(snapshot);
		}

		return result;
	}

	std::vector<std::byte> read_binary_file(const std::string& binary_path)
	{
		std::vector<std::byte> result;
		std::ifstream file(binary_path, std::ios::ate | std::ios::binary);
		if (!file.good()) return result;

		auto file_size = file.tellg();
		if (file_size < 0x1000) return result;

		file.seekg(0, std::ios::end);
		result.resize(file_size);
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(result.data()), file_size);

		file.close();

		return result;
	}

	//compare two strs regardless the letter case
	const char* stristr(const char* haystack, const char* needle)
	{
		do {
			const char* h = haystack;
			const char* n = needle;
			while (tolower((unsigned char)*h) == tolower((unsigned char)*n) && *n) {
				h++;
				n++;
			}
			if (*n == 0) {
				return (const char*)haystack;
			}
		} while (*haystack++);
		return 0;
	}

	DWORD remote_wow64_procedure(const DWORD& process_id, const char* module_name, const char* procedure_name)
	{
		auto snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);
		if (!snapshot_handle)
			return 0;

		DWORD module_address = 0x0;
		auto me32 = MODULEENTRY32{};
		me32.dwSize = sizeof(MODULEENTRY32);

		if (Module32First(snapshot_handle, &me32))
		{
			do
			{
				if (stristr(me32.szModule, module_name))
				{
					module_address = reinterpret_cast<DWORD>(me32.modBaseAddr);
					break;
				}

			} while (Module32Next(snapshot_handle, &me32));
		}

		if (snapshot_handle) CloseHandle(snapshot_handle);
		if (!module_address) return 0;

		char sys_wow64_path[MAX_PATH]{};
		GetSystemWow64Directory(sys_wow64_path, MAX_PATH);

		std::string kernel_path(sys_wow64_path);
		kernel_path += "\\";
		kernel_path += module_name;
		kernel_path += ".dll";

		auto kernel_module = LoadLibraryEx(kernel_path.c_str(), nullptr, LOAD_LIBRARY_AS_IMAGE_RESOURCE);
		if (!kernel_module) return 0;

		auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>((DWORD_PTR)kernel_module & ~0xFFFF); //xxxx0000
		if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) return 0;

		auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS32>((DWORD_PTR)dos_header + dos_header->e_lfanew);
		auto export_dir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>((DWORD_PTR)dos_header + nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
		auto fun_table = reinterpret_cast<PDWORD>((DWORD_PTR)dos_header + (DWORD)(export_dir->AddressOfFunctions));
		auto name_table = reinterpret_cast<PDWORD>((DWORD_PTR)dos_header + (DWORD)(export_dir->AddressOfNames));
		auto ord_table = reinterpret_cast<PWORD>((DWORD_PTR)dos_header + (DWORD)(export_dir->AddressOfNameOrdinals));

		DWORD procedure_rva = 0;

		for (int i = 0; i < export_dir->NumberOfNames; i++)
		{
			const auto name = reinterpret_cast<const char*>((DWORD_PTR)dos_header + name_table[i]);
			procedure_rva = fun_table[ord_table[i]];
			if (!strcmp(name, procedure_name))
			{
				break;
			}
		}

		FreeLibrary(kernel_module);

		return module_address + procedure_rva;
	}

}