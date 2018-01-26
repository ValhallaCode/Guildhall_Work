#include "Engine/Core/CriticalSection.hpp"
#include "Engine/Input/FileStream.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Job.hpp"

CriticalSection::CriticalSection()
{
	InitializeCriticalSection(&m_windowsCritical);
}

CriticalSection::~CriticalSection()
{
	DeleteCriticalSection(&m_windowsCritical);
}

void CriticalSection::Lock()
{
	EnterCriticalSection(&m_windowsCritical);
}

void CriticalSection::Unlock()
{
	LeaveCriticalSection(&m_windowsCritical);
}

ScopedCriticalSection::ScopedCriticalSection(CriticalSection* cs)
{
	m_critical = cs;
	m_critical->Lock();
}

ScopedCriticalSection::~ScopedCriticalSection()
{
	m_critical->Unlock();
}

struct thread_pass_data_t
{
	thread_cb cb;
	void *arg;
};

//------------------------------------------------------------------------
static DWORD WINAPI ThreadEntryPointCommon(void *arg)
{
	thread_pass_data_t *pass_ptr = (thread_pass_data_t*)arg;

	pass_ptr->cb(pass_ptr->arg);
	delete pass_ptr;
	return 0;
}

//------------------------------------------------------------------------
// Creates a thread with the entry point of cb, passed data
thread_handle ThreadCreate(thread_cb cb, void *data)
{
	// handle is like pointer, or reference to a thread
	// thread_id is unique identifier
	thread_pass_data_t *pass = new thread_pass_data_t();
	pass->cb = cb;
	pass->arg = data;

	DWORD thread_id;
	thread_handle th = ::CreateThread(nullptr,   // SECURITY OPTIONS
		0,                         // STACK SIZE, 0 is default
		ThreadEntryPointCommon,    // "main" for this thread
		pass,                     // data to pass to it
		0,                         // initial flags
		&thread_id);              // thread_id

	return th;
}

//------------------------------------------------------------------------
void ThreadSleep(unsigned int ms)
{
	::Sleep(ms);
}

//------------------------------------------------------------------------
void ThreadYield()
{
	::SwitchToThread();
}

//------------------------------------------------------------------------
// Releases my hold on this thread.
void ThreadDetach(thread_handle th)
{
	::CloseHandle(th);
}

//------------------------------------------------------------------------
void ThreadJoin(thread_handle th)
{
	::WaitForSingleObject(th, INFINITE);
	::CloseHandle(th);
}

//------------------------------------------------------------------------
struct thread_test_info
{
	const char* filepath;
	size_t byte_count;
};

//------------------------------------------------------------------------
void GenerateGarbageWork(void *data)
{
	thread_test_info* testInfo = (thread_test_info*)data;
	FileBinaryStream stream;
	stream.open_for_write(testInfo->filepath);

	for (unsigned int index = 0; index < testInfo->byte_count; index += sizeof(int))
	{
		int rdm = GetRandomIntInRange(0, 1000);
		stream.write(rdm);
	}
	stream.close();
}

//------------------------------------------------------------------------
void ThreadTest(char const *filePath, size_t byte_count)
{
	thread_test_info* testInfo = new thread_test_info();
	testInfo->filepath = filePath;
	testInfo->byte_count = byte_count;

	thread_handle th = ThreadCreate(GenerateGarbageWork, testInfo);
	ThreadJoin(th);
}

//--------------------------------------------------------------------------
void CreateLargeFileAsync(char const *filePath, size_t byte_count)
{
	thread_test_info* testInfo = new thread_test_info();
	testInfo->filepath = filePath;
	testInfo->byte_count = byte_count;

	JobRun(JOB_GENERIC, GenerateGarbageWork, testInfo);
}