#pragma once

#include <string>
#include <vector>

struct Handle;

struct Process
{
	const uint64_t       pid;
	const std::string    name;
	std::vector<Handle*> handles;
};
