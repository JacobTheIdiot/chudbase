#include "pch.h"
#include <vector>
#include <algorithm>

#include "convars.h"
#include "interface/interfaces.h"
#include "interface/interfaces/ienginecvar.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#include "core/util/log.h"

//credit: asphyxia for most of it

inline static void WriteConVarType(HANDLE hFile, const uint32_t nType)
{
	switch ((EConVarType)nType)
	{
	case EConVarType_Bool:
		::WriteFile(hFile, XOR("[bool] "), CRT::StringLength(XOR("[bool] ")), nullptr, nullptr);
		break;
	case EConVarType_Int16:
		::WriteFile(hFile, XOR("[int16] "), CRT::StringLength(XOR("[int16] ")), nullptr, nullptr);
		break;
	case EConVarType_UInt16:
		::WriteFile(hFile, XOR("[uint16] "), CRT::StringLength(XOR("[uint16] ")), nullptr, nullptr);
		break;
	case EConVarType_Int32:
		::WriteFile(hFile, XOR("[int32] "), CRT::StringLength(XOR("[int32] ")), nullptr, nullptr);
		break;
	case EConVarType_UInt32:
		::WriteFile(hFile, XOR("[uint32] "), CRT::StringLength(XOR("[uint32] ")), nullptr, nullptr);
		break;
	case EConVarType_Int64:
		::WriteFile(hFile, XOR("[int64] "), CRT::StringLength(XOR("[int64] ")), nullptr, nullptr);
		break;
	case EConVarType_UInt64:
		::WriteFile(hFile, XOR("[uint64] "), CRT::StringLength(XOR("[uint64] ")), nullptr, nullptr);
		break;
	case EConVarType_Float32:
		::WriteFile(hFile, XOR("[float32] "), CRT::StringLength(XOR("[float32] ")), nullptr, nullptr);
		break;
	case EConVarType_Float64:
		::WriteFile(hFile, XOR("[float64] "), CRT::StringLength(XOR("[float64] ")), nullptr, nullptr);
		break;
	case EConVarType_String:
		::WriteFile(hFile, XOR("[string] "), CRT::StringLength(XOR("[string] ")), nullptr, nullptr);
		break;
	case EConVarType_Color:
		::WriteFile(hFile, XOR("[color] "), CRT::StringLength(XOR("[color] ")), nullptr, nullptr);
		break;
	case EConVarType_Vector2:
		::WriteFile(hFile, XOR("[vector2] "), CRT::StringLength(XOR("[vector2] ")), nullptr, nullptr);
		break;
	case EConVarType_Vector3:
		::WriteFile(hFile, XOR("[vector3] "), CRT::StringLength(XOR("[vector3] ")), nullptr, nullptr);
		break;
	case EConVarType_Vector4:
		::WriteFile(hFile, XOR("[vector4] "), CRT::StringLength(XOR("[vector4] ")), nullptr, nullptr);
		break;
	case EConVarType_Qangle:
		::WriteFile(hFile, XOR("[qangle] "), CRT::StringLength(XOR("[qangle] ")), nullptr, nullptr);
		break;
	default:
		::WriteFile(hFile, XOR("[unknown-type] "), CRT::StringLength(XOR("[unknown-type] ")), nullptr, nullptr);
		break;
	}
}

inline static void WriteConVarFlags(HANDLE hFile, const uint32_t nFlags)
{
	if (nFlags & FCVAR_CLIENTDLL)
		::WriteFile(hFile, XOR("[client.dll] "), CRT::StringLength(XOR("[client.dll] ")), nullptr, nullptr);
	else if (nFlags & FCVAR_GAMEDLL)
		::WriteFile(hFile, XOR("[games's dll] "), CRT::StringLength(XOR("[games's dll] ")), nullptr, nullptr);

	if (nFlags & FCVAR_PROTECTED)
		::WriteFile(hFile, XOR("[protected] "), CRT::StringLength(XOR("[protected] ")), nullptr, nullptr);

	if (nFlags & FCVAR_CHEAT)
		::WriteFile(hFile, XOR("[cheat] "), CRT::StringLength(XOR("[cheat] ")), nullptr, nullptr);

	if (nFlags & FCVAR_HIDDEN)
		::WriteFile(hFile, XOR("[hidden] "), CRT::StringLength(XOR("[hidden] ")), nullptr, nullptr);

	if (nFlags & FCVAR_DEVELOPMENTONLY)
		::WriteFile(hFile, XOR("[devonly] "), CRT::StringLength(XOR("[devonly] ")), nullptr, nullptr);

	::WriteFile(hFile, XOR("\n"), CRT::StringLength(XOR("\n")), nullptr, nullptr);
}

