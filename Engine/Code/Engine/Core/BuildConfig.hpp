#pragma once
#include <string>

typedef unsigned int uint;


class CallStack;

// #define DETECT_MEMORY_OVERRUN
#define TRACK_MEMORY_BASIC    (0)
#define TRACK_MEMORY_VERBOSE  (1)

// If not defined, we will not track memory at all
// BASIC will track bytes used, and count
// VERBOSE will track individual call stacks
#if defined(_DEBUG)
#define TRACK_MEMORY           TRACK_MEMORY_VERBOSE
#define PROFILED_BUILD
#elif defined(FINAL_BUILD)
// undefined
#else 
//#define TRACK_MEMORY           TRACK_MEMORY_BASIC
#endif


// Treat as Linked List
struct callstack_list
{
	CallStack* currentVal = nullptr;
	size_t total_allocation = 0;
	callstack_list* next = nullptr;
};

struct allocation_t
{
	size_t byte_size;
	#if defined(TRACK_MEMORY)
		#if (TRACK_MEMORY == TRACK_MEMORY_VERBOSE)
			callstack_list callstack_node;
		#endif
	#endif
};



void operator delete(void *ptr);
void* operator new(size_t const size);
uint GetHighwaterAllocation();
void ConsolePrintMemValues(const std::string& args);
void ResetFrameMemoryTracking();
void ReportVerboseCallStacks(const std::string& args);
bool SortCallStackList(const callstack_list* first, const callstack_list* second);
void InitializeVerboseReporting();
void ShutDownVerboseReport();
uint GetAllocCount();
uint GetFrameAllocs();
uint GetFrameFrees();
size_t GetAllocByteCount();