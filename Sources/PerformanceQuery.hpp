#pragma once

#include "Process.hpp"
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <Pdh.h>
#include <PdhMsg.h>
#endif

class Query
{
public:
	Query(Process* proc);
	~Query();

	std::unordered_map<std::string, double> Retrieve();
private:
	Process* _proc;

	#ifdef _WIN32
	std::vector<std::string> _CounterName;
	PDH_HQUERY _pdhQuery;
    std::vector<PDH_HCOUNTER> _pdhCounter;
	#endif
};