#pragma once

#include "Process.hpp"
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <Pdh.h>
#include <PdhMsg.h>
#endif

class Handle;

class HandleQuery
{
public:
	static void GenerateHandles(uint64_t pid, std::vector<Handle*>& handles);
};
