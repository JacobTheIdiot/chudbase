#pragma once
#include "stb_sprintf.h"
class CConVar;

namespace convar
{
	// dump convars to file (i'll do this later)
	bool dump(const wchar_t* wszFileName);
	bool setup();

	inline CConVar* m_pitch = nullptr;
	inline CConVar* m_yaw = nullptr;
	inline CConVar* sensitivity = nullptr;

	inline CConVar* game_type = nullptr;
	inline CConVar* game_mode = nullptr;

	inline CConVar* mp_teammates_are_enemies = nullptr;

	inline CConVar* sv_autobunnyhopping = nullptr;
}
