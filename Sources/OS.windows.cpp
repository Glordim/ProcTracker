#include "Pch.hpp"
#include "OS.hpp"

#define WIN32_LEAN_AND_MEAN
#include <comdef.h>
#include <DbgHelp.h>
#include <Windows.h>

#include <iostream>

bool OS::Init()
{
	HRESULT hres;

	// Required by SystemWatcher
	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres))
	{
		std::cerr << "OS: Unable to initialize COM" << std::endl;
		return false;
	}

	hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	if (FAILED(hres))
	{
		std::cerr << "OS: Unable to initialize COM security" << std::endl;
		return false;
	}
	//

	return true;
}

void OS::Terminate()
{
	CoUninitialize();
}

std::string OS::GetLastWin32ErrorMessage()
{
	LPVOID lpMsgBuf;
	DWORD  dw = GetLastError();

	::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	                (LPTSTR)&lpMsgBuf, 0, NULL);

	std::string message = (char*)lpMsgBuf;

	::LocalFree(lpMsgBuf);

	return message;
}
