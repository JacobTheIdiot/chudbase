#pragma once

// used: callvfunc
#include "core/mem/memory.h"


enum EClientFrameStage : int
{
	FRAME_UNDEFINED = -1,
	FRAME_START,
	// a network packet is being received
	FRAME_NET_UPDATE_START,
	// data has been received and we are going to start calling postdataupdate
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	// data has been received and called postdataupdate on all data recipients
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	// received all packets, we can now do interpolation, prediction, etc
	FRAME_NET_UPDATE_END,
	// start rendering the scene
	FRAME_RENDER_START,
	// finished rendering the scene
	FRAME_RENDER_END,
	FRAME_NET_FULL_FRAME_UPDATE_ON_REMOVE
};

class IEngineClient
{
public:
	int GetMaxClients()
	{
		return memory::CallVFunc<int, 34U>(this);
	}

	bool IsInGame()
	{
		return memory::CallVFunc<bool, 38>(this);
	}

	bool IsConnected()
	{
		return memory::CallVFunc<bool, 39>(this);
	}

	// return CBaseHandle index
	int GetLocalPlayer()
	{
		int nIndex = -1;

		memory::CallVFunc<void, 53>(this, std::ref(nIndex), 0);

		return nIndex + 1;
	}

	[[nodiscard]] const char* GetLevelName()
	{
		return memory::CallVFunc<const char*, 62>(this);
	}

	[[nodiscard]] const char* GetLevelNameShort()
	{
		return memory::CallVFunc<const char*, 63>(this);
	}

	[[nodiscard]] const char* GetProductVersionString()
	{
		return memory::CallVFunc<const char*, 90>(this);
	}
	[[nodiscard]] const char* GetProductDateString()
	{
		return memory::CallVFunc<const char*, 89>(this);
	}

	void ExecuteClientCmdUnrestricted(const char* szCommand)
	{
		memory::CallVFunc<void, 49>(this, 0, szCommand, 0x7FFEF001);
	}
};
