#pragma once

#include "SystemSpecs.hpp"
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <Pdh.h>
#include <PdhMsg.h>
#endif

struct Process;

struct PerformanceSnapshot
{
	struct DataOp
	{
		double bytesPerSec = 0.0f;
		double opPerSec = 0.0f;
	};

	double   cpuUsage = 0.0f;
	double   time = 0.0f;
	uint32_t handleCount = 0;
	uint32_t threadCount = 0;

	DataOp read;
	DataOp write;
	DataOp other;

	double   pageFaultsPerSec = 0.0f;
	uint64_t privateBytes = 0;
	uint64_t workingSet = 0;
	uint64_t virtualBytes = 0;
};

std::pair<double, const char*> AdjustSizeValue(double bytes);

struct Time
{
	uint32_t d = 0;
	uint32_t h = 0;
	uint32_t m = 0;
	uint32_t s = 0;
};

Time AdjustTimeValue(double sec);

class Query
{
public:
	Query(const Process& proc);
	Query(Query&&);
	~Query();

	Query& operator=(Query&& other);

	void Update();

	PerformanceSnapshot Retrieve(const SystemSpecs& specs);

#ifdef _WIN32
	static DWORD FmtForCounter(uint32_t idx);
	static void  Write(uint32_t idx, PDH_FMT_COUNTERVALUE_ITEM_W const& item, PerformanceSnapshot& res);

	std::vector<std::string>  _CounterName;
	PDH_HQUERY                _pdhQuery;
	std::vector<PDH_HCOUNTER> _pdhCounter;
#endif
};
