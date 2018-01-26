#include "Engine/Core/Interval.hpp"
#include "Engine/Core/Time.hpp"

void Interval::SetSeconds(float seconds)
{
	interval_time = seconds;
	target_time = (float)GetCurrentTimeSeconds() + seconds;
}

void Interval::SetFrequency(float hz) 
{ 
	SetSeconds(1.0f / hz);
}

bool Interval::Check()
{
	float current_time = (float)GetCurrentTimeSeconds();
	return (current_time >= target_time);
}

bool Interval::CheckAndDecrement()
{
	if (Check()) {
		target_time += interval_time;
		return true;
	}
	else {
		return false;
	}
}

bool Interval::CheckAndReset()
{
	if (Check()) {
		Reset();
		return true;
	}
	else {
		return false;
	}
}

uint Interval::DecrementAll()
{
	uint count = 0;
	while (CheckAndDecrement()) {
		++count;
	}

	return count;
}

void Interval::Reset()
{
	target_time = (float)GetCurrentTimeSeconds() + interval_time;
}
