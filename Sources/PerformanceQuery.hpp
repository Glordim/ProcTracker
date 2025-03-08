#pragma once

#include "Process.hpp"
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <Pdh.h>
#include <PdhMsg.h>
#endif

struct PerformanceSnapshot
{
	struct DataOp
	{
		double bytesPerSec;
		double opPerSec;
	};

	double   cpuUsage;
	double   time;
	uint32_t handleCount;
	uint32_t threadCount;

	DataOp read;
	DataOp write;
	DataOp other;

	double   pageFaultsPerSec;
	uint64_t privateBytes;
	uint64_t workingSet;
	uint64_t virtualBytes;
};

std::pair<double, const char*> AdjustSizeValue(double bytes);

class Query
{
public:
	Query(Process* proc);
	Query(Query&&);
	~Query();

	Query& operator=(Query&& other);

	void Update();

	PerformanceSnapshot Retrieve();

private:
	Process* _proc;

#ifdef _WIN32
	static DWORD FmtForCounter(uint32_t idx);
	static void  Write(uint32_t idx, PDH_FMT_COUNTERVALUE_ITEM_W const& item, PerformanceSnapshot& res);

	std::vector<std::string>  _CounterName;
	PDH_HQUERY                _pdhQuery;
	std::vector<PDH_HCOUNTER> _pdhCounter;
#endif
};