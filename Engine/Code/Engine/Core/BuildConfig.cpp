#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Core/CommandSystem.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Core/CallStack.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Logging.hpp"
#include "Engine/Core/CriticalSection.hpp"
#include <sstream>
#include <vector>
#include <algorithm>


static uint g_allocCount = 0;
static uint g_frameAllocs = 0;
static uint g_frameFrees = 0;
static size_t g_allocatedByteCount = 0;
static uint g_highWater = 0;
static callstack_list* g_callstackHead = nullptr;
static bool g_wasReportRun = false;
static CriticalSection g_memoryLocker;



uint GetHighwaterAllocation()
{
	if (g_frameAllocs > g_highWater)
		g_highWater = g_frameAllocs;

	return g_highWater;
}

#if defined (TRACK_MEMORY)
void* operator new(size_t const size)
{
	SCOPE_LOCK(&g_memoryLocker);
	++g_allocCount;
	++g_frameAllocs;
	g_allocatedByteCount += size;

	size_t alloc_size = size + sizeof(allocation_t);
	allocation_t *ptr = (allocation_t*)malloc(alloc_size);
	ptr->byte_size = size;

	//Verbose
	#if (TRACK_MEMORY == TRACK_MEMORY_VERBOSE)
		ptr->callstack_node.currentVal = CreateCallstack(0);
		ptr->callstack_node.total_allocation = size;
		ptr->callstack_node.next = nullptr;

		bool run = true;
		callstack_list* currentNode = nullptr;
		while (g_callstackHead != nullptr && run)
		{
			if (currentNode == nullptr)
			{
				currentNode = g_callstackHead;
			}

			if (currentNode->next != nullptr)
			{
				currentNode = currentNode->next;
			}
			else
			{
				currentNode->next = &ptr->callstack_node;
				run = false;
			}

		}

		if (g_callstackHead == nullptr)
		{
			g_callstackHead = &ptr->callstack_node;
		}
	#endif

	return ptr + 1;
}
#endif

#if defined (TRACK_MEMORY)
void operator delete(void *ptr)
{
	if (nullptr == ptr)
		return;

	SCOPE_LOCK(&g_memoryLocker);

	--g_allocCount;
	++g_frameFrees;

	allocation_t *size_ptr = (allocation_t*)ptr;
	size_ptr--;

	g_allocatedByteCount -= size_ptr->byte_size;

	#if (TRACK_MEMORY == TRACK_MEMORY_VERBOSE)
		if(size_ptr->callstack_node.currentVal != nullptr)
			DestroyCallstack(size_ptr->callstack_node.currentVal);
		
		//Set pointer to current stack null
		bool run = true;
		callstack_list* currentNode = nullptr;
		while (g_callstackHead != nullptr && run && &size_ptr->callstack_node != NULL)
		{
			if (currentNode == nullptr && g_callstackHead != &size_ptr->callstack_node)
			{
				currentNode = g_callstackHead;
			}
			else
			{
				g_callstackHead = nullptr;
				run = false;
				continue;
			}

			if (currentNode->next != nullptr && currentNode->next != &size_ptr->callstack_node)
			{
				currentNode = currentNode->next;
			}
			else
			{
				currentNode->next = nullptr;
				run = false;
			}
		}
	#endif

	free(size_ptr);
}
#endif

void ResetFrameMemoryTracking()
{
	g_frameAllocs = 0;
	g_frameFrees = 0;
}

