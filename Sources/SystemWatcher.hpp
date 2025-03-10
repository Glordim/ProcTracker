#pragma once

#include <functional>
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

	bool	Init();
	void	Terminate();

	bool	StartWatch(std::string_view processName, const std::function<void(std::string const&, uint64_t)>& onProcessCreated, const std::function<void(uint64_t)>& onProcessTerminated);
	void	StopWatch();

private:

	std::function<void(std::string const&, uint64_t)> _onProcessCreated;
	std::function<void(uint64_t)> _onProcessTerminated;

#if _WIN32
	IWbemLocator*			_locator = NULL;
	IWbemServices*			_services = NULL;

	IWbemObjectSink*		_createSink = NULL;
	IWbemObjectSink*		_terminateSink = NULL;
#endif
};
