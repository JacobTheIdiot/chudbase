#include "detourhook.h"
#include <Windows.h>
#include <string>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <vector>
#include "cs2/datatypes/matrix.h"
#include "cs2/interface/Interfaces.h"
#include "cs2/interface/interfaces/iswapchaindx11.h"
#include "cs2/interface/interfaces/iclientmodeshared.h"
#include "cs2/interface/interfaces/ccsgoinput.h"
#include "cs2/interface/interfaces/iengineclient.h"
#include "cs2/inputsystem.h"
#include "cs2/entity/entity.h"
//credit: asphyxia (for the vtable namespace)
namespace VTABLE
{
	namespace D3D
	{
		enum
		{
			PRESENT = 8U,
			RESIZEBUFFERS = 13U,
			RESIZEBUFFERS_CSTYLE = 39U,
		};
	}

	namespace DXGI
	{
		enum
		{
			CREATESWAPCHAIN = 10U,
		};
	}
	namespace CLIENT
	{
		enum
		{
			CREATEMOVEPREPREDICTION = 5U,
			MOUSEINPUTENABLED = 19U,
			CREATEMOVEPOSTPREDICTION = 21U,
			FRAMESTAGENOTIFY = 36U,
		};
	}

	namespace ENTITYSYSTEM
	{
		enum
		{
			ONADDENTITY = 15U,
			ONREMOVEENTITY = 16U,
		};
	}


}

namespace View
{
	inline ViewMatrix_t m_ViewMatrix = { };
}

namespace Globals
{

}
//client.dll
//xref: str: "CCSGOINPUT"
#define CREATEMOVE_PATTERN XOR("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 48 8B F9 E8 ? ? ? ? 48 8B 0D")

//xref: class: CGameEntitySystem->index 15 and 16
#define ON_ADD_ENTITY_PATTERN XOR("48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 81")
#define ON_REMOVE_ENTITY_PATTERN XOR("48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 89")

//xref: class: ClientModeCSNormal->index 15
#define OVERRIDE_VIEW_PATTERN XOR("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B FA E8")

//scenesystem.dll:
//placeholder for now

namespace detours
{
	//client.dll:
	inline CDetourHook Present = CDetourHook(XOR("Present"));
	inline CDetourHook WndProc = CDetourHook(XOR("WndProc"));
	inline CDetourHook RelativeMouseMode = CDetourHook(XOR("RelativeMouseMode"));
	inline CDetourHook ResizeBuffers = CDetourHook(XOR("ResizeBuffers"));
	inline CDetourHook CreateSwapChain = CDetourHook(XOR("CreateSwapChain"));

	inline CDetourHook OnAddEntity = CDetourHook(XOR("OnAddEntity"));
	inline CDetourHook OnRemoveEntity = CDetourHook(XOR("OnRemoveEntity"));

	inline CDetourHook OverrideView = CDetourHook(XOR("OverrideView"));
	inline CDetourHook OnRenderStart = CDetourHook(XOR("OnRenderStart"));


	//scenesystem.dll:
}

namespace hooks
{
	bool setup();
	void shutdown();

	bool setuphook(const wchar_t* module, const char* pattern, LPVOID hook, LPVOID* original);

	inline std::vector<CDetourHook> vecDetours = {};
	inline void SubscribeHook(CDetourHook& detour, const bool create = true)
	{
		if (detour.bIsHooked)
			return;

		bool bFound = false;

		for (auto& d : vecDetours)
		{
			if (&d != &detour)
				continue;

			bFound = true;

			const bool bReplaceFn = d.pReplaceFn == detour.pReplaceFn;
			const bool bBaseFn = d.pBaseFn == detour.pBaseFn;
			const bool bOriginalFn = d.pOriginalFn == detour.pOriginalFn;

			if (create && bReplaceFn && (!bBaseFn || !bOriginalFn || d.pBaseFn == nullptr || d.pOriginalFn == nullptr))
				d.Create();
			else
				throw std::runtime_error(XOR("unknown error while subscribing hook"));
		}

		if (!bFound)
		{
			vecDetours.emplace_back(detour);
			if (create)
				detour.Create();
		}
	}
	inline void RevokeHook(CDetourHook& detour)
	{
		for (const auto& d : vecDetours)
		{
			const bool bReplaceFn = d.pReplaceFn == detour.pReplaceFn;
			const bool bBaseFn = d.pBaseFn == detour.pBaseFn;
			const bool bOriginalFn = d.pOriginalFn == detour.pOriginalFn;

			if (!bReplaceFn || !bBaseFn || !bOriginalFn)
				continue;

			detour.Restore();
			detour.~CDetourHook();
		}
	}

	HRESULT WINAPI hkPresent(IDXGISwapChain* pSwapChain, UINT uSyncInterval, UINT uFlags);
	HRESULT CS_FASTCALL hkResizeBuffers(IDXGISwapChain* pSwapChain, std::uint32_t nBufferCount, std::uint32_t nWidth, std::uint32_t nHeight, DXGI_FORMAT newFormat, std::uint32_t nFlags);
	HRESULT WINAPI hkCreateSwapChain(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain);
	long CALLBACK hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void* CS_FASTCALL hkRelativeMouseMode(void* pThisptr, bool bActive);

	void* __fastcall hkOnAddEntity(void* pThis, CEntityInstance* pInstance, CBaseHandle uHandle);
	void* __fastcall hkOnRemoveEntity(void* pThis, CEntityInstance* pInstance, CBaseHandle uHandle);

	void __fastcall hkOverrideView(void* a1, CViewSetup* pSetup);
	void __fastcall hkOnRenderStart(CViewRender* pViewRender);
}
