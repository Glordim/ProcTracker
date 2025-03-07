#include "Pch.hpp"
#include "SystemWatcher.hpp"

SystemWatcher::~SystemWatcher()
{
	Terminate();
}

void SystemWatcher::NotifyOnProcessCreated(uint64_t pid)
{
	_onProcessCreated(pid);
}

void SystemWatcher::NotifyOnProcessTerminated(uint64_t pid)
{
	_onProcessTerminated(pid);
}
