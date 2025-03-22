#include "Pch.hpp"

#include "PerformanceQuery.hpp"
#include "Process.hpp"

#include <format>
#include <string_view>

#include <phnt_windows.h>

#include <phnt.h>

void*    Query::processesData {nullptr};
uint64_t Query::processesDataSize {0x4000};

Query::Query(const Process& proc)
: pid {proc.pid}
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
	bool                first = !sysTotalTime;

	uint64_t                                  totalTime {0};
	SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* cpuPerfs = new SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION[specs.GetProcessorNumber()];
	NtQuerySystemInformation(SystemProcessorPerformanceInformation, cpuPerfs, sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * specs.GetProcessorNumber(), nullptr);
	for (uint32_t i {0}; i < specs.GetProcessorNumber(); ++i)
	{
		cpuPerfs[i].KernelTime.QuadPart -= cpuPerfs[i].IdleTime.QuadPart;

		totalTime += cpuPerfs[i].KernelTime.QuadPart + cpuPerfs[i].UserTime.QuadPart + cpuPerfs[i].IdleTime.QuadPart;
	}

	double sysDtTime = totalTime - sysTotalTime;
	sysTotalTime = totalTime;

	double            cpuDt = 0.0;
	KERNEL_USER_TIMES times;
	NTSTATUS          status = NtQueryInformationProcess(procHandle, ProcessTimes, &times, sizeof(KERNEL_USER_TIMES), nullptr);
	if (NT_SUCCESS(status))
	{
		snap.cpuUsage = (times.UserTime.QuadPart + times.KernelTime.QuadPart);
		cpuDt = snap.cpuUsage - last.cpuUsage;
	}

	IO_COUNTERS io;
	status = NtQueryInformationProcess(procHandle, ProcessIoCounters, &io, sizeof(IO_COUNTERS), nullptr);
	if (NT_SUCCESS(status))
	{
		snap.read.bytes = io.ReadTransferCount;
		snap.read.op = io.ReadOperationCount;
		snap.write.bytes = io.WriteTransferCount;
		snap.write.op = io.WriteOperationCount;
		snap.other.bytes = io.OtherTransferCount;
		snap.other.op = io.OtherOperationCount;
	}

	VM_COUNTERS mem;
	status = NtQueryInformationProcess(procHandle, ProcessVmCounters, &mem, sizeof(VM_COUNTERS), nullptr);
	if (NT_SUCCESS(status))
	{
		snap.workingSet = mem.WorkingSetSize;
		snap.virtualBytes = mem.VirtualSize;
		snap.privateBytes = mem.PagefileUsage;
	}

	PROCESS_NETWORK_COUNTERS net;
	status = NtQueryInformationProcess(procHandle, ProcessNetworkIoCounters, &net, sizeof(PROCESS_NETWORK_COUNTERS), nullptr);
	if (NT_SUCCESS(status))
	{
		snap.netDown = net.BytesIn;
		snap.netUp = net.BytesOut;
	}

	LARGE_INTEGER current;
	NtQuerySystemTime(&current);

	SYSTEM_PROCESS_INFORMATION* info = (SYSTEM_PROCESS_INFORMATION*)Query::FindProcessInfo(pid);
	if (info)
	{
		snap.threadCount = info->NumberOfThreads;
		SYSTEM_PROCESS_INFORMATION_EXTENSION* ext =
			(SYSTEM_PROCESS_INFORMATION_EXTENSION*)(((uint8_t*)info) + offsetof(SYSTEM_PROCESS_INFORMATION, Threads) + (sizeof(SYSTEM_THREAD_INFORMATION) * info->NumberOfThreads));
		snap.read.bytes = ext->DiskCounters.BytesRead;
		snap.read.op = ext->DiskCounters.ReadOperationCount;
		snap.write.bytes = ext->DiskCounters.BytesWritten;
		snap.write.op = ext->DiskCounters.WriteOperationCount;
	}

	PerformanceSnapshot out = snap;
	if (first)
	{
		last = snap;
	}

	out.cpuUsage = 100.0 * cpuDt / (double)sysDtTime;
	out.time = (current.QuadPart - times.CreateTime.QuadPart) / 10000000;
	out.read.bytes = snap.read.bytes - last.read.bytes;
	out.read.op = snap.read.op - last.read.op;
	out.write.bytes = snap.write.bytes - last.write.bytes;
	out.write.op = snap.write.op - last.write.op;
	out.other.bytes = snap.other.bytes - last.other.bytes;
	out.other.op = snap.other.op - last.other.op;
	out.netDown = snap.netDown - last.netDown;
	out.netUp = snap.netUp - last.netUp;

	last = snap;

	return out;
}

void Query::UpdateStatic()
{
	if (!processesData)
	{
		processesData = malloc(processesDataSize);
	}

	unsigned long newSize {0};
	NTSTATUS      status;
	while (true)
	{
		status = NtQuerySystemInformation(SystemProcessInformation, processesData, processesDataSize, &newSize);

		if (status == STATUS_BUFFER_TOO_SMALL || status == STATUS_INFO_LENGTH_MISMATCH)
		{
			processesData = realloc(processesData, newSize);
			processesDataSize = newSize;
		}
		else
		{
			break;
		}
	}
}

void* Query::FindProcessInfo(uint64_t pid)
{
	SYSTEM_PROCESS_INFORMATION* data = (SYSTEM_PROCESS_INFORMATION*)processesData;
	uint8_t*                    dataBytes = (uint8_t*)processesData;

	while (data->UniqueProcessId != (HANDLE)pid)
	{
		if (data->NextEntryOffset)
		{
			dataBytes += data->NextEntryOffset;
			data = (SYSTEM_PROCESS_INFORMATION*)dataBytes;
		}
		else
		{
			return nullptr;
		}
	}

	return data;
}