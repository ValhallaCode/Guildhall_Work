#include "Engine/Core/CallStack.hpp"
#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Core/Time.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#pragma warning( disable : 4091 ) //  warning C4091: 'typedef ': ignored on left of '' when no variable is declared
#include <DbgHelp.h>


const unsigned int MAX_SYMBOL_NAME_LENGTH = 128;
const unsigned int MAX_FILENAME_LENGTH = 1024;
const unsigned int MAX_DEPTH = 128;

typedef BOOL(__stdcall *sym_initialize_t)(IN HANDLE hProcess, IN PSTR UserSearchPath, IN BOOL fInvadeProcess);
typedef BOOL(__stdcall *sym_cleanup_t)(IN HANDLE hProcess);
typedef BOOL(__stdcall *sym_from_addr_t)(IN HANDLE hProcess, IN DWORD64 Address, OUT PDWORD64 Displacement, OUT PSYMBOL_INFO Symbol);

typedef BOOL(__stdcall *sym_get_line_t)(IN HANDLE hProcess, IN DWORD64 dwAddr, OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE64 Symbol);


static HMODULE g_debugHelp;
static HANDLE g_process;
static SYMBOL_INFO* g_symbol;

static sym_initialize_t sym_initialize;
static sym_cleanup_t sym_cleanup;
static sym_from_addr_t sym_fromAddr;
static sym_get_line_t sym_getLineFromAddr64;

static int g_callstackCount = 0;

//------------------------------------------------------------------------
CallStack::CallStack()
	: m_hash(0)
	, m_frameCount(0) {}


//------------------------------------------------------------------------
bool CallstackSystemInit()
{
	// Load the dll, similar to OpenGL function fecthing.
	// This is where these functions will come from.
	g_debugHelp = LoadLibraryA("dbghelp.dll");
	if (g_debugHelp == nullptr) {
		return false;
	}

	// Get pointers to the functions we want from the loded library.
	sym_initialize = (sym_initialize_t)GetProcAddress(g_debugHelp, "SymInitialize");
	sym_cleanup = (sym_cleanup_t)GetProcAddress(g_debugHelp, "SymCleanup");
	sym_fromAddr = (sym_from_addr_t)GetProcAddress(g_debugHelp, "SymFromAddr");
	sym_getLineFromAddr64 = (sym_get_line_t)GetProcAddress(g_debugHelp, "SymGetLineFromAddr64");

	// Initialize the system using the current process [see MSDN for details]
	g_process = ::GetCurrentProcess();
	sym_initialize(g_process, NULL, TRUE);

	// Preallocate some memory for loading symbol information. 
	g_symbol = (SYMBOL_INFO *) ::malloc(sizeof(SYMBOL_INFO) + (MAX_FILENAME_LENGTH * sizeof(char)));
	g_symbol->MaxNameLen = MAX_FILENAME_LENGTH;
	g_symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	return true;
}

//------------------------------------------------------------------------
void CallstackSystemDeinit()
{
	// cleanup after ourselves
	free(g_symbol);
	g_symbol = nullptr;

	sym_cleanup(g_process);

	FreeLibrary(g_debugHelp);
	g_debugHelp = NULL;
}

//------------------------------------------------------------------------
// Can not be static - called when
// the callstack is freed.
void DestroyCallstack(CallStack *ptr)
{
	::free(ptr);
}


//------------------------------------------------------------------------
CallStack* CreateCallstack(uint skip_frames)
{
	// Cappture the callstack frames - uses a windows call
	void *stack[MAX_DEPTH];
	DWORD hash;

	// skip_frames:  number of frames to skip [starting at the top - so don't return the frames for "CreateCallstack" (+1), plus "skip_frame_" layers.
	// max_frames to return
	// memory to put this information into.
	// out pointer to back trace hash.
	uint32_t frames = CaptureStackBackTrace(1 + skip_frames, MAX_DEPTH, stack, &hash);

	// create the callstack using an untracked allocation
	CallStack *cs = (CallStack*) ::malloc(sizeof(CallStack));

	// force call the constructor (new in-place0
	cs = new (cs) CallStack();

	// copy the frames to our callstack object
	uint frame_count = min(MAX_FRAMES_PER_CALLSTACK, frames);
	cs->m_frameCount = frame_count;
	memcpy(cs->m_frames, stack, sizeof(void*) * frame_count);

	cs->m_time = GetCurrentTimeSeconds();

	cs->m_hash = hash;

	return cs;
}

//------------------------------------------------------------------------
// Fills lines with human readable data for the given callstack
// Fills from top to bottom (top being most recently called, with each next one being the calling function of the previous)
//
// Additional features you can add;
// [ ] If a file exists in yoru src directory, clip the filename
// [ ] Be able to specify a list of function names which will cause this trace to stop.
uint CallstackGetLines(callstack_line_t *line_buffer, uint const max_lines, CallStack *cs)
{
	IMAGEHLP_LINE64 line_info;
	DWORD line_offset = 0; // Displacement from the beginning of the line 
	line_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);


	uint count = min(max_lines, cs->m_frameCount);
	uint idx = 0;

	for (uint i = 0; i < count; ++i) {
		callstack_line_t *line = &(line_buffer[idx]);
		DWORD64 ptr = (DWORD64)(cs->m_frames[i]);
		if (FALSE == sym_fromAddr(g_process, ptr, 0, g_symbol)) {
			continue;
		}

		strcpy_s(line->function_name, 256, g_symbol->Name);

		BOOL bRet = sym_getLineFromAddr64(
			GetCurrentProcess(), // Process handle of the current process 
			ptr, // Address 
			&line_offset, // Displacement will be stored here by the function 
			&line_info);         // File name / line information will be stored here 

		if (bRet) {
			line->line = line_info.LineNumber;

			strcpy_s(line->filename, 128, line_info.FileName);
			line->offset = line_offset;

		}
		else {
			// no information
			line->line = 0;
			line->offset = 0;
			strcpy_s(line->filename, 128, "N/A");
		}

		++idx;
	}

	return idx;
}

void CallstackExample()
{
	// create a callstack, skip no frames (so CallstackExample should exist in the returned callstack)
	CallStack *cs = CreateCallstack(0);

	// cleanup
	DestroyCallstack(cs);
}
