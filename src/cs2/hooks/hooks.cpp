#include "pch.h"
#include "hooks.h"
#include "minhook/include/MinHook.h"
#include "core/mem/memory.h"
#include "core/util/log.h"
#include "core/util/fnv1a.h"
#include "cheat/gui/gui.h"
#define WIN32_LEAN_AND_MEAN

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
bool hooks::setup()
{
	if (MH_Initialize() != MH_OK)
		return false;

	//client.dll
	SubscribeHook(detours::Present.Init(&hkPresent, memory::GetVFunc(Interface::SwapChain->pDXGISwapChain, 8U)));
	SubscribeHook(detours::ResizeBuffers.Init(&hkResizeBuffers, memory::GetVFunc(Interface::SwapChain->pDXGISwapChain, VTABLE::D3D::RESIZEBUFFERS)));
	SubscribeHook(detours::RelativeMouseMode.Init(&hkRelativeMouseMode, Interface::InputSystem, 76));
	SubscribeHook(detours::OnAddEntity.Init(&hkOnAddEntity, memory::FindPattern(CLIENT_DLL, ON_ADD_ENTITY_PATTERN)));
	SubscribeHook(detours::OnRemoveEntity.Init(&hkOnRemoveEntity, memory::FindPattern(CLIENT_DLL, ON_REMOVE_ENTITY_PATTERN)));
	SubscribeHook(detours::OnRenderStart.Init(&hkOnRenderStart, Interface::m_pViewRender, 4));
	SubscribeHook(detours::OverrideView.Init(&hkOverrideView, memory::FindPattern(CLIENT_DLL, OVERRIDE_VIEW_PATTERN)));



	//  swapchain hook
	IDXGIDevice* pDXGIDevice = NULL;
	Interface::Device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice));

	IDXGIAdapter* pDXGIAdapter = NULL;
	pDXGIDevice->GetAdapter(&pDXGIAdapter);

	IDXGIFactory* pIDXGIFactory = NULL;
	pDXGIAdapter->GetParent(IID_PPV_ARGS(&pIDXGIFactory));

	SubscribeHook(detours::CreateSwapChain.Init(&hkCreateSwapChain, memory::GetVFunc(Interface::SwapChain->pDXGISwapChain, VTABLE::DXGI::CREATESWAPCHAIN)));
	pDXGIDevice->Release();
	pDXGIDevice = nullptr;
	pDXGIAdapter->Release();
	pDXGIAdapter = nullptr;
	pIDXGIFactory->Release();
	pIDXGIFactory = nullptr;



	MH_EnableHook(MH_ALL_HOOKS);
	
	return true;
}

void hooks::shutdown()
{
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);
	MH_Uninitialize();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
long hooks::hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	inputsystem::OnWndProc(hWnd, uMsg, wParam, lParam);
	ClipCursor(nullptr);

	if (gui::m_bOpen)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

		const auto oRelativeMouseMode = detours::RelativeMouseMode.GetOriginal< decltype(&hkRelativeMouseMode) >();
		if (oRelativeMouseMode)
			oRelativeMouseMode(Interface::InputSystem, gui::m_bOpen ? false : gui::m_bInput);

		return 1L;
	}

	return ::CallWindowProcW(inputsystem::pOldWndProc, hWnd, uMsg, wParam, lParam);
}
HRESULT CS_FASTCALL hooks::hkResizeBuffers(IDXGISwapChain* pSwapChain, std::uint32_t nBufferCount, std::uint32_t nWidth, std::uint32_t nHeight, DXGI_FORMAT newFormat, std::uint32_t nFlags)
{
	const auto oResizeBuffer = detours::ResizeBuffers.GetOriginal< decltype(&hkResizeBuffers) >();
	HRESULT hResult = oResizeBuffer(pSwapChain, nBufferCount, nWidth, nHeight, newFormat, nFlags); //ReturnAddressSpoofer::SpoofCall(ReturnAddressSpoofGadgets::m_pGameOverlayGadget, oResizeBuffer, pSwapChain, nBufferCount, nWidth, nHeight, newFormat, nFlags);
	if (SUCCEEDED(hResult))
	{
		Interface::CreateRenderTarget();
		//ImGui_ImplDX11_CreateDeviceObjects();
	}


	return hResult;
}
HRESULT WINAPI hooks::hkCreateSwapChain(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	const auto oCreateSwapChain = detours::CreateSwapChain.GetOriginal< decltype(&hkCreateSwapChain) >();

	Interface::DestroyRenderTarget();
	L_PRINT(LOG_INFO) << XOR("render target view has been destroyed");

	return oCreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
}

void* __fastcall hooks::hkRelativeMouseMode(void* pThis, bool bActive)
{
	const auto oRelativeMouseMode = detours::RelativeMouseMode.GetOriginal< decltype(&hkRelativeMouseMode) >();

	gui::m_bInput = bActive;
	return oRelativeMouseMode(pThis, gui::m_bOpen ? false : bActive);
}

HRESULT WINAPI hooks::hkPresent(IDXGISwapChain* pSwapChain, UINT uSyncInterval, UINT uFlags)
{
	const auto oPresent = detours::Present.GetOriginal<decltype(&hkPresent)>();

	if (Interface::RenderTargetView == nullptr)
		Interface::CreateRenderTarget();

	if (Interface::RenderTargetView != nullptr)
		Interface::DeviceContext->OMSetRenderTargets(1, &Interface::RenderTargetView, nullptr);


	static bool initialized = false;
	if (!initialized)
	{
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(inputsystem::hWindow);
		ImGui_ImplDX11_Init(Interface::Device, Interface::DeviceContext);

		initialized = true;
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//draw here
	gui::drawgui();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return oPresent(pSwapChain, uSyncInterval, uFlags);
}

void* __fastcall hooks::hkOnAddEntity(void* pThis, CEntityInstance* pInstance, CBaseHandle uHandle)
{
	const auto oOnAddEntity = detours::OnAddEntity.GetOriginal< decltype(&hkOnAddEntity) >();
	//g_Entities->OnAddEntity(pInstance, uHandle);
	return oOnAddEntity(pThis, pInstance, uHandle);
}

void* __fastcall hooks::hkOnRemoveEntity(void* pThis, CEntityInstance* pInstance, CBaseHandle uHandle)
{
	const auto oOnRemoveEntity = detours::OnRemoveEntity.GetOriginal< decltype(&hkOnRemoveEntity) >();
	//g_Entities->OnRemoveEntity(pInstance, uHandle);
	return oOnRemoveEntity(pThis, pInstance, uHandle);
}
void __fastcall hooks::hkOverrideView(void* a1, CViewSetup* pSetup)
{
	const auto oOverrideView = detours::OverrideView.GetOriginal< decltype(&hkOverrideView) >();

	return oOverrideView(a1, pSetup);
}

void __fastcall hooks::hkOnRenderStart(CViewRender* pViewRender)
{
	const auto oOnRenderStart = detours::OnRenderStart.GetOriginal<decltype(&hkOnRenderStart)>();

	oOnRenderStart(pViewRender);
	
	/*
	* you're gonna wanna do smth like this:
	* return features::onRenderStart(pViewRender); 
	* but since i haven't made that in this base we'll just return the og
	*/
	return oOnRenderStart(pViewRender);

}
