#pragma once

#include "PerformanceQuery.hpp"
#include "RingBuffer.hpp"
#include "SystemSpecs.hpp"

#include <string>

struct Process;

class ProcessDrawer
{
public:
	ProcessDrawer(Process& process);

	void Update();
	bool Draw();

private:
	PerformanceSnapshot _data;

	RingBuffer<float> _cpuUsageRingBuffer;

	RingBuffer<uint64_t> _memoryUsageRingBuffer;
	uint64_t             _memoryMax = 0;

	RingBuffer<uint64_t> _ioReadUsageRingBuffer;
	RingBuffer<uint64_t> _ioWriteUsageRingBuffer;
	uint64_t             _ioMax = 0;

	RingBuffer<uint64_t> _netDownUsageRingBuffer;
	RingBuffer<uint64_t> _netUpUsageRingBuffer;
	uint64_t             _netMax = 0;

	Query _performanceQuery;

	Process& _process;

	std::string _title;

	static SystemSpecs _specs;
};