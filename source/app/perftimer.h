#ifndef PERFTIMER_H
#define PERFTIMER_H

#include <windows.h>
#include <SDL/SDL.h>

class PerfTimer
{
public:
	static void Init();

	PerfTimer(const char* function);
	~PerfTimer();

	double Current();

private:
	static double PCFreq;
	Sint64 startTime;
	const char* functionName;
};

#endif