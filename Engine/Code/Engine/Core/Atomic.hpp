#pragma  once
#include <stdint.h>
#include <atomic>

typedef unsigned int uint;
typedef unsigned __int8 byte_t;


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

__forceinline
uint AtomicAdd(uint volatile *ptr, uint const value)
{
	return (uint) ::InterlockedAddNoFence((LONG volatile*)ptr, (LONG)value);
}

//--------------------------------------------------------------------
__forceinline
uint AtomicIncrement(uint *ptr)
{
	return (uint) ::InterlockedIncrementNoFence((LONG volatile*)ptr);
}

//--------------------------------------------------------------------
__forceinline
uint AtomicDecrement(uint *ptr)
{
	return (uint) ::InterlockedDecrementNoFence((LONG volatile*)ptr);
}

//--------------------------------------------------------------------
__forceinline
uint CompareAndSet(uint volatile *ptr, uint const comparand, uint const value)
{
	return ::InterlockedCompareExchange(ptr, value, comparand);
}

__forceinline
bool CompareAndSet64(uint64_t volatile *data, uint64_t *comparand, uint64_t *value)
{
	return 1 == ::InterlockedCompareExchange64((long long volatile*)data, *(long long*)value, *(long long*)comparand);
}


//--------------------------------------------------------------------
template <typename T>
__forceinline T* CompareAndSetPointer(T *volatile *ptr, T *comparand, T *value)
{
	return (T*)::InterlockedCompareExchangePointerNoFence((PVOID volatile*)ptr, (PVOID)value, (PVOID)comparand);
}

