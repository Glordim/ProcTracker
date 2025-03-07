#pragma once

#include <string>

class OS
{
public:
	static bool Init();
	static void Terminate();

#if defined(_WIN32)
	static std::string	GetLastWin32ErrorMessage();
#endif
};
