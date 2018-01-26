#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <tuple>
#include <utility>

#define INVALID_THREAD_HANDLE 0

typedef void* thread_handle;
typedef void(*thread_cb)(void*);
typedef unsigned int uint;

class CriticalSection
{
public:
	CriticalSection();
	~CriticalSection();
	void Lock();
	void Unlock();
public:
	CRITICAL_SECTION m_windowsCritical;
};

class ScopedCriticalSection
{
public:
	ScopedCriticalSection(CriticalSection* cs);
	~ScopedCriticalSection();
public:
	CriticalSection* m_critical;
};

#define COMBINE1(X,Y) X##Y
#define COMBINE(X,Y) COMBINE1(X,Y)
#define SCOPE_LOCK( csp ) ScopedCriticalSection COMBINE(__scs_,__LINE__)(csp)


void ThreadTest(char const *path, size_t byte_size);
void CreateLargeFileAsync(char const *filePath, size_t byte_count);
thread_handle ThreadCreate(thread_cb cb, void *data);
void ThreadSleep(unsigned int ms);
void ThreadDetach(thread_handle th);
void ThreadJoin(thread_handle th);
void ThreadYield();


template <typename CB, typename ...ARGS>
struct pass_data_t
{
	CB cb;
	std::tuple<ARGS...> args;

	pass_data_t(CB cb, ARGS ...args)
		: cb(cb)
		, args(args...) {}
};


template <typename CB, typename TUPLE, size_t ...INDICES>
void ForwardArgumentsWithIndices(CB cb, TUPLE &args, std::integer_sequence<size_t, INDICES...>&)
{
	cb(std::get<INDICES>(args)...);
}

//------------------------------------------------------------------------
template <typename CB, typename ...ARGS>
void ForwardArgumentsThread(void *ptr)
{
	pass_data_t<CB, ARGS...> *args = (pass_data_t<CB, ARGS...>*) ptr;
	auto seq_args = std::make_index_sequence<sizeof...(ARGS)>();
	ForwardArgumentsWithIndices(args->cb, args->args, seq_args);
	delete args;
}

//------------------------------------------------------------------------
template <typename CB, typename ...ARGS>
thread_handle ThreadCreate(CB entry_point, ARGS ...args)
{
	pass_data_t<CB, ARGS...> *pass = new pass_data_t<CB, ARGS...>(entry_point, args...);
	return ThreadCreate(ForwardArgumentsThread<CB, ARGS...>, (void*)pass);
}