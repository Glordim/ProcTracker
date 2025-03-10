#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

#include <stdint.h>

class SystemSpecs
{
public:
	SystemSpecs();

	uint32_t GetProcessorNumber() const;

private:
#ifdef _WIN32
	SYSTEM_INFO _sysInfo;
#endif
};