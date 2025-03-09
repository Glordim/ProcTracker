#include "Pch.hpp"

#include "SystemSpecs.hpp"

SystemSpecs::SystemSpecs()
{
	GetNativeSystemInfo(&_sysInfo);
}

uint32_t SystemSpecs::GetProcessorNumber() const
{
	return _sysInfo.dwNumberOfProcessors;
}