//-----------------------------------------------------------------------------------------------
// Time.cpp
//	A simple high-precision time utility function for Windows
//	based on code by Squirrel Eiserloh

//-----------------------------------------------------------------------------------------------
#include "Engine/Core/Time.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>

static InternalTimeSystem g_time;

//-----------------------------------------------------------------------------------------------
double InitializeTime( LARGE_INTEGER& out_initialTime )
{
	LARGE_INTEGER countsPerSecond;
	QueryPerformanceFrequency( &countsPerSecond );
	QueryPerformanceCounter( &out_initialTime );
	return( 1.0 / static_cast< double >( countsPerSecond.QuadPart ) );
}


//-----------------------------------------------------------------------------------------------
double GetCurrentTimeSeconds()
{
	static LARGE_INTEGER initialTime;
	static double secondsPerCount = InitializeTime( initialTime );
	LARGE_INTEGER currentCount;
	QueryPerformanceCounter( &currentCount );
	LONGLONG elapsedCountsSinceInitialTime = currentCount.QuadPart - initialTime.QuadPart;

	double currentSeconds = static_cast< double >( elapsedCountsSinceInitialTime ) * secondsPerCount;
	return currentSeconds;
}

void SleepSeconds(float secondsToSleep)
{
	int msToSleep = (int)(1000.f * secondsToSleep);
	Sleep(msToSleep);
}

//------------------------------------------------------------------------
uint64_t __fastcall TimeGetOpCount()
{
	uint64_t i;
	QueryPerformanceCounter((LARGE_INTEGER*)&i);
	return i;
}

//------------------------------------------------------------------------
uint __fastcall TimeGet_ms()
{
	uint64_t i = TimeGetOpCount() - g_time.start_ops;
	i = (i * 1000U) / g_time.ops_per_second;
	return (uint)i;
}

//------------------------------------------------------------------------
uint __fastcall TimeGet_us()
{
	uint64_t i = TimeGetOpCount() - g_time.start_ops;;
	i = (i * 1000U * 1000U) / g_time.ops_per_second;   // f is operations per second, so I want to return micro seconds, so times the top by 1000000 us / 1 s
	return (uint)i;
}

//------------------------------------------------------------------------
double __fastcall TimeGetSeconds()
{
	uint64_t op = TimeGetOpCount() - g_time.start_ops;
	return (double)op * g_time.seconds_per_op;
}

//------------------------------------------------------------------------
uint64_t TimeOpCountTo_us(uint64_t op_count)
{
	op_count *= (1000U * 1000U);
	uint64_t const us = (uint64_t)(op_count * g_time.seconds_per_op);
	return us;
}

//------------------------------------------------------------------------
double TimeOpCountTo_ms(uint64_t op_count)
{
	double seconds = op_count * g_time.seconds_per_op;
	return seconds * 1000.0;
}

//------------------------------------------------------------------------
uint64_t TimeOpCountFrom_ms(double ms)
{
	double s = ms / 1000.0;
	uint64_t const ops = (uint64_t)(s * g_time.ops_per_second);
	return ops;
}

//------------------------------------------------------------------------
std::string TimeOpCountToString(uint64_t op_count)
{
	char buffer[128];
	uint64_t us = TimeOpCountTo_us(op_count);

	if (us < 1500) {
		sprintf_s(buffer, 128, "%llu us", us);
	}
	else if (us < 1500000) {
		double ms = (double)us / (double)1000.0;
		sprintf_s(buffer, 128, "%.4f ms", ms);
	}
	else {
		double s = (double)us / (double)(1000000.0);
		sprintf_s(buffer, 128, "%.4f s", s);
	}

	std::string val = "";
	val += buffer;
	return val;
}

double TimeOpCountToSeconds(uint64_t op_count)
{
	uint64_t us = TimeOpCountTo_us(op_count);

	if (us < 1500)
		return (double)us;
	else if (us < 1500000)
		return (double)us / (double)1000.0;
	else
		return (double)us / (double)(1000000.0);
}

InternalTimeSystem::InternalTimeSystem()
{
	::QueryPerformanceFrequency((LARGE_INTEGER*)&ops_per_second);
	seconds_per_op = 1.0 / (double)ops_per_second;

	::QueryPerformanceCounter((LARGE_INTEGER*)&start_ops);
}

const float MIN_FRAMES_PER_SECOND = 10.f;
const float MAX_FRAMES_PER_SECOND = 60.f;
const float MIN_SECONDS_PER_FRAME = (1.f / MAX_FRAMES_PER_SECOND);
const float MAX_SECONDS_PER_FRAME = (1.f / MIN_FRAMES_PER_SECOND);

float CalculateDeltaSeconds()
{
	double timeNow = GetCurrentTimeSeconds();
	static double lastFrameTime = timeNow;
	double deltaSeconds = timeNow - lastFrameTime;

	// Wait until [nearly] the minimum frame time has elapsed (limit framerate to within the max)
	while (deltaSeconds < MIN_SECONDS_PER_FRAME * .999f)
	{
		timeNow = GetCurrentTimeSeconds();
		deltaSeconds = timeNow - lastFrameTime;
	}
	lastFrameTime = timeNow;

	// Clamp deltaSeconds to be within the max time allowed (e.g. sitting at a debug break point)
	if (deltaSeconds > MAX_SECONDS_PER_FRAME)
	{
		deltaSeconds = MAX_SECONDS_PER_FRAME;
	}

	return (float)deltaSeconds;
}