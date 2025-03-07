#include "Pch.hpp"
#include "Process.hpp"

Process::Process(uint64_t pid)
: _pid(pid)
{

}

uint64_t Process::GetPid() const
{
	return _pid;
}
