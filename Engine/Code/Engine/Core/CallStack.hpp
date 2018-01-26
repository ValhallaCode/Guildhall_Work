#pragma once
#include <stdint.h>

typedef unsigned int uint;

const unsigned int MAX_FRAMES_PER_CALLSTACK = 128;

struct callstack_line_t
{
	char filename[128];
	char function_name[256];
	uint32_t line;
	uint32_t offset;
};

class CallStack
{
public:
	CallStack();

	uint32_t m_hash;
	uint m_frameCount;
	double m_time;
	void* m_frames[MAX_FRAMES_PER_CALLSTACK];
};

bool CallstackSystemInit();
void CallstackSystemDeinit();

// As this is - this will create a callstack using malloc (untracked allocation), skipping the first few frames.
CallStack* CreateCallstack(uint skip_frames);
void DestroyCallstack(CallStack *c);

uint CallstackGetLines(callstack_line_t *line_buffer, uint const max_lines, CallStack *cs);