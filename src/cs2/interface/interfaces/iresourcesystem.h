#pragma once

// used: callvfunc
#include "core/mem/memory.h"

struct ResourceBinding_t;

class IResourceSystem
{
public:
	void* QueryInterface(const char* szInterfaceName)
	{
		return memory::CallVFunc<void*, 2U>(this, szInterfaceName);
	}
};

class CResourceHandleUtils
{
public:
	void DeleteResource(const ResourceBinding_t* pBinding)
	{
		memory::CallVFunc<void, 2U>(this, pBinding);
	}
};
