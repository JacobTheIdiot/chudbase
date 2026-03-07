#pragma once
#include "core/util/log.h"
#include "core/mem/memory.h"
#include "pch.h"
#include "minhook/include/MinHook.h"
#include <Windows.h>

//@credit: https://github.com/TsudaKageyu/minhook

class CDetourHook
{
public:
	CDetourHook() = default;
	CDetourHook(std::string szName)
		: strName(szName) {
	}

	explicit CDetourHook(void* pFunction, void* pDetour)
		: pBaseFn(pFunction), pReplaceFn(pDetour) {
	}

	CDetourHook& Init(void* pDetour, void* pInterface, const size_t nIndex)
	{
		if (bInit)
			return *this;

		pBaseFn = memory::GetVFunc(pInterface, nIndex);
		pReplaceFn = pDetour;

		if (pBaseFn == nullptr || pReplaceFn == nullptr)
		{

			L_PRINT(LOG_WARNING) << XOR("[warning] failed to get virtual function: ") << strName.c_str();

			bInit = false;
		}
		else bInit = true;
		return *this;
	}

	CDetourHook& Init(void* pDetour, void* pBase)
	{
		if (bInit)
			return *this;

		pBaseFn = pBase;
		pReplaceFn = pDetour;

		if (pBaseFn == nullptr || pReplaceFn == nullptr)
		{
			L_PRINT(LOG_WARNING) << ("[warning] failed to get virtual function at base addr -> {}\n"), reinterpret_cast<std::uintptr_t>(pBaseFn);

			bInit = false;
		}
		else bInit = true;
		return *this;
	}

	void Create()
	{
		if (!bInit || !bCanCreate || bIsHooked)
			return;

		if (pBaseFn == nullptr || pReplaceFn == nullptr)
			return;

		if (const MH_STATUS status = MH_CreateHook(pBaseFn, pReplaceFn, &pOriginalFn); status != MH_OK)
		{
			const char* szStatus = MH_StatusToString(status);
			uintptr_t pBase = reinterpret_cast<uintptr_t>(pBaseFn);
			L_PRINT(LOG_ERROR) << XOR("[error] failed to create hook function, status: ") << szStatus;
		}

		if (!this->Replace())
			return;

		L_PRINT(LOG_INFO) << XOR("[+] hooked ") << strName.c_str();
	}

	bool Replace()
	{
		if (pBaseFn == nullptr)
			return false;

		if (bIsHooked)
			return false;

		if (const MH_STATUS status = MH_EnableHook(pBaseFn); status != MH_OK)
		{
			const char* szStatus = MH_StatusToString(status);
			uintptr_t pBase = reinterpret_cast<uintptr_t>(pBaseFn);
			L_PRINT(LOG_ERROR) << XOR("[error] failed to create hook function, status: ") << szStatus;
		}

		bIsHooked = true;
		return true;
	}

	bool Remove()
	{
		if (!this->Restore())
			return false;

		if (const MH_STATUS status = MH_RemoveHook(pBaseFn); status != MH_OK)
		{
			const char* szStatus = MH_StatusToString(status);
			uintptr_t pBase = reinterpret_cast<uintptr_t>(pBaseFn);
			L_PRINT(LOG_ERROR) << XOR("[error] failed to create hook function, status: ") << szStatus;
		}

		return true;
	}

	bool Restore()
	{
		if (!bIsHooked)
			return false;

		if (const MH_STATUS status = MH_DisableHook(pBaseFn); status != MH_OK)
		{
			const char* szStatus = MH_StatusToString(status);
			uintptr_t pBase = reinterpret_cast<uintptr_t>(pBaseFn);
			L_PRINT(LOG_ERROR) << XOR("[error] failed to create hook function, status: ") << szStatus;
		}

		bIsHooked = false;
		return true;
	}

	template <typename Fn>
	Fn GetOriginal()
	{
		return static_cast<Fn>(pOriginalFn);
	}

	void* pBaseFn = nullptr;
	void* pReplaceFn = nullptr;
	void* pOriginalFn = nullptr;

	bool bInit = false;
	bool bIsHooked = false;
	bool bCanCreate = true;

	std::string strName = "";
};