void ReportVerboseCallStacks(const std::string& args)
{
	SCOPE_LOCK(&g_memoryLocker);
	g_wasReportRun = true;

	std::stringstream stream = std::stringstream(args);
	std::string list[2];
	std::string segment;

	int index = 0;
	while (std::getline(stream, segment, '-'))
	{
		if (index > 1)
			index = 1;

		list[index] = segment;
		++index;
	}

	double startTime = 0.0;
	double endTime = GetCurrentTimeSeconds();

	if (list[0] != "" && list[1] != "")
	{
		startTime = std::stod(list[0]);
		endTime = std::stod(list[1]);
	}

	std::vector<callstack_list*> report_callstack;
	report_callstack.reserve(g_allocCount + 1);

	callstack_list* temp = g_callstackHead;
	while (temp != nullptr)
	{
		report_callstack.push_back(temp);
		temp = temp->next;
	}

	std::sort(report_callstack.begin(), report_callstack.end(), SortCallStackList);

	callstack_list* currentNode = nullptr;

	uint totalSimiliarAllocs = 0;
	size_t totalSimiliarSize = 0;

	float reportedTotalBytes = (float)g_allocatedByteCount;
	std::string sizeTotal = "B";

	if (g_allocatedByteCount > (2 * 1024) && g_allocatedByteCount < (2 * 1024 * 1024))
	{
		//KB
		reportedTotalBytes /= 1024.0f;
		sizeTotal = "KiB";
	}
	else if ((unsigned long)g_allocatedByteCount > (2UL * 1024UL * 1024UL) && (unsigned long)g_allocatedByteCount < (2UL * 1024UL * 1024UL * 1024UL))
	{
		//MB
		reportedTotalBytes /= (1024.0f * 1024.0f);
		sizeTotal = "MiB";
	}
	else if ((unsigned long)g_allocatedByteCount > (2UL * 1024UL * 1024UL * 1024UL))
	{
		//GB
		reportedTotalBytes /= (1024.0f * 1024.0f * 1024.0f);
		sizeTotal = "GiB";
	}

	char init_buffer[64];
	sprintf_s(init_buffer, 64, "\n%u leaked allocation(s).  Total: %0.3f %s\n", g_allocCount, reportedTotalBytes, sizeTotal.c_str());
	//OutputDebugStringA(init_buffer);
	//printf(init_buffer);
	LogTaggedPrintf("leaks", init_buffer);

	for(auto iterate = report_callstack.begin(); iterate != report_callstack.end(); ++iterate)
	{
		currentNode = *iterate;
		callstack_list* nextNode = currentNode->next;

		uint32_t& currentHash = currentNode->currentVal->m_hash;
		uint32_t nextHash;
		if (nextNode == nullptr)
			nextHash = currentHash + 1;
		else
			nextHash = nextNode->currentVal->m_hash;

		if (nextHash == currentHash) 
		{
			totalSimiliarSize += currentNode->total_allocation;
			totalSimiliarAllocs++;
		}

		if (nextHash != currentHash)
		{
			//Print total allocs for type and total size
			float reportedBytes = (float)totalSimiliarSize;
			std::string size = "B";

			if (totalSimiliarSize > (2 * 1024) && totalSimiliarSize < (2 * 1024 * 1024))
			{
				//KB
				reportedBytes /= 1024.0f;
				size = "KiB";
			}
			else if ((unsigned long)totalSimiliarSize > (2UL * 1024UL * 1024UL) && (unsigned long)totalSimiliarSize < (2UL * 1024UL * 1024UL * 1024UL))
			{
				//MB
				reportedBytes /= (1024.0f * 1024.0f);
				size = "MiB";
			}
			else if ((unsigned long)totalSimiliarSize > (2UL * 1024UL * 1024UL * 1024UL))
			{
				//GB
				reportedBytes /= (1024.0f * 1024.0f * 1024.0f);
				size = "GiB";
			}

			char collection_buffer[128];
			sprintf_s(collection_buffer, 128, "\nGroup contained %s allocation(s), Total: %0.3f %s\n", std::to_string(totalSimiliarAllocs).c_str(), reportedBytes, size.c_str());
			//OutputDebugStringA(collection_buffer);
			//printf(collection_buffer);
			LogTaggedPrintf("allocs", collection_buffer);

			//Reset total allocs and size
			totalSimiliarAllocs = 0;
			totalSimiliarSize = 0;
		}

		// Printing a call stack, happens when making report
		char line_buffer[512];
		callstack_line_t lines[128];
		uint line_count = CallstackGetLines(lines, 128, currentNode->currentVal);
		for (uint i = 0; i < line_count; ++i) {
			// this specific format will make it double click-able in an output window 
			// taking you to the offending line.

			if(args == "" || args == "default" || (currentNode->currentVal->m_time >= startTime - 0.00001 && currentNode->currentVal->m_time <= endTime + 0.00001))
			{
				//Print Line For Call Stack
				sprintf_s(line_buffer, 512, "     %s(%u): %s\n", lines[i].filename, lines[i].line, lines[i].function_name);
			}

			// print to output and console
			//OutputDebugStringA(line_buffer);
			//printf(line_buffer);
			LogTaggedPrintf("stack", line_buffer);
		}
	}
}

