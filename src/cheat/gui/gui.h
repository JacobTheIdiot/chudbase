#pragma once
#include "imgui/imgui.h"
#include <windows.h>

namespace gui
{
	inline bool m_bOpen=false;
	inline bool m_bInput = false;
	constexpr int m_iBind = VK_INSERT;

	void drawgui();
}