#include "Pch.hpp"
#include "SystemWatcher.hpp"
#include "OS.hpp"

#include <Windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

#include <iostream>

#include <format>

class SystemWatcherWbemSink : public IWbemObjectSink
{
public:
	SystemWatcherWbemSink(const std::function<void(uint64_t)>& callback)
	: _refCount(1)
	, _callback(callback)
	{

	}

	// IUnknown Methods
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) override
	{
		if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
		{
			*ppv = static_cast<IWbemObjectSink*>(this);
			AddRef();
			return S_OK;
		}
		*ppv = nullptr;
		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef() override
	{
		return InterlockedIncrement(&_refCount);
	}

	ULONG STDMETHODCALLTYPE Release() override
	{
		ULONG count = InterlockedDecrement(&_refCount);
		if (count == 0)
		{
			delete this;
		}
		return count;
	}

	// IWbemObjectSink Methods
	HRESULT STDMETHODCALLTYPE Indicate(LONG lObjectCount, IWbemClassObject **ppObjects) override
	{
		for (LONG i = 0; i < lObjectCount; ++i)
		{
			VARIANT var;
			VariantInit(&var); // Toujours initialiser un VARIANT avant usage
			
			HRESULT hr = ppObjects[i]->Get(L"TargetInstance", 0, &var, 0, 0);
			if (SUCCEEDED(hr) && var.vt == VT_UNKNOWN)
			{
				IWbemClassObject *pObj = nullptr;
				hr = var.punkVal->QueryInterface(IID_IWbemClassObject, (void **)&pObj);
				if (SUCCEEDED(hr) && pObj)
				{
					VARIANT varName, varPid;
					VariantInit(&varName);
					VariantInit(&varPid);
					if (SUCCEEDED(pObj->Get(L"Name", 0, &varName, 0, 0)) && SUCCEEDED(pObj->Get(L"ProcessId", 0, &varPid, 0, 0)))
					{
						std::wcout << L"New process : " << varName.bstrVal << L" (PID: " << varPid.uintVal << L")" << std::endl;
						_callback(varPid.uintVal);
						VariantClear(&varName);
						VariantClear(&varPid);
					}
					pObj->Release();
				}
			}
			VariantClear(&var); // Nettoyage après utilisation
		}
		return WBEM_S_NO_ERROR;
	}

	HRESULT STDMETHODCALLTYPE SetStatus(LONG lStatus, HRESULT hResult, BSTR strParam, IWbemClassObject *pObjParam) override
	{
		// Si vous avez besoin de gérer les statuts d'erreur ou d'autres informations de statut.
		return WBEM_S_NO_ERROR;
	}

private:

	long							_refCount;
	std::function<void(uint64_t)>	_callback;
};

bool SystemWatcher::Init()
{
	HRESULT hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&_locator);
	if (FAILED(hres))
	{
		std::cerr << "SystemWatcher: Unable to create IWbemLocator" << std::endl;
		return false;
	}

	hres = _locator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &_services);
	if (FAILED(hres))
	{
		std::cerr << "SystemWatcher: Unable to connect to WMI server" << std::endl;
		_locator->Release();
		return false;
	}

	hres = CoSetProxyBlanket(_services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	if (FAILED(hres))
	{
		std::cerr << "SystemWatcher: Unable to setup WMI proxy" << std::endl;
		_services->Release();
		_locator->Release();
		return false;
	}

	return true;
}

void SystemWatcher::Terminate()
{
	if (_services != NULL)
	{
		_services->Release();
		_services = NULL;
	}

	if (_locator != NULL)
	{
		_locator->Release();
		_locator = NULL;
	}
}

bool SystemWatcher::StartWatch(std::string_view processName, const std::function<void(uint64_t)>& onProcessCreated, const std::function<void(uint64_t)>& onProcessTerminated)
{
	assert(_services);
	assert(_createSink == nullptr);
	assert(_terminateSink == nullptr);

	_onProcessCreated = onProcessCreated;
	_onProcessTerminated = onProcessTerminated;

	_createSink = new SystemWatcherWbemSink([this](uint64_t pid) { _onProcessCreated(pid); });
	std::string request = std::format("SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process' AND TargetInstance.Name = '{}.exe'", processName);
	HRESULT hres = _services->ExecNotificationQueryAsync(_bstr_t("WQL"), _bstr_t(request.c_str()), 0, NULL, _createSink);
	if (FAILED(hres))
	{
		std::cerr << "SystemWatcher: Unable to execute WMI request" << std::endl;
		return false;
	}

	_terminateSink = new SystemWatcherWbemSink([this](uint64_t pid) { _onProcessTerminated(pid); });
	request = std::format("SELECT * FROM __InstanceDeletionEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process' AND TargetInstance.Name = '{}.exe'", processName);
	hres = _services->ExecNotificationQueryAsync(_bstr_t("WQL"), _bstr_t(request.c_str()), 0, NULL, _terminateSink);
	if (FAILED(hres))
	{
		std::cerr << "SystemWatcher: Unable to execute WMI request" << std::endl;
		return false;
	}

	return true;
}

void SystemWatcher::StopWatch()
{
	_services->CancelAsyncCall(_createSink);
	_createSink->Release();
	_createSink = nullptr;

	_services->CancelAsyncCall(_terminateSink);
	_terminateSink->Release();
	_terminateSink = nullptr;
}
