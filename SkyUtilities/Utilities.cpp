#include "Utilities.h"

Utilities::Timer::Timer() 
{
	startTime = -1000000;
	secondsPassed = 0;
}

Utilities::Timer::~Timer(){}

void Utilities::Timer::StartTimer()
{
	startTime = std::clock(); //Start timer
}

bool Utilities::Timer::HasMillisecondsPassed(double secondsToDelay)
{
	secondsPassed = (std::clock() - startTime) / (CLOCKS_PER_SEC / 1000);

	if (secondsPassed >= secondsToDelay)
		return true;
	return false;
}