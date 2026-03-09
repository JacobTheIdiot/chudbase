#include "pch.h"
// used: [d3d] api
#include <d3d11.h>


#include "interfaces.h"

// used: findpattern, callvirtual, getvfunc...
#include "core/mem/memory.h"

// used: l_print
#include "core/util/log.h"

// used: iswapchaindx11
#include "interfaces/iswapchaindx11.h"
#include "interfaces/iresourcesystem.h"

#pragma region interfaces_get

using InstantiateInterfaceFn_t = void* (*)();

class CInterfaceRegister
{
public:
	InstantiateInterfaceFn_t fnCreate;
	const char* szName;
	CInterfaceRegister* pNext;
};

static const CInterfaceRegister* GetRegisterList(const wchar_t* wszModuleName)
{
	void* hModule = memory::GetModuleBaseHandle(wszModuleName);
	if (hModule == nullptr)
		return nullptr;

	std::uint8_t* pCreateInterface = reinterpret_cast<std::uint8_t*>(memory::GetExportAddress(hModule, XOR("CreateInterface")));

	if (pCreateInterface == nullptr)
	{
		L_PRINT(LOG_ERROR) << XOR("failed to get \"CreateInterface\" address");
		return nullptr;
	}

	return *reinterpret_cast<CInterfaceRegister**>(memory::ResolveRelativeAddress(pCreateInterface, 0x3, 0x7));
}

template <typename T = void*>
T* Capture(const CInterfaceRegister* pModuleRegister, const char* szInterfaceName)
{
	for (const CInterfaceRegister* pRegister = pModuleRegister; pRegister != nullptr; pRegister = pRegister->pNext)
	{
		if (const std::size_t nInterfaceNameLength = CRT::StringLength(szInterfaceName);
			// found needed interface
			CRT::StringCompareN(szInterfaceName, pRegister->szName, nInterfaceNameLength) == 0 &&
			// and we've given full name with hardcoded digits
			(CRT::StringLength(pRegister->szName) == nInterfaceNameLength ||
				// or it contains digits after name
				CRT::StringToInteger<int>(pRegister->szName + nInterfaceNameLength, nullptr, 10) > 0))
		{
			// capture our interface
			void* pInterface = pRegister->fnCreate();

#ifdef _DEBUG
			// log interface address
			L_PRINT(LOG_INFO) << XOR("captured \"") << pRegister->szName << XOR("\" interface at address: ") << console::AddFlags(LOG_MODE_INT_SHOWBASE | LOG_MODE_INT_FORMAT_HEX) << reinterpret_cast<std::uintptr_t>(pInterface);
#else
			L_PRINT(LOG_INFO) << XOR("captured \"") << pRegister->szName << XOR("\" interface");
#endif

			return static_cast<T*>(pInterface);
		}
	}

	L_PRINT(LOG_ERROR) << XOR("failed to find interface \"") << szInterfaceName << XOR("\"");
	return nullptr;
}

#pragma endregion

