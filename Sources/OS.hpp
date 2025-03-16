#pragma once

#include <string>

class OS
{
public:
	static bool Init();
	static void Terminate();

	static bool KillProcess(uint64_t pid);

#if defined(_WIN32)
	static std::string GetLastWin32ErrorMessage();
#endif
};
