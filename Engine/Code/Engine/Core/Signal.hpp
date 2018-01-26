#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

typedef unsigned int uint;

class Signal
{
public:
	Signal();
	~Signal();

	// will signal all threads waiting on this object.
	void signal_all();

	// Suspstends calling thread until object is signaled.
	void wait();

	// Suspends calling thread until object is signaled, or 'ms' MS have
	// passed.  Returns true if object was signaled, and fales if it timed out.
	bool wait_for(uint ms);

public:
	HANDLE os_event;
};

void SignalTest();