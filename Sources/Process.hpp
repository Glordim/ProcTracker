#pragma once

class Process
{
public:

	Process(uint64_t pid);

	uint64_t	GetPid() const;

private:

	const uint64_t _pid;
};
