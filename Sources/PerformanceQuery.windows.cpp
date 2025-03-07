#include "Pch.hpp"

#include "PerformanceQuery.hpp"

#include <format>
#include <string_view>
// #include <wformat>

namespace
{
	constexpr std::wstring_view perfCounters[] {
		L"\\Process V2(%.*hs:%u)\\Working Set - Private",
		L"\\Process V2(%.*hs:%u)\\IO Other Bytes/sec",
		L"\\Process V2(%.*hs:%u)\\IO Data Bytes/sec",
		L"\\Process V2(%.*hs:%u)\\IO Write Bytes/sec",
		L"\\Process V2(%.*hs:%u)\\IO Read Bytes/sec",
		L"\\Process V2(%.*hs:%u)\\IO Other Operations/sec",
		L"\\Process V2(%.*hs:%u)\\IO Data Operations/sec",
		L"\\Process V2(%.*hs:%u)\\IO Write Operations/sec",
		L"\\Process V2(%.*hs:%u)\\IO Read Operations/sec",
		L"\\Process V2(%.*hs:%u)\\Handle Count",
		L"\\Process V2(%.*hs:%u)\\Pool Nonpaged Bytes",
		L"\\Process V2(%.*hs:%u)\\Pool Paged Bytes",
		L"\\Process V2(%.*hs:%u)\\Creating Process ID",
		L"\\Process V2(%.*hs:%u)\\Process ID",
		L"\\Process V2(%.*hs:%u)\\Elapsed Time",
		L"\\Process V2(%.*hs:%u)\\Priority Base",
		L"\\Process V2(%.*hs:%u)\\Thread Count",
		L"\\Process V2(%.*hs:%u)\\Private Bytes",
		L"\\Process V2(%.*hs:%u)\\Page File Bytes",
		L"\\Process V2(%.*hs:%u)\\Page File Bytes Peak",
		L"\\Process V2(%.*hs:%u)\\Working Set",
		L"\\Process V2(%.*hs:%u)\\Working Set Peak",
		L"\\Process V2(%.*hs:%u)\\Page Faults/sec",
		L"\\Process V2(%.*hs:%u)\\Virtual Bytes",
		L"\\Process V2(%.*hs:%u)\\Virtual Bytes Peak",
		L"\\Process V2(%.*hs:%u)\\%% Privileged Time",
		L"\\Process V2(%.*hs:%u)\\%% User Time",
		L"\\Process V2(%.*hs:%u)\\%% Processor Time"
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
		res = PdhAddCounterW(_pdhQuery, fmt, 0, &_pdhCounter[i]);
	}
}

Query::~Query()
{
	PdhCloseQuery(_pdhQuery);
}

std::unordered_map<std::string, double> Query::Retrieve()
{
	std::unordered_map<std::string, double> out;

	PDH_STATUS res = PdhCollectQueryData(_pdhQuery);

	for (uint32_t i {0}; i < _pdhCounter.size(); ++i)
	{

		uint8_t* buffer = nullptr;
		DWORD bufferSize = 0;
		DWORD itemCount  = 0;

		PDH_FMT_COUNTERVALUE_ITEM_W *pdhItems = NULL;

		res = PdhGetFormattedCounterArrayW(
			_pdhCounter[i],
			PDH_FMT_DOUBLE,
			&bufferSize,
			&itemCount,
			pdhItems);

		if (itemCount > 0)
		{
			buffer = new uint8_t[bufferSize];
			pdhItems = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W*>(buffer);
			res = PdhGetFormattedCounterArrayW(
				_pdhCounter[i],
				PDH_FMT_DOUBLE,
				&bufferSize,
				&itemCount,
				pdhItems);
		}

		if (itemCount > 0)
		{
			uint64_t find = perfCounters[i].rfind(L"\\") + 1;
			size_t len = wcstombs(nullptr, perfCounters[i].data() + find, 0);
			std::string nameUtf8(len, '\0');
			wcstombs(nameUtf8.data(), perfCounters[i].data() + find, len);
			out.try_emplace(nameUtf8, pdhItems[0].FmtValue.doubleValue);

			delete[] buffer;
		}
	}

	return out;
}