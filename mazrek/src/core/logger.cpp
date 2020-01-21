#include "logger.hpp"

namespace core
{
	namespace logger
	{
		std::vector<std::pair<std::string, log_type>> logs;

		void log_info(const std::string& message)
		{
			logs.emplace_back(std::make_pair(message, log_type::information_type));
		}

		void log_error(const std::string& message)
		{
			logs.emplace_back(std::make_pair(message, log_type::error_type));
		}

		void log_success(const std::string& message)
		{
			logs.emplace_back(std::make_pair(message, log_type::success_type));
		}
	}
}
