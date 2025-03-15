#pragma once

#include "Process.hpp"
#include <unordered_map>
#include <vector>

struct Handle;

class HandleQuery
{
public:
	static void GenerateHandles(uint64_t pid, std::vector<Handle*>& handles);
};
