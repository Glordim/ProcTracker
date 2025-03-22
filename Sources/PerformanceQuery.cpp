#include "Pch.hpp"

#include "PerformanceQuery.hpp"

std::pair<double, const char*> AdjustSizeValue(uint64_t bytes)
{
	uint32_t unitScale {0};
	uint64_t bytesVal = bytes;
	while (bytesVal > 1024)
	{
		bytesVal /= 1024.0;
		++unitScale;
	}

	const char* unit;
	switch (unitScale)
	{
		case 0: unit = "B"; break;
		case 1: unit = "KB"; break;
		case 2: unit = "MB"; break;
		case 3: unit = "GB"; break;
		case 4: unit = "TB"; break;

		default: break;
	}

	return {bytes / pow(1024.0, unitScale), unit};
}

Time AdjustTimeValue(double sec)
{
	Time res;
	res.d = sec / (60 * 60 * 24);
	sec -= res.d * (60 * 60 * 24);
	res.h = sec / (60 * 60);
	sec -= res.h * (60 * 60);
	res.m = sec / 60;
	res.s = fmod(sec, 60);

	return res;
}