bool Interface::Setup()
{
	bool bSuccess = true;

#pragma region interface_game_exported
	const auto pTier0Handle = memory::GetModuleBaseHandle(TIER0_DLL);
	if (pTier0Handle == nullptr)
		return false;

	MemAlloc = *reinterpret_cast<IMemAlloc**>(memory::GetExportAddress(pTier0Handle, XOR("g_pMemAlloc")));
	bSuccess &= (MemAlloc != nullptr);

	const auto pSchemaSystemRegisterList = GetRegisterList(SCHEMASYSTEM_DLL);
	if (pSchemaSystemRegisterList == nullptr)
		return false;

	SchemaSystem = Capture<ISchemaSystem>(pSchemaSystemRegisterList, SCHEMA_SYSTEM);
	bSuccess &= (SchemaSystem != nullptr);

	const auto pInputSystemRegisterList = GetRegisterList(INPUTSYSTEM_DLL);
	if (pInputSystemRegisterList == nullptr)
		return false;

	InputSystem = Capture<IInputSystem>(pInputSystemRegisterList, INPUT_SYSTEM_VERSION);
	bSuccess &= (InputSystem != nullptr);

	const auto pEngineRegisterList = GetRegisterList(ENGINE2_DLL);
	if (pEngineRegisterList == nullptr)
		return false;

	const auto pMeshSystemRegister = GetRegisterList(L"meshsystem.dll");
	if (pMeshSystemRegister == nullptr)
		return false;

	m_pMeshSystem = Capture<CMeshSystem>(pMeshSystemRegister, "MeshSystem001");
	bSuccess &= (m_pMeshSystem != nullptr);

	const auto pSceneSystemRegister = GetRegisterList(SCENESYSTEM_DLL);
	if (pSceneSystemRegister == nullptr)
		return false;

	m_pSceneSystem = Capture<ISceneSystem>(pSceneSystemRegister, "SceneSystem_002");
	bSuccess &= (m_pSceneSystem != nullptr);


	GameResourceService = Capture<IGameResourceService>(pEngineRegisterList, GAME_RESOURCE_SERVICE_CLIENT);
	bSuccess &= (GameResourceService != nullptr);

	Engine = Capture<IEngineClient>(pEngineRegisterList, SOURCE2_ENGINE_TO_CLIENT);
	bSuccess &= (Engine != nullptr);

	NetworkClientService = Capture<INetworkClientService>(pEngineRegisterList, NETWORK_CLIENT_SERVICE);
	bSuccess &= (NetworkClientService != nullptr);

	const auto pTier0RegisterList = GetRegisterList(TIER0_DLL);
	if (pTier0RegisterList == nullptr)
		return false;

	Cvar = Capture<IEngineCVar>(pTier0RegisterList, ENGINE_CVAR);
	bSuccess &= (Cvar != nullptr);

	const auto pClientRegister = GetRegisterList(CLIENT_DLL);
	if (pClientRegister == nullptr)
		return false;

	Client = Capture<ISource2Client>(pClientRegister, SOURCE2_CLIENT);
	bSuccess &= (Client != nullptr);


	const auto pMaterialSystem2Register = GetRegisterList(MATERIAL_SYSTEM2_DLL);
	if (pMaterialSystem2Register == nullptr)
		return false;

	m_pMaterialSystem2 = Capture<IMaterialSystem2>(pMaterialSystem2Register, MATERIAL_SYSTEM2);
	bSuccess &= (m_pMaterialSystem2 != nullptr);

	const auto pResourceSystemRegisterList = GetRegisterList(RESOURCESYSTEM_DLL);
	if (pResourceSystemRegisterList == nullptr)
		return false;

	ResourceSystem = Capture<IResourceSystem>(pResourceSystemRegisterList, RESOURCE_SYSTEM);
	bSuccess &= (ResourceSystem != nullptr);

	if (ResourceSystem != nullptr)
	{
		ResourceHandleUtils = reinterpret_cast<CResourceHandleUtils*>(ResourceSystem->QueryInterface(RESOURCE_HANDLE_UTILS));
		bSuccess &= (ResourceHandleUtils != nullptr);
	}

#pragma endregion

	// @ida:  #STR: "r_gpu_mem_stats", "-threads", "CTSListBase: Misaligned list\n", "CTSQueue: Misaligned queue\n", "Display GPU memory usage.", "-r_max_device_threads"
	SwapChain = **reinterpret_cast<ISwapChainDx11***>(memory::ResolveRelativeAddress(memory::FindPattern(RENDERSYSTEM_DLL, XOR("48 89 2d ? ? ? ? 66 0f 7f 05")), 0x3, 0x7));
	bSuccess &= (SwapChain != nullptr);

	// grab's d3d11 interfaces for later use
	if (SwapChain != nullptr)
	{
		if (FAILED(SwapChain->pDXGISwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&Device)))
		{
			L_PRINT(LOG_ERROR) << XOR("failed to get device from swapchain");
			CS_ASSERT(false);
			return false;
		}
		else
			// we successfully got device, so we can get immediate context
			Device->GetImmediateContext(&DeviceContext);
	}
	bSuccess &= (Device != nullptr && DeviceContext != nullptr);

	// #STR: CSGOInput
	Input = *reinterpret_cast<CCSGOInput**>(memory::ResolveRelativeAddress(memory::FindPattern(CLIENT_DLL, XOR("48 8B 0D ? ? ? ? 4C 8B C6 8B 10 E8")), 0x3, 0x7));
	bSuccess &= (Input != nullptr);
	L_PRINT(LOG_INFO) << "found csgoinput interface";
	// @ida: #STR: "gpGlocals->rendertime() called while IsInSimulation() is true, "gpGlocals->curtime() called while IsInSimulation() is false
	// @ida: #STR: "C_SceneEntity::SetupClientOnlyScene:  C" then go up until you see it
	GlobalVars = *reinterpret_cast<IGlobalVars**>(memory::ResolveRelativeAddress(memory::FindPattern(CLIENT_DLL, XOR("48 89 15 ? ? ? ? 48 89 42")), 0x3, 0x7));
	bSuccess &= (GlobalVars != nullptr);
	L_PRINT(LOG_INFO) << "found globalvars interface";

	// @ida: #STR: "CRenderingWorldSession::OnLoopActivate" go down just a bit
	PVS = reinterpret_cast<CPVS*>(memory::ResolveRelativeAddress(memory::FindPattern(ENGINE2_DLL, XOR("48 8D 0D ? ? ? ? 33 ? FF 50")), 0x3, 0x7));
	bSuccess &= (PVS != nullptr);
	L_PRINT(LOG_INFO) << "found pvs interface";

	m_pClientMode = reinterpret_cast<IClientMode*>(memory::GetAbsoluteAddress(memory::FindPattern(CLIENT_DLL, XOR("48 8D 0D ? ? ? ? 48 69 C0 ? ? ? ? 48 03 C1 C3 CC CC")), 0x3, 0x0));
	bSuccess &= (m_pClientMode != nullptr);
	L_PRINT(LOG_INFO) << "found clientmode interface";

	//xref: str: GetViewRenderInstance() must be implemented by game.
	m_pViewRender = *reinterpret_cast<IViewRender**>(memory::GetAbsoluteAddress(memory::FindPattern(CLIENT_DLL, XOR("48 89 05 ? ? ? ? 48 8B C8 48 85 C0")), 0x3, 0x0));
	bSuccess &= (m_pViewRender != nullptr);
	L_PRINT(LOG_INFO) << "found m_pViewRender interface";

	GameEntitySystem = *reinterpret_cast<CGameEntitySystem**>(memory::GetAbsoluteAddress(memory::FindPattern(CLIENT_DLL, XOR("48 8B 0D ? ? ? ? 48 89 7C 24 ? 8B FA C1 EB")), 0x3));
	bSuccess &= (GameEntitySystem != nullptr);
	L_PRINT(LOG_INFO) << "found GameEntitySystem interface";

	// @ida: #STR: "Physics/TraceShape (Client)"
	// @ida: #STR: "Weapon_Knife.Stab" then go up
	GameTraceManager = *reinterpret_cast<CGameTraceManager**>(memory::GetAbsoluteAddress(memory::FindPattern(CLIENT_DLL, XOR("48 8B 1D ? ? ? ? 24 ? 0C ?")), 0x3, 0x0)); //48 8B 1D ? ? ? ? 24 ? 0C ?
	bSuccess &= (GameTraceManager != nullptr);
	L_PRINT(LOG_INFO) << "found GameTraceManager interface";

	return bSuccess;
}

