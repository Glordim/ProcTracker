#pragma once

#include <functional>
#include <set>
#include <string>

#if _WIN32
struct IWbemLocator;
struct IWbemServices;
struct IWbemObjectSink;
#endif

class SystemWatcher final
{
public:
	~SystemWatcher();

	bool Init();
	void Terminate();

	void ListAllProcess(std::set<std::string>& processes, const std::string& filter);

	bool StartWatch(std::string_view processName, const std::function<void(const std::string&, uint64_t)>& onProcessCreated,
	                const std::function<void(uint64_t)>& onProcessTerminated);
	void StopWatch();

private:
	std::function<void(const std::string&, uint64_t)> _onProcessCreated;
	std::function<void(uint64_t)>                     _onProcessTerminated;

#if _WIN32
	IWbemLocator*  _locator = NULL;
	IWbemServices* _services = NULL;

	IWbemObjectSink* _createSink = NULL;
	IWbemObjectSink* _terminateSink = NULL;
#endif
};