bool convar::dump(const wchar_t* wszFileName)
{
	/*wchar_t wszDumpFilePath[MAX_PATH];
	if (!FileUtils::GetWorkingPath(wszDumpFilePath))
		return false;

	CRT::StringCat(wszDumpFilePath, wszFileName);

	HANDLE hOutFile = ::CreateFileW(wszDumpFilePath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hOutFile == INVALID_HANDLE_VALUE)
		return false;

	// @todo: maybe remove this redundant? and put it inside CRT::String_t c'tor
	const std::time_t time = std::time(nullptr);
	std::tm timePoint;
	localtime_s(&timePoint, &time);

	CRT::String_t<64> szTimeBuffer(CS_XOR("[%d-%m-%Y %T] asphyxia | convars dump\n\n"), &timePoint);

	// write current date, time and info
	::WriteFile(hOutFile, szTimeBuffer.Data(), szTimeBuffer.Length(), nullptr, nullptr);

	for (int i = I::Cvar->listConvars.Head(); i != I::Cvar->listConvars.InvalidIndex(); i = I::Cvar->listConvars.Next(i))
	{
		CConVar* pConVar = I::Cvar->listConvars.Element(i);
		if (pConVar != nullptr)
		{
			// dump to file
			WriteConVarType(hOutFile, pConVar->nType);

			CRT::String_t<526> szBuffer(CS_XOR("%s : \"%s\" "), pConVar->szName, pConVar->szDescription[0] == '\0' ? CS_XOR("no description") : pConVar->szDescription);
			::WriteFile(hOutFile, szBuffer.Data(), szBuffer.Length(), nullptr, nullptr);

			// write flags
			WriteConVarFlags(hOutFile, pConVar->nFlags);
		}
	}

	::CloseHandle(hOutFile);

	return true;

	*/
	return true;
}

bool convar::setup()
{
	bool bSuccess = true;
	L_PRINT(LOG_NONE) << L::SetColor(LOG_COLOR_FORE_CYAN | LOG_COLOR_FORE_INTENSITY) << XOR("checkpoint1");
	m_pitch = Interface::Cvar->Find(FNV1A::HashConst("m_pitch"));
	bSuccess &= m_pitch != nullptr;
	L_PRINT(LOG_NONE) << L::SetColor(LOG_COLOR_FORE_CYAN | LOG_COLOR_FORE_INTENSITY) << XOR("checkpoint2");
	m_yaw = Interface::Cvar->Find(FNV1A::HashConst("m_yaw"));
	bSuccess &= m_yaw != nullptr;
	L_PRINT(LOG_NONE) << L::SetColor(LOG_COLOR_FORE_CYAN | LOG_COLOR_FORE_INTENSITY) << XOR("checkpoint3");
	sensitivity = Interface::Cvar->Find(FNV1A::HashConst("sensitivity"));
	bSuccess &= sensitivity != nullptr;
	L_PRINT(LOG_NONE) << L::SetColor(LOG_COLOR_FORE_CYAN | LOG_COLOR_FORE_INTENSITY) << XOR("checkpoint4");
	game_type = Interface::Cvar->Find(FNV1A::HashConst("game_type"));
	bSuccess &= game_type != nullptr;
	L_PRINT(LOG_NONE) << L::SetColor(LOG_COLOR_FORE_CYAN | LOG_COLOR_FORE_INTENSITY) << XOR("checkpoint5");
	game_mode = Interface::Cvar->Find(FNV1A::HashConst("game_mode"));
	bSuccess &= game_mode != nullptr;
	L_PRINT(LOG_NONE) << L::SetColor(LOG_COLOR_FORE_CYAN | LOG_COLOR_FORE_INTENSITY) << XOR("checkpoint6");
	mp_teammates_are_enemies = Interface::Cvar->Find(FNV1A::HashConst("mp_teammates_are_enemies"));
	bSuccess &= mp_teammates_are_enemies != nullptr;
	L_PRINT(LOG_NONE) << L::SetColor(LOG_COLOR_FORE_CYAN | LOG_COLOR_FORE_INTENSITY) << XOR("checkpoint7");
	sv_autobunnyhopping = Interface::Cvar->Find(FNV1A::HashConst("sv_autobunnyhopping"));
	bSuccess &= sv_autobunnyhopping != nullptr;

	L_PRINT(LOG_NONE) << L::SetColor(LOG_COLOR_FORE_CYAN | LOG_COLOR_FORE_INTENSITY) << XOR("checkpoint8");
	return bSuccess;
}