void Interface::CreateRenderTarget()
{
	if (FAILED(SwapChain->pDXGISwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&Device)))
	{
		L_PRINT(LOG_ERROR) << XOR("failed to get device from swapchain");
		CS_ASSERT(false);
	}
	else
		// we successfully got device, so we can get immediate context
		Device->GetImmediateContext(&DeviceContext);

	// @note: i dont use this anywhere else so lambda is fine
	static const auto GetCorrectDXGIFormat = [](DXGI_FORMAT eCurrentFormat)
		{
			switch (eCurrentFormat)
			{
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			}

			return eCurrentFormat;
		};

	DXGI_SWAP_CHAIN_DESC sd;
	SwapChain->pDXGISwapChain->GetDesc(&sd);

	ID3D11Texture2D* pBackBuffer = nullptr;
	if (SUCCEEDED(SwapChain->pDXGISwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))))
	{
		if (pBackBuffer)
		{
			D3D11_RENDER_TARGET_VIEW_DESC desc{};
			desc.Format = static_cast<DXGI_FORMAT>(GetCorrectDXGIFormat(sd.BufferDesc.Format));
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			if (FAILED(Device->CreateRenderTargetView(pBackBuffer, &desc, &RenderTargetView)))
			{
				L_PRINT(LOG_WARNING) << XOR("failed to create render target view with D3D11_RTV_DIMENSION_TEXTURE2D...");
				L_PRINT(LOG_INFO) << XOR("retrying to create render target view with D3D11_RTV_DIMENSION_TEXTURE2DMS...");
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
				if (FAILED(Device->CreateRenderTargetView(pBackBuffer, &desc, &RenderTargetView)))
				{
					L_PRINT(LOG_WARNING) << XOR("failed to create render target view with D3D11_RTV_DIMENSION_TEXTURE2D...");
					L_PRINT(LOG_INFO) << XOR("retrying...");
					if (FAILED(Device->CreateRenderTargetView(pBackBuffer, NULL, &RenderTargetView)))
					{
						L_PRINT(LOG_ERROR) << XOR("failed to create render target view");
						CS_ASSERT(false);
					}
				}
			}
			pBackBuffer->Release();
			pBackBuffer = nullptr;
		}
	}
}

void Interface::DestroyRenderTarget()
{
	if (RenderTargetView != nullptr)
	{
		RenderTargetView->Release();
		RenderTargetView = nullptr;
	}
}
