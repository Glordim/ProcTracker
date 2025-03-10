#include "Pch.hpp"

#include "PerformanceQuery.hpp"

#include <format>
#include <string_view>

// #include <wformat>

namespace
{
	constexpr std::wstring_view perfCounters[] {
		L"\\Process V2(%.*hs:%u)\\%% Processor Time",
		L"\\Process V2(%.*hs:%u)\\Elapsed Time",
		L"\\Process V2(%.*hs:%u)\\Handle Count",
		L"\\Process V2(%.*hs:%u)\\Thread Count",
		L"\\Process V2(%.*hs:%u)\\IO Read Bytes/sec",
		L"\\Process V2(%.*hs:%u)\\IO Read Operations/sec",
		L"\\Process V2(%.*hs:%u)\\IO Write Bytes/sec",
		L"\\Process V2(%.*hs:%u)\\IO Write Operations/sec",
		L"\\Process V2(%.*hs:%u)\\IO Other Bytes/sec",
		L"\\Process V2(%.*hs:%u)\\IO Other Operations/sec",
		L"\\Process V2(%.*hs:%u)\\Page Faults/sec",
		L"\\Process V2(%.*hs:%u)\\Private Bytes",
		L"\\Process V2(%.*hs:%u)\\Working Set",
		L"\\Process V2(%.*hs:%u)\\Virtual Bytes"
		// L"\\Process V2(%.*hs:%u)\\IO Data Bytes/sec",
	    // L"\\Process V2(%.*hs:%u)\\IO Data Operations/sec",
	    // L"\\Process V2(%.*hs:%u)\\Pool Nonpaged Bytes",
	    // L"\\Process V2(%.*hs:%u)\\Pool Paged Bytes",
	    // L"\\Process V2(%.*hs:%u)\\Creating Process ID",
	    // L"\\Process V2(%.*hs:%u)\\Process ID",
	    // L"\\Process V2(%.*hs:%u)\\Priority Base",
	    // L"\\Process V2(%.*hs:%u)\\Page File Bytes",
	    // L"\\Process V2(%.*hs:%u)\\Page File Bytes Peak",
	    // L"\\Process V2(%.*hs:%u)\\Working Set Peak",
	    // L"\\Process V2(%.*hs:%u)\\Virtual Bytes Peak",
	    // L"\\Process V2(%.*hs:%u)\\%% Privileged Time",
	    // L"\\Process V2(%.*hs:%u)\\Working Set - Private",
	    // L"\\Process V2(%.*hs:%u)\\%% Processor Time",
	};
}

Query::Query(Process* proc)
: _proc {proc}
{
	uint32_t perfCountersSize = sizeof(perfCounters) / sizeof(std::wstring_view);
	_pdhCounter.resize(perfCountersSize);

	PDH_STATUS res = PdhOpenQueryW(nullptr, 0, &_pdhQuery);

	for (uint32_t i {0}; i < perfCountersSize; ++i)
	{
		static wchar_t fmt[4096] {'\0'};
		swprintf(fmt, 4096, perfCounters[i].data(), proc->name.size() - 4, proc->name.data(), proc->pid);
		wprintf(L"%s\n", fmt);
		res = PdhAddEnglishCounterW(_pdhQuery, fmt, 0, &_pdhCounter[i]);
	}
}

Query::Query(Query&& other)
{
	PdhCloseQuery(_pdhQuery);

	_proc = other._proc;
	_pdhQuery = other._pdhQuery;
	_pdhCounter = other._pdhCounter;

	other._pdhQuery = nullptr;
}

Query::~Query()
{
	if (_pdhQuery)
	{
		PdhCloseQuery(_pdhQuery);
	}
}

Query& Query::operator=(Query&& other)
{
	PdhCloseQuery(_pdhQuery);

	_proc = other._proc;
	_pdhQuery = other._pdhQuery;
	_pdhCounter = other._pdhCounter;

	other._pdhQuery = nullptr;

	return *this;
}

void Query::Update() {}

PerformanceSnapshot Query::Retrieve(const SystemSpecs& specs)
{
	PerformanceSnapshot out {};

	PDH_STATUS res = PdhCollectQueryData(_pdhQuery);

	for (uint32_t i {0}; i < _pdhCounter.size(); ++i)
	{
		uint8_t* buffer = nullptr;
		DWORD    bufferSize = 0;
		DWORD    itemCount = 0;

		PDH_FMT_COUNTERVALUE_ITEM_W* pdhItems = NULL;
		DWORD                        fmt = FmtForCounter(i);

		res = PdhGetFormattedCounterArrayW(_pdhCounter[i], fmt, &bufferSize, &itemCount, pdhItems);

		if (itemCount > 0)
		{
			buffer = new uint8_t[bufferSize];
			pdhItems = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W*>(buffer);
			res = PdhGetFormattedCounterArrayW(_pdhCounter[i], fmt, &bufferSize, &itemCount, pdhItems);
		}
		else
		{
			printf("No data for counter %u\n", i);
		}

		if (itemCount > 0)
		{
			Write(i, pdhItems[0], out);
		}
	}

	out.cpuUsage /= specs.GetProcessorNumber();

	return out;
}

DWORD Query::FmtForCounter(uint32_t idx)
{
	switch (idx)
	{
		case 0: // cpu usage
			return PDH_FMT_DOUBLE | PDH_FMT_NOCAP100;

		case 1:  // elapsed time
		case 4:  // read  bytes
		case 5:  //  *    op
		case 6:  // write bytes
		case 7:  //  *    op
		case 8:  // other bytes
		case 9:  //  *    op
		case 10: // page faults
			return PDH_FMT_DOUBLE;

		case 2: // handle count
		case 3: // thread count
			return PDH_FMT_LONG;

		case 11: // private bytes
		case 12: // working set
		case 13: // virtual bytes
			return PDH_FMT_LARGE;

		default: return PDH_FMT_DOUBLE;
	}
}

void Query::Write(uint32_t idx, PDH_FMT_COUNTERVALUE_ITEM_W const& item, PerformanceSnapshot& res)
{
	switch (idx)
	{
		case 0:
		{
			res.cpuUsage = item.FmtValue.doubleValue;
			break;
		}

		case 1:
		{
			res.time = item.FmtValue.doubleValue;
			break;
		}

		case 2:
		{
			res.handleCount = item.FmtValue.longValue;
			break;
		}

		case 3:
		{
			res.threadCount = item.FmtValue.longValue;
			break;
		}

		case 4:
		{
			res.read.bytesPerSec = item.FmtValue.doubleValue;
			break;
		}

		case 5:
		{
			res.read.opPerSec = item.FmtValue.doubleValue;
			break;
		}

		case 6:
		{
			res.write.bytesPerSec = item.FmtValue.doubleValue;
			break;
		}

		case 7:
		{
			res.write.opPerSec = item.FmtValue.doubleValue;
			break;
		}

		case 8:
		{
			res.other.bytesPerSec = item.FmtValue.doubleValue;
			break;
		}

		case 9:
		{
			res.other.opPerSec = item.FmtValue.doubleValue;
			break;
		}

		case 10:
		{
			res.pageFaultsPerSec = item.FmtValue.doubleValue;
			break;
		}

		case 11:
		{
			res.privateBytes = item.FmtValue.largeValue;
			break;
		}

		case 12:
		{
			res.workingSet = item.FmtValue.largeValue;
			break;
		}

		case 13:
		{
			res.virtualBytes = item.FmtValue.largeValue;
			break;
		}

		default: break;
	}
}