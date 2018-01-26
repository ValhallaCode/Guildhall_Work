#include "Engine/Core/Logging.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/CallStack.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Core/CriticalSection.hpp"
#include "Engine/Core/Event.hpp"
#include "Engine/Core/Signal.hpp"
#include "Engine/Core/ThreadSafeQueue.hpp"
#include "Engine/Core/Job.hpp"
#include <time.h>

#pragma warning(push)
#pragma warning(disable: 4996)

static ThreadSafeQueue<std::string> g_messages;
static bool g_loggerThreadRunning = false;
static bool g_flushMessageRequest = false;
static thread_handle gLoggerThread = nullptr;
static Signal gLogSignal;
static EventV0 gLogEvent;
static FILE* g_filePTR = nullptr;
static JobConsumer g_logConsumer;

static std::vector<const char*> g_loggerDisabledTags; // Black List
static bool g_enableAllTags = true;

tm GetTimeInfo()
{
	time_t timeStruct;
	tm thing;
	time(&timeStruct);

	localtime_s(&thing, &timeStruct);

	return thing;
}

std::string GetTimeStampForInsideLog()
{
	tm thing = GetTimeInfo();

	char time_string[32];
	sprintf(time_string, "[ %i/%i/%i @ %i:%i:%i ]", thing.tm_mon, thing.tm_mday, thing.tm_year, thing.tm_hour, thing.tm_min, thing.tm_sec);

	return std::string(time_string);
}

std::wstring GetTimeStampForLogName()
{
	tm thing = GetTimeInfo();

	char file_date[32];
	sprintf(file_date, "log_%i%i%i_%i%i%i.log", thing.tm_mon, thing.tm_mday, thing.tm_year, thing.tm_hour, thing.tm_min, thing.tm_sec);

	std::string string = std::string(file_date);
	return std::wstring(string.begin(), string.end());
}

bool IsTagInBlackList(const char* tag)
{
	for (unsigned int index = 0; index < g_loggerDisabledTags.size(); ++index)
	{
		const char* val = g_loggerDisabledTags[index];
		if (val == tag)
		{
			return true;
		}
	}

	return false;
}


void LogDisable(char const *tag)
{
	g_loggerDisabledTags.push_back(tag);
}

void ConsoleLogDisable(void* data)
{
	arguments args = *(arguments*)data;
	for (std::string& string : args.arg_list)
		LogDisable(string.c_str());
}

void LogEnable(char const *tag)
{
	for (unsigned int index = 0; index < g_loggerDisabledTags.size(); ++index)
	{
		const char* val = g_loggerDisabledTags[index];
		if (val == tag)
		{
			g_loggerDisabledTags[index] = g_loggerDisabledTags.back();
			g_loggerDisabledTags.pop_back();
		}
	}
}

void ConsoleLogEnable(void* data)
{
	arguments args = *(arguments*)data;
	for(std::string& string : args.arg_list)
		LogEnable(string.c_str());
}

void LogDisableAll() 
{
	g_enableAllTags = false;
}

void ConsoleDisableAll(void*)
{
	LogDisableAll();
}

void LogEnableAll()
{
	g_enableAllTags = true;
	g_loggerDisabledTags.clear();
}

void ConsoleEnableAll(void*)
{
	LogEnableAll();
}

//------------------------------------------------------------------------
uint FlushMessages(FILE *fh)
{
	uint count = 0;
	std::string msg;

	while (g_messages.pop(&msg)) {
		gLogEvent.trigger(&msg);
		++count;
	}

	if (g_flushMessageRequest)
	{
		fflush(fh);
		g_flushMessageRequest = false;
	}

	return count;
}

//------------------------------------------------------------------------
void LogWriteToFile(void *user_arg, void *event_arg)
{
	FILE *fh = (FILE*)user_arg;
	std::string *msg = (std::string*)event_arg;

	fprintf(fh, "%s\n", msg->c_str());
}

//------------------------------------------------------------------------
void LogWriteToDebugger(void *user_arg, void *event_arg)
{
	user_arg;
	std::string *msg = (std::string*)event_arg;
	OutputDebugStringA(msg->c_str());
	OutputDebugStringA("\n");
}


//------------------------------------------------------------------------
void LoggerThread(void*)
{
	if(nullptr == g_filePTR)
	{
		errno_t err = fopen_s(&g_filePTR, "Data/Log/output.log", "w+");
		if ((err != 0) || (g_filePTR == nullptr)) {
			return;
		}
	}

	gLogEvent.subscribe(g_filePTR, LogWriteToFile);

	while (g_loggerThreadRunning) {
		gLogSignal.wait();
		FlushMessages(g_filePTR);
		// Messages are empty

	}

	FlushMessages(g_filePTR);
	fclose(g_filePTR);
}

void LogFlush()
{
	if (g_loggerThreadRunning) {

		g_flushMessageRequest = true;

		while (g_flushMessageRequest)
		{
			// Loops until Flush has Finished pushing to file
			ThreadYield();
		}
	}
}

