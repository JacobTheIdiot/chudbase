#pragma once
// used: MEM_PAD
#include "core/mem/memory.h"


class CUtlBuffer
{
public:
	MEM_PAD(0x80);

	CUtlBuffer(int a1, int nSize, int a3)
	{
#ifdef CS_PARANOID
		CS_ASSERT(memory::fnUtlBufferInit != nullptr);
#endif

		memory::fnUtlBufferInit(this, a1, nSize, a3);
	}

	void PutString(const char* szString)
	{
#ifdef CS_PARANOID
		CS_ASSERT(memory::fnUtlBufferPutString != nullptr);
#endif

		memory::fnUtlBufferPutString(this, szString);
	}

	void EnsureCapacity(int nSize)
	{
#ifdef CS_PARANOID
		CS_ASSERT(memory::fnUtlBufferEnsureCapacity != nullptr);
#endif

		memory::fnUtlBufferEnsureCapacity(this, nSize);
	}
};
