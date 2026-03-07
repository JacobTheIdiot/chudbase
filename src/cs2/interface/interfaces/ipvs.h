#pragma once

// used: memory::CallVFunc
#include "core/mem/memory.h"

class CPVS
{
public:
	void Set(bool bState)
	{
		memory::CallVFunc<void*, 7U>(this, bState);
	}
};
