#pragma once

#include <string>

#include <Windows.h>

struct Handle
{
	enum class Type : uint8_t
	{
		Unknown = 0,
		AlpcPort,
		Desktop,
		Directory,
		DxgkSharedResource,
		Event,
		File,
		IoCompletion,
		IoCompletionReserve,
		IrTimer,
		Key,
		Mutant,
		SchedulerSharedData,
		Section,
		Semaphore,
		Thread,
		Timer,
		TpWorkerFactory,
		WaitCompletionPacket,
		WindowStation,

		Count
	};

	using Id = HANDLE;

	Handle(Type type, Id id, const std::string& info = "")
	: type(type)
	, id(id)
	, info(info)
	{

	}

	const Type			type;
	const Id			id;
	const std::string	info;
};
