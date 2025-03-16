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
	PerformanceSnapshot  _data;
	RingBuffer<float>    _cpuUsageRingBuffer;
	RingBuffer<uint64_t> _memoryUsageRingBuffer;
	uint64_t             _maxRam = 0;
	Query                _performanceQuery;

	Process& _process;

	std::string _title;

	static SystemSpecs _specs;
};