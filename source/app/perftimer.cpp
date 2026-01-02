#include "perftimer.h"
#include "debug.h"

double PerfTimer::PCFreq = 0.0;

void PerfTimer::Init()
{
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li))
	{
		Debug::console("QueryPerformanceFrequency failed!\n");
	}

	PCFreq = double(li.QuadPart) / 1000000.0;
}

PerfTimer::PerfTimer(const char* function)
{
	functionName = function;
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	startTime = li.QuadPart;
}

double PerfTimer::Current()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - startTime) / PCFreq;
}

PerfTimer::~PerfTimer()
{
	Debug::console("PERFORMANCE: %s took %lf nanoseconds\n", functionName, Current());
}