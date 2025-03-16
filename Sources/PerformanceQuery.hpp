#pragma once

#include "SystemSpecs.hpp"
#include <unordered_map>
#include <utility>
#include <vector>

struct Process;

struct PerformanceSnapshot
{
	struct DataOp
	{
		double bytesPerSec = 0.0f;
		double opPerSec = 0.0f;
	};

	double   cpuUsage = 0.0f;
	int64_t  cpuDt = 0;
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

	uint64_t accCyclesTime = 0;
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
	HANDLE              procHandle;
	PerformanceSnapshot last;

	int64_t  sysDtTime;
	uint64_t sysTotalTime;
#endif
};