bool SortCallStackList(const callstack_list* first, const callstack_list* second)
{
	return (first->total_allocation < second->total_allocation);
}

void ConsolePrintMemValues(const std::string& args)
{
	args;

	#if defined (TRACK_MEMORY) 

		#if (TRACK_MEMORY == TRACK_MEMORY_BASIC)
			float reportedBytes = (float)g_allocatedByteCount;
			std::string size = " B";

			if (g_allocatedByteCount > (2 * 1024) && g_allocatedByteCount < (2 * 1024 * 1024))
			{
				//KB
				reportedBytes /= 1024.0f;
				size = " KiB";
			}
			else if (g_allocatedByteCount > (2 * 1024 * 1024) && g_allocatedByteCount < (2 * 1024 * 1024 * 1024))
			{
				//MB
				reportedBytes /= (1024.0f * 1024.0f);
				size = " MiB";
			}
			else if (g_allocatedByteCount > (2 * 1024 * 1024 * 1024))
			{
				//GB
				reportedBytes /= (1024.0f * 1024.0f * 1024.0f);
				size = " GiB";
			}

			command_cb totalAllocations;
			totalAllocations.message = "Total Allocations: " + std::to_string(g_allocCount);
			totalAllocations.color = Rgba(255, 153, 51, 255);
			g_console->m_logHistory.push_back(totalAllocations);
			g_console->m_cursorY -= g_console->m_consoleFont->m_lineHeight;

			command_cb totalAllocatedBytes;
			totalAllocatedBytes.message = "Bytes Allocated: " + std::to_string(reportedBytes) + size;
			totalAllocatedBytes.color = Rgba(255, 153, 51, 255);
			g_console->m_logHistory.push_back(totalAllocatedBytes);
			g_console->m_cursorY -= g_console->m_consoleFont->m_lineHeight;

			command_cb lastFrameAlloc;
			lastFrameAlloc.message = "Allocations Last Frame: " + std::to_string(g_frameAllocs);
			lastFrameAlloc.color = Rgba(255, 153, 51, 255);
			g_console->m_logHistory.push_back(lastFrameAlloc);
			g_console->m_cursorY -= g_console->m_consoleFont->m_lineHeight;

			command_cb freesLastFrame;
			freesLastFrame.message = "Frees Last Frame: " + std::to_string(g_frameFrees);
			freesLastFrame.color = Rgba(255, 153, 51, 255);
			g_console->m_logHistory.push_back(freesLastFrame);
			g_console->m_cursorY -= g_console->m_consoleFont->m_lineHeight;

		#elif (TRACK_MEMORY == TRACK_MEMORY_VERBOSE)
			ReportVerboseCallStacks(args);
		#endif

	#endif

}

void ShutDownVerboseReport()
{
	#if defined(TRACK_MEMORY)
		if (g_wasReportRun == true)
		{
			CallstackSystemDeinit();
			return;
		}
	
		#if (TRACK_MEMORY == TRACK_MEMORY_VERBOSE)
			ReportVerboseCallStacks("");
			CallstackSystemDeinit();
		#endif
	#endif
}

uint GetAllocCount()
{
	return g_allocCount;
}

uint GetFrameAllocs()
{
	return g_frameAllocs;
}

uint GetFrameFrees()
{
	return g_frameFrees;
}

size_t GetAllocByteCount()
{
	return g_allocatedByteCount;
}

void InitializeVerboseReporting()
{
	#if defined(TRACK_MEMORY)
		#if (TRACK_MEMORY == TRACK_MEMORY_VERBOSE)
			CallstackSystemInit();
		#endif
	#endif
}