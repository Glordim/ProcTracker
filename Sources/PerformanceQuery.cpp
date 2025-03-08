#include "Pch.hpp"

#include "PerformanceQuery.hpp"

std::pair<double, const char*> AdjustSizeValue(double bytes)
{
	uint32_t unitScale {0};
	while (bytes > 1024)
	{
		bytes /= 1024.0;
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

	return {bytes, unit};
}