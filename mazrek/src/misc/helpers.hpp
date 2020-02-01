#pragma once

#include <windows.h>
#include <vector>
#include <string>

namespace misc
{
	const bool enable_debug_priv();
	const bool file_exist(const std::string& path);	
	const std::string open_filename(const HWND& hwnd);
	const std::string strip_path(const std::string& directory);
	const std::vector<std::byte> read_binary_file(const std::string& binary_path);
	std::vector<std::pair<std::string, DWORD>> enumerate_processes();
	const char* stristr(const char* haystack, const char* needle);
	const DWORD remote_wow64_procedure(const DWORD& process_id, const char* module_name, const char* procedure_name);
}