void LogTaggedPrintv(char const *tag, char const *format, va_list variableArgumentList)
{
	if (!g_enableAllTags && IsTagInBlackList(tag))
		return;

	const int MESSAGE_MAX_LENGTH = 2048;
	char messageLiteral[MESSAGE_MAX_LENGTH];
	vsnprintf_s(messageLiteral, MESSAGE_MAX_LENGTH, _TRUNCATE, format, variableArgumentList);
	va_end(variableArgumentList);
	messageLiteral[MESSAGE_MAX_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string string = GetTimeStampForInsideLog();
	string += " [ ";
	string += tag;
	string += " ] ";
	string += messageLiteral;

	g_messages.push(string.c_str());
	gLogSignal.signal_all();
}

void LogCallStackPrint(const char* tag, const char* format, ...)
{
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	LogTaggedPrintv(tag, format, variableArgumentList);


	CallStack* stack = CreateCallstack(0);
	

	char line_buffer[512];
	callstack_line_t lines[128];
	uint line_count = CallstackGetLines(lines, 128, stack);
	for (uint i = 0; i < line_count; ++i) {
		// this specific format will make it double click-able in an output window 
		// taking you to the offending line.

		//Print Line For Call Stack
		sprintf_s(line_buffer, 512, "       %s(%u): %s\n\0", lines[i].filename, lines[i].line, lines[i].function_name);
		
		// print to output and console
		LogTaggedPrintf(tag, line_buffer);
	}
}

//------------------------------------------------------------------------
void LogWarning(char const *msg, ...)
{
	va_list variableArgumentList;
	va_start(variableArgumentList, msg);
	LogTaggedPrintv("warning", msg, variableArgumentList);
}

//------------------------------------------------------------------------
void LogError(char const *msg, ...)
{
	va_list variableArgumentList;
	va_start(variableArgumentList, msg);
	LogTaggedPrintv("error", msg, variableArgumentList);
	ASSERT_OR_DIE(false, "An error was thrown, see log file!");
}

//------------------------------------------------------------------------
void LogPrint(char const *msg, ...)
{
	va_list variableArgumentList;
	va_start(variableArgumentList, msg);
	LogTaggedPrintv("default", msg, variableArgumentList);
}

//------------------------------------------------------------------------
void LogStartup()
{
	g_loggerThreadRunning = true;
	gLoggerThread = ThreadCreate(LoggerThread, nullptr);
	g_logConsumer.add_category(JOB_LOGGER);
	JobSystemSetCategorySignal(JOB_LOGGER, &gLogSignal);
}

void LogConsume()
{
	g_logConsumer.consume_all();
}

void CreateHistoryFile()
{
	std::wstring newPath = L"Data/Log/LogHistory/";
	std::wstring nameString = GetTimeStampForLogName();
	//std::string nameString = std::string(newName);
	newPath += nameString;


	//FILE *fh = nullptr;
	//errno_t err = fopen_s(&fh, "Data/Log/output.log", "w+");

	BOOL b = CopyFile(L"Data/Log/output.log", newPath.c_str(), 0);
	if (!b) {
		LogError("Error: %lu", GetLastError());
	}
}

//------------------------------------------------------------------------
void LogShutdown()
{
	g_loggerThreadRunning = false;
	gLogSignal.signal_all();
	ThreadJoin(gLoggerThread);
	gLoggerThread = INVALID_THREAD_HANDLE;
	CreateHistoryFile();
}

struct logger_test_info
{
	int thread_id;
	int line_count;
};

void LogTestWrite(void* data)
{
	logger_test_info* thread_info = (logger_test_info*)data;

	for (int line = 1; line <= thread_info->line_count; ++line)
	{
		LogPrint("Thread %i, writing line %i", thread_info->thread_id, line);
	}
}

void LogTest(int thread_count, int line_count)
{
	for (int thread_size = 0; thread_size < thread_count; ++thread_size)
	{
		logger_test_info info;
		info.thread_id = thread_size;
		info.line_count = line_count;
		thread_handle th = ThreadCreate(LogTestWrite, &info);
		ThreadDetach(th);
	}
}

void LogFlushTest(char const *text)
{
	LogPrint(text);
	LogFlush();
	ASSERT_OR_DIE(false, "Break to check Flush Test for Logger!");
}

void FlushLoggerCmd(void* data)
{
	arguments args = *(arguments*)data;
	g_console->ConsolePrintf(Rgba(255, 255, 255, 255), "Starting Flush Command...", "");
	for(std::string& string : args.arg_list)
		LogFlushTest(string.c_str());
	g_console->ConsolePrintf(Rgba(255, 255, 255, 255), "Finished Flush Command", "");
}

void LogTaggedPrintf(char const *tag, char const *format, ...)
{
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	LogTaggedPrintv(tag, format, variableArgumentList);
}

void CopyLogFile(void* data)
{
	fclose(g_filePTR);

	const char* file_path = (const char*)data;
	std::wstring path(strlen(file_path) + 1, L'#');

	mbstowcs(&path[0], file_path, strlen(file_path) + 1);

	BOOL b = CopyFile(L"Data/Log/output.log", path.c_str(), 0);
	if (!b) {
		LogError("Error: %lu", GetLastError());
	}

	fopen_s(&g_filePTR, "Data/Log/output.log", "a+");
}


void LogCopyFromDev(void* data)
{
	arguments args = *(arguments*)data;
	for(std::string& string : args.arg_list)
	{
		Job* job = JobCreate(JOB_LOGGER, CopyLogFile, (void*)string.c_str());
		JobDispatchAndRelease(job);
	}
}

void RegisterLogCommands()
{
	g_console->RegisterCommand("FlushLog", FlushLoggerCmd);
	g_console->RegisterCommand("EnableAllLogTags", ConsoleEnableAll);
	g_console->RegisterCommand("DisableAllLogTags", ConsoleDisableAll);
	g_console->RegisterCommand("LogEnable", ConsoleLogEnable);
	g_console->RegisterCommand("LogDisable", ConsoleLogDisable);
	g_console->RegisterCommand("CopyLog", LogCopyFromDev);
}




#pragma warning(pop)