#pragma once

#include <Windows.h>
#include <string>

namespace core
{
	enum class injection 
	{ 
		loadlibrary, 
		manualmap 
	};

	enum class execution 
	{ 
		createremotethread, 
		ntcreatethreadex, 
		setwindowhookex, 
		hijackthread	
	};

	namespace options
	{
		extern injection inject;
		extern execution execute;
	}

	/*
		purpose; loads image into specified process id
		params; 
			@process_id; target image which will load the dll
			@dll_path; dll that will load
	*/
	bool inject(const DWORD& process_id, const std::string& dll_path);
}
