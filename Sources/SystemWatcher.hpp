#pragma once

#include <functional>

#if _WIN32
struct IWbemLocator;
struct IWbemServices;
struct IWbemObjectSink;
#endif

class SystemWatcher final
{
#if _WIN32
	friend class SystemWatcherWbemSink;
#endif

public:

	~SystemWatcher();

	bool	Init();
	void	Terminate();

	bool	StartWatch(std::string_view processName, const std::function<void(uint64_t)>& onProcessCreated, const std::function<void(uint64_t)>& onProcessTerminated);
	void	StopWatch();

private:

	void	NotifyOnProcessCreated(uint64_t pid);
	void	NotifyOnProcessTerminated(uint64_t pid);

private:

	std::function<void(uint64_t)> _onProcessCreated;
	std::function<void(uint64_t)> _onProcessTerminated;

#if _WIN32
	IWbemLocator*			_locator = NULL;
	IWbemServices*			_services = NULL;

	IWbemObjectSink*		_createSink = NULL;
	IWbemObjectSink*		_terminateSink = NULL;
#endif
};
