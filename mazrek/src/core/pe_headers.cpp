#include "pe_headers.hpp"

namespace core
{
	pe_headers::pe_headers(const std::vector<std::byte>& buffer)
		: m_buffer(buffer)
	{
	}

	pe_headers::~pe_headers()
	{
	}

	bool pe_headers::initialise()
	{
		m_dos_header = reinterpret_cast<IMAGE_DOS_HEADER*>(m_buffer.data());
		if (m_dos_header->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return false;
		}

		m_nt_header = reinterpret_cast<IMAGE_NT_HEADERS*>(m_buffer.data() + m_dos_header->e_lfanew);
		if (m_nt_header->Signature != IMAGE_NT_SIGNATURE)
		{
			return false;
		}

		m_file_header = &m_nt_header->FileHeader;
		m_optional_header = &m_nt_header->OptionalHeader;

		return true;
	}

	IMAGE_DOS_HEADER* pe_headers::get_dos_header() const
	{
		return m_dos_header;
	}

	IMAGE_NT_HEADERS* pe_headers::get_nt_header() const
	{
		return m_nt_header;
	}

	IMAGE_FILE_HEADER* pe_headers::get_file_header() const
	{
		return m_file_header;
	}

	IMAGE_OPTIONAL_HEADER* pe_headers::get_optional_header() const
	{
		return m_optional_header;
	}
}