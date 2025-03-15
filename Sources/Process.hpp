#pragma once

#include <string>
#include <vector>

class Handle;

struct Process
{
	const uint64_t       pid;
	const std::string    name;
	std::vector<Handle*> handles;
};
