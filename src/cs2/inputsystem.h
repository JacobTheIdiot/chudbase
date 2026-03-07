#pragma once
// used: [win] winapi
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "pch.h"
#include "imgui/imgui.h"
#include "cheat/config/config.h"
#include <cstdint>

//@credit: asphyxia  
namespace inputsystem
{
	using KeyState_t = std::uint8_t;

	enum EKeyState : KeyState_t
	{
		KEY_STATE_NONE,
		KEY_STATE_DOWN,
		KEY_STATE_UP,
		KEY_STATE_RELEASED
	};

	inline HWND hWindow = nullptr;
	inline WNDPROC pOldWndProc = nullptr;
	inline KeyState_t arrKeyState[256] = {};

	bool setup();
	void destroy();

	bool OnWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	bool GetBindState(KeyBind_t& keyBind);

	[[nodiscard]] bool IsHovered(const ImVec2& vecPosition, const ImVec2& vecSize);

	[[nodiscard]] CS_INLINE bool IsKeyDown(const std::uint32_t uButtonCode)
	{
		return arrKeyState[uButtonCode] == KEY_STATE_DOWN;
	}

	[[nodiscard]] CS_INLINE bool IsKeyReleased(const std::uint32_t uButtonCode)
	{
		if (arrKeyState[uButtonCode] == KEY_STATE_RELEASED)
		{
			arrKeyState[uButtonCode] = KEY_STATE_UP;
			return true;
		}

		return false;
	}
}