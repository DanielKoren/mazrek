#pragma once

#include <Windows.h>
#include <vector>

namespace core
{
	class pe_headers
	{
	public:
		pe_headers(const std::vector<std::byte>& buffer);
		~pe_headers();

		bool initialise();

		IMAGE_DOS_HEADER* get_dos_header() const;
		IMAGE_NT_HEADERS* get_nt_header() const;
		IMAGE_FILE_HEADER* get_file_header() const;
		IMAGE_OPTIONAL_HEADER* get_optional_header() const;

	private:
		std::vector<std::byte> m_buffer{};
		IMAGE_DOS_HEADER* m_dos_header = nullptr;
		IMAGE_NT_HEADERS* m_nt_header = nullptr;
		IMAGE_FILE_HEADER* m_file_header = nullptr;
		IMAGE_OPTIONAL_HEADER* m_optional_header = nullptr;

	};
}
