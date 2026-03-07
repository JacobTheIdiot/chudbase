#pragma once

#include "core/mem/memory.h"

#include "cs2/entity/entityhandle.h"
#include "core/util/log.h"

#define MAX_ENTITIES_IN_LIST 512
#define MAX_ENTITY_LISTS 64 // 0x3F
#define MAX_TOTAL_ENTITIES MAX_ENTITIES_IN_LIST * MAX_ENTITY_LISTS

class C_BaseEntity;
class CGameEntitySystem
{
public:
	// GetClientEntity
	template <typename T = C_BaseEntity>
	T* Get(int nIndex)
	{
		return reinterpret_cast<T*>(this->GetEntityByIndex(nIndex));
	}

	// GetClientEntityFromHandle
	template <typename T = C_BaseEntity>
	T* Get(const CBaseHandle hHandle)
	{
		if (!hHandle.IsValid())
			return nullptr;

		return reinterpret_cast<T*>(this->GetEntityByIndex(hHandle.GetEntryIndex()));
	}

	int GetHighestEntityIndex()
	{
		return *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x20F0); //might still be 0x20F0 
	}

	template<class T>
	T* CreateEntityByClassName(const char* szName) {
		// x-ref: str: Attempted to create unknown entity classname \"%s\"!\n
		using fn_t = void* (__fastcall*)(void*, int, const char*, int, int, int, char);
		static fn_t fn = (fn_t)memory::FindPattern(CLIENT_DLL, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 49 8B F8");
		return reinterpret_cast<T*>(fn(this, -1, szName, 0, -1, -1, 0));
	}

private:
	void* GetEntityByIndex(int nIndex)
	{
		//should prolly make a functions file for this tbh but i'm lazy rn
		//xref: str: "screenshot" -> look for a function parsing a qword and pointer -> follow call
		void* (__fastcall * fnGetBaseEntity)(CGameEntitySystem*, int) = reinterpret_cast<decltype(fnGetBaseEntity)>(memory::FindPattern(CLIENT_DLL, XOR("4C 8D 49 ? 81 FA")));

		return fnGetBaseEntity(this, nIndex);
	}
};