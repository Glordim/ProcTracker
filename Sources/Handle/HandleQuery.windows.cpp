#include "Pch.hpp"
#include "Handle.hpp"
#include "HandleQuery.hpp"

#include <Ntstatus.h>
#include <Windows.h>
#include <winternl.h>

#include <functional>
#include <unordered_map>

#pragma comment(lib, "ntdll.lib")

// TODO may be add https://github.com/winsiderss/phnt in deps ?
struct SYSTEM_HANDLE
{
	USHORT      UniqueProcessId;
	USHORT      CreatorBackTraceIndex;
	UCHAR       ObjectTypeNumber;
	UCHAR       Flags;
	USHORT      Handle;
	PVOID       Object;
	ACCESS_MASK GrantedAccess;
};

static constexpr SYSTEM_INFORMATION_CLASS SystemHandleInformation = (SYSTEM_INFORMATION_CLASS)16;

struct SYSTEM_HANDLE_INFORMATION
{
	ULONG         HandleCount;
	SYSTEM_HANDLE Handles[1];
};

static constexpr OBJECT_INFORMATION_CLASS ObjectNameInformation = (OBJECT_INFORMATION_CLASS)1;

struct OBJECT_NAME_INFORMATION
{
	UNICODE_STRING Name;
};

//

std::wstring GetHandleName(HANDLE handle)
{
	BYTE  nameBuffer[1024];
	ULONG returnLength;

	NTSTATUS status = NtQueryObject(handle, (OBJECT_INFORMATION_CLASS)ObjectNameInformation, nameBuffer, sizeof(nameBuffer), &returnLength);
	if (status == 0)
	{
		OBJECT_NAME_INFORMATION* nameInfo = (OBJECT_NAME_INFORMATION*)nameBuffer;
		return (const wchar_t*)nameInfo->Name.Buffer;
	}

	return std::wstring();
}

void ConvertNtPathToWin32Path(std::wstring& ntPath)
{
	static std::unordered_map<std::wstring, std::wstring> ntDeviceToWin32Mapping;
	if (ntDeviceToWin32Mapping.empty())
	{
		wchar_t ntDevice[MAX_PATH] = {0};
		wchar_t drive[3] = L"A:";
		while (drive[0] <= L"Z"[0])
		{
			if (QueryDosDeviceW(drive, ntDevice, MAX_PATH) != 0)
			{
				ntDeviceToWin32Mapping.emplace(ntDevice, drive);
			}

			drive[0]++;
		}
	}

	if (ntPath.starts_with(L"\\Device\\HarddiskVolume"))
	{
		size_t       nextSeparator = ntPath.find('\\', 22);
		std::wstring ntDevice = ntPath.substr(0, nextSeparator);
		auto         it = ntDeviceToWin32Mapping.find(ntDevice);
		if (it != ntDeviceToWin32Mapping.end())
		{
			ntPath.replace(0, nextSeparator, it->second);
		}
	}
}

std::string WStringToString(const std::wstring& wstr)
{
	std::string str;
	size_t      size;
	str.resize(wstr.length());
	wcstombs_s(&size, &str[0], str.size() + 1, wstr.c_str(), wstr.size());
	return str;
}

Handle* CreateAlpcPortHandle(HANDLE handle)
{
	return new Handle(Handle::Type::AlpcPort, handle);
}

Handle* CreateDesktopHandle(HANDLE handle)
{
	return new Handle(Handle::Type::Desktop, handle);
}

Handle* CreateDirectoryHandle(HANDLE handle)
{
	return new Handle(Handle::Type::Directory, handle);
}

Handle* CreateDxgkSharedResourceHandle(HANDLE handle)
{
	return new Handle(Handle::Type::DxgkSharedResource, handle);
}

Handle* CreateEventHandle(HANDLE handle)
{
	return new Handle(Handle::Type::Event, handle);
}

Handle* CreateFileHandle(HANDLE handle)
{
	std::wstring path = GetHandleName(handle);
	ConvertNtPathToWin32Path(path);

	return new Handle(Handle::Type::File, handle, WStringToString(path));
}

Handle* CreateIoCompletionHandle(HANDLE handle)
{
	return new Handle(Handle::Type::IoCompletion, handle);
}

Handle* CreateIoCompletionReserveHandle(HANDLE handle)
{
	return new Handle(Handle::Type::IoCompletionReserve, handle);
}

Handle* CreateIrTimerHandle(HANDLE handle)
{
	return new Handle(Handle::Type::IrTimer, handle);
}

