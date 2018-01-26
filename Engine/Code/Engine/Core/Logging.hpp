#pragma once

void LogTaggedPrintf(char const *tag, char const *format, ...);
void LogPrint(char const *msg, ...);
void RegisterLogCommands();
void LogFlushTest(char const *text);
void LogTest(int thread_count, int line_count);
void LogShutdown();
void LogStartup();
void LogError(char const *msg, ...);
void LogWarning(char const *msg, ...);
void LogCallStackPrint(const char* tag, const char* format, ...);
void LogConsume();