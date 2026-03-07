#pragma once
#include "core/mem/memory.h"

class CNetworkGameClient
{
public:
	bool IsConnected()
	{
		return memory::CallVFunc<bool, 12U>(this);
	}

	// force game to clear cache and reset delta tick
	void FullUpdate()
	{
		// @ida: #STR: "Requesting full game update (%s)...\n"
		memory::CallVFunc<void, 28U>(this, XOR("unk"));
	}

	int GetDeltaTick()
	{
		// @ida: offset in FullUpdate();
		// (nDeltaTick = -1) == FullUpdate() called
		return *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x25C);
	}
};

class INetworkClientService
{
public:
	[[nodiscard]] CNetworkGameClient* GetNetworkGameClient()
	{
		return memory::CallVFunc<CNetworkGameClient*, 23U>(this);
	}
};
