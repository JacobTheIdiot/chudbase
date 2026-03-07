#pragma once
// used: MEM_PAD, FindPattern
#include "core/mem/memory.h"


struct KV3ID_t
{
	const char* szName;
	std::uint64_t unk0;
	std::uint64_t unk1;
};

class CKeyValues3
{
public:
	MEM_PAD(0x100);
	std::uint64_t uKey;
	void* pValue;
	MEM_PAD(0x8);

	void LoadFromBuffer(const char* szString);
	bool LoadKV3(CUtlBuffer* buffer);

	static CKeyValues3* CreateMaterialResource()
	{
		using fnSetTypeKV3 = CKeyValues3 * (CS_FASTCALL*)(CKeyValues3*, unsigned int, unsigned int);
		static const fnSetTypeKV3 oSetTypeKV3 = reinterpret_cast<fnSetTypeKV3>(memory::FindPattern(CLIENT_DLL, XOR("40 53 48 83 EC 30 48 8B D9 49")));

#ifdef CS_PARANOID
		CS_ASSERT(oSetTypeKV3 != nullptr);
#endif

		CKeyValues3* pKeyValue = new CKeyValues3[0x10];
		return oSetTypeKV3(pKeyValue, 1U, 6U);
	}
};
