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
		uint64_t bytes = 0.0f;
		uint64_t op = 0.0f;
	};

	double   cpuUsage = 0.0f;
	double   time = 0.0f;
	uint32_t threadCount = 0;

	DataOp read;
	DataOp write;
	DataOp other;

	uint64_t netDown;
	double   netUp;

	uint64_t privateBytes = 0;
	uint64_t workingSet = 0;
	uint64_t virtualBytes = 0;
};

std::pair<double, const char*> AdjustSizeValue(uint64_t bytes);

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

	static void UpdateStatic();

	static void* FindProcessInfo(uint64_t pid);

#ifdef _WIN32
	HANDLE              procHandle;
	uint64_t            pid;
	PerformanceSnapshot last;

	uint64_t sysTotalTime {0};

	static void*    processesData;
	static uint64_t processesDataSize;
#endif
};
