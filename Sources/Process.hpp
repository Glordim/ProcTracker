#pragma once

#include <string>
#include <vector>

class Handle;

struct Process
{
	const uint64_t pid;
	std::string const name;
	std::vector<Handle*> handles;
};