Handle* CreateKeyHandle(HANDLE handle)
{
	std::wstring path = GetHandleName(handle);
	return new Handle(Handle::Type::Key, handle, WStringToString(path));
}

Handle* CreateMutantHandle(HANDLE handle)
{
	return new Handle(Handle::Type::Mutant, handle);
}

Handle* CreateSchedulerSharedDataHandle(HANDLE handle)
{
	return new Handle(Handle::Type::SchedulerSharedData, handle);
}

Handle* CreateSectionHandle(HANDLE handle)
{
	return new Handle(Handle::Type::Section, handle);
}

Handle* CreateSemaphoreHandle(HANDLE handle)
{
	return new Handle(Handle::Type::Semaphore, handle);
}

Handle* CreateThreadHandle(HANDLE handle)
{
	return new Handle(Handle::Type::Thread, handle);
}

Handle* CreateTimerHandle(HANDLE handle)
{
	return new Handle(Handle::Type::Timer, handle);
}

Handle* CreateTpWorkerFactoryHandle(HANDLE handle)
{
	return new Handle(Handle::Type::TpWorkerFactory, handle);
}

Handle* CreateWaitCompletionPacketHandle(HANDLE handle)
{
	return new Handle(Handle::Type::WaitCompletionPacket, handle);
}

Handle* CreateWindowStationHandle(HANDLE handle)
{
	return new Handle(Handle::Type::WindowStation, handle);
}

void HandleQuery::GenerateHandles(uint64_t pid, std::vector<Handle*>& handles)
{
	static std::unordered_map<std::wstring, std::function<Handle*(HANDLE)>> handleFactory = {
		{L"AlpcPort", &CreateAlpcPortHandle},
		{L"Desktop", &CreateDesktopHandle},
		{L"Directory", &CreateDirectoryHandle},
		{L"DxgkSharedResource", &CreateDxgkSharedResourceHandle},
		{L"Event", &CreateEventHandle},
		{L"File", &CreateFileHandle},
		{L"IoCompletion", &CreateIoCompletionHandle},
		{L"IoCompletionReserve", &CreateIoCompletionReserveHandle},
		{L"IrTimer", &CreateIrTimerHandle},
		{L"Key", &CreateKeyHandle},
		{L"Mutant", &CreateMutantHandle},
		{L"SchedulerSharedData", &CreateSchedulerSharedDataHandle},
		{L"Section", &CreateSectionHandle},
		{L"Semaphore", &CreateSemaphoreHandle},
		{L"Thread", &CreateThreadHandle},
		{L"Timer", &CreateTimerHandle},
		{L"TpWorkerFactory", &CreateTpWorkerFactoryHandle},
		{L"WaitCompletionPacket", &CreateWaitCompletionPacketHandle},
		{L"WindowStation", &CreateWindowStationHandle},
	};

	ULONG    bufferSize = 1024 * 8;
	PVOID    buffer = malloc(bufferSize);
	NTSTATUS status;
	while ((status = NtQuerySystemInformation(SystemHandleInformation, buffer, bufferSize, &bufferSize)) == STATUS_INFO_LENGTH_MISMATCH)
	{
		buffer = realloc(buffer, bufferSize);
	}

	if (status != 0)
	{
		fprintf(stderr, "NtQuerySystemInformation failed (status: 0x%08X)\n", status);
		free(buffer);
		return;
	}

	SYSTEM_HANDLE_INFORMATION* handleInfo = (SYSTEM_HANDLE_INFORMATION*)buffer;
	for (ULONG i = 0; i < handleInfo->HandleCount; ++i)
	{
		const SYSTEM_HANDLE& ntHandle = handleInfo->Handles[i];
		if (ntHandle.UniqueProcessId == pid)
		{
			BYTE  buffer[1024];
			ULONG returnLength;

			NTSTATUS status = NtQueryObject((HANDLE)ntHandle.Handle, ObjectTypeInformation, buffer, sizeof(buffer), &returnLength);
			if (status == 0)
			{
				PUBLIC_OBJECT_TYPE_INFORMATION* pInfo = (PUBLIC_OBJECT_TYPE_INFORMATION*)buffer;

				auto it = handleFactory.find(pInfo->TypeName.Buffer);
				if (it != handleFactory.end())
				{
					Handle* handle = it->second((HANDLE)ntHandle.Handle);
					if (handle != nullptr)
					{
						handles.push_back(handle);
					}
				}
				else
				{
					// todo error ?
				}
			}
		}
	}

	free(buffer);
}
