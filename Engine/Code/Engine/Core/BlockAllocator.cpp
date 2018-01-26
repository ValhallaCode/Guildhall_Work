#include "Engine/Core/BlockAllocator.hpp"
#include "Engine/Core/ThreadSafeQueue.hpp"



class SystemAllocator : public IAllocator
{
public:
	void* alloc(size_t size) { return ::malloc(size); }
	void free(void *ptr) { return ::free(ptr); }
};

static int const BLOCK_SIZE = 1024;
static uint const NUM_QUEUES = 8;

static SystemAllocator gSystemAllocator;
static BlockAllocator gBlockAllocator(BLOCK_SIZE);
static ThreadSafeBlockAllocator gTSBlockAllocator(BLOCK_SIZE);
static LocklessBlockAllocator gLFBlockAllocator(BLOCK_SIZE);

static ThreadSafeQueue<void*> gQueues[NUM_QUEUES];

// static uint AddBytes(void *ptr)
// {
// 	uint v = 0;
// 	byte_t *c = (byte_t*)ptr;
// 	for (uint i = 0; i < 512; ++i) {
// 		v += c[i];
// 		c[i] = (byte_t)(2 * c[i] + 3);
// 	}
// 
// 	return v;
// }