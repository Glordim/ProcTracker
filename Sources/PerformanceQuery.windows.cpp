#include "Pch.hpp"

#include "PerformanceQuery.hpp"
#include "Process.hpp"

#include <format>
#include <string_view>

#include <phnt_windows.h>

#include <phnt.h>

Query::Query(const Process& proc)
{
	CLIENT_ID id;
	id.UniqueProcess = (HANDLE)proc.pid;
	id.UniqueThread = nullptr;
	OBJECT_ATTRIBUTES attr;
	InitializeObjectAttributes(&attr, nullptr, 0, nullptr, nullptr);
	NTSTATUS status = NtOpenProcess(&procHandle, PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, &attr, &id);
	if (!NT_SUCCESS(status))
	{
		printf("Failed to open process\n");
	}
}

Query::Query(Query&& other)
{
	NtClose(procHandle);

	other.procHandle = nullptr;
}

Query::~Query()
{
	if (procHandle)
	{
		NtClose(procHandle);
	}
}

Query& Query::operator=(Query&& other)
{
	NtClose(procHandle);

	other.procHandle = nullptr;

	return *this;
}

void Query::Update() {}

PerformanceSnapshot Query::Retrieve(const SystemSpecs& specs)
{
	PerformanceSnapshot snap {};

	uint64_t                                  totalTime {0};
	SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* cpuPerfs = new SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION[specs.GetProcessorNumber()];
	NtQuerySystemInformation(SystemProcessorPerformanceInformation, cpuPerfs, sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * specs.GetProcessorNumber(), nullptr);
	for (uint32_t i {0}; i < specs.GetProcessorNumber(); ++i)
	{
		cpuPerfs[i].KernelTime.QuadPart -= cpuPerfs[i].IdleTime.QuadPart;

		totalTime += cpuPerfs[i].KernelTime.QuadPart + cpuPerfs[i].UserTime.QuadPart + cpuPerfs[i].IdleTime.QuadPart;
	}

	sysDtTime = totalTime - sysTotalTime;
	sysTotalTime = totalTime;

	PROCESS_CYCLE_TIME_INFORMATION cycles;

	NTSTATUS status = NtQueryInformationProcess(procHandle, ProcessCycleTime, &cycles, sizeof(PROCESS_CYCLE_TIME_INFORMATION), nullptr);
	if (NT_SUCCESS(status))
	{
		snap.accCyclesTime = cycles.AccumulatedCycles;
	}

	KERNEL_USER_TIMES times;
	status = NtQueryInformationProcess(procHandle, ProcessTimes, &times, sizeof(KERNEL_USER_TIMES), nullptr);
	if (NT_SUCCESS(status))
	{
		snap.cpuUsage = (times.UserTime.QuadPart + times.KernelTime.QuadPart);
		snap.cpuDt = snap.cpuUsage - last.cpuUsage;
	}

	IO_COUNTERS io;
	status = NtQueryInformationProcess(procHandle, ProcessIoCounters, &io, sizeof(IO_COUNTERS), nullptr);
	if (NT_SUCCESS(status))
	{
		snap.read.bytesPerSec = io.ReadTransferCount;
		snap.read.opPerSec = io.ReadOperationCount;
		snap.write.bytesPerSec = io.WriteTransferCount;
		snap.write.opPerSec = io.WriteOperationCount;
		snap.other.bytesPerSec = io.OtherTransferCount;
		snap.other.opPerSec = io.OtherOperationCount;
	}

	VM_COUNTERS mem;
	status = NtQueryInformationProcess(procHandle, ProcessVmCounters, &mem, sizeof(VM_COUNTERS), nullptr);
	if (NT_SUCCESS(status))
	{
		snap.workingSet = mem.WorkingSetSize;
		snap.virtualBytes = mem.VirtualSize;
		snap.privateBytes = mem.PagefileUsage;
	}

	LARGE_INTEGER current;
	NtQuerySystemTime(&current);

	PerformanceSnapshot out = snap;
	out.cpuUsage = 100.0 * snap.cpuDt / (double)sysDtTime;
	out.time = (current.QuadPart - times.CreateTime.QuadPart) / 10000000;
	out.read.bytesPerSec = snap.read.bytesPerSec - last.read.bytesPerSec;
	out.read.opPerSec = snap.read.opPerSec - last.read.opPerSec;
	out.write.bytesPerSec = snap.write.bytesPerSec - last.write.bytesPerSec;
	out.write.opPerSec = snap.write.opPerSec - last.write.opPerSec;
	out.other.bytesPerSec = snap.other.bytesPerSec - last.other.bytesPerSec;
	out.other.opPerSec = snap.other.opPerSec - last.other.opPerSec;

	last = snap;

	return out;
}