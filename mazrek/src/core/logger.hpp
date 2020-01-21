#pragma once

#include <windows.h>
#include <vector>
#include <string>

namespace core
{
	namespace logger
	{
		enum class log_type
		{
			information_type,
			error_type,
			success_type
		};

		extern std::vector<std::pair<std::string, log_type>> logs;

		void log_info(const std::string& message);
		void log_error(const std::string& message);
		void log_success(const std::string& message);
	}
}
