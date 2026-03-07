#pragma once

#include "cs2/datatypes/vector.h"
#include "cs2/datatypes/color.h"

enum class EKeyBindMode : int
{
	HOLD = 0,
	TOGGLE
};
struct KeyBind_t
{
	constexpr KeyBind_t(const char* szName, const unsigned int uKey = 0U, const EKeyBindMode nMode = EKeyBindMode::HOLD) :
		szName(szName), uKey(uKey), nMode(nMode) {
	}

	bool m_bEnable = false;
	const char* szName = nullptr;
	unsigned int uKey = 0U;
	EKeyBindMode nMode = EKeyBindMode::HOLD;
};

namespace config
{
	namespace bools
	{

	}

	namespace ints
	{

	}
	namespace floats
	{

	}

	namespace color
	{

	}

	//idk whartevwer else i wanna add
}