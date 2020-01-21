#pragma once

#include <windows.h>
#include <vector>
#include <string>

namespace misc
{
	bool file_exist(const std::string& path);	
	std::string open_filename(const HWND& hwnd);
	std::string strip_path(const std::string& directory);
	std::vector<std::pair<std::string, DWORD>> enumerate_processes();
	std::vector<std::byte> read_binary_file(const std::string& binary_path);
	const char* stristr(const char* haystack, const char* needle);
	DWORD remote_wow64_procedure(const DWORD& process_id, const char* module_name, const char* procedure_name);
}
