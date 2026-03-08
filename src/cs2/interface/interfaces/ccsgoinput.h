#pragma once

// used: mem_pad
#include "core/mem/memory.h"

// used: cusercmd
#include "cs2/datatypes/usercmd.h"

#define MULTIPLAYER_BACKUP 150

class CCSInputMessage
{
public:
	std::int32_t m_nFrameTickCount; //0x0000
	float m_flFrameTickFraction; //0x0004
	std::int32_t m_nPlayerTickCount; //0x0008
	float m_flPlayerTickFraction; //0x000C
	QAngle_t m_angViewAngles; //0x0010
	Vector_t m_vecShootPosition; //0x001C 
	std::int32_t m_nTargetIndex; //0x0028
	Vector_t m_vecTargetHeadPosition; //0x002C What's this???
	Vector_t m_vecTargetAbsOrigin; //0x0038
	Vector_t m_vecTargetAngle; //0x0044
	std::int32_t m_nsvShowHitRegistration; //0x0050
	std::int32_t m_nEntryIndexMax; //0x0054
	std::int32_t m_nIndexUnk; //0x0058
	int idekjustpadding;
};
static_assert(sizeof(CCSInputMessage) == 0x60);


class CTinyMoveStepData
{
public:
	float flWhen; //0x0000
	MEM_PAD(0x4); //0x0004
	std::uint64_t nButton; //0x0008
	bool bPressed; //0x0010
	MEM_PAD(0x7); //0x0011
}; //Size: 0x0018

class CSubtickInput {
public:
	float m_flWhen;
	float m_flDelta;
	uint64_t m_nButton;
	bool m_bPressed;
	char pad_0011[3];
	int m_nMouseInput;
	Vector_t angles;
};
class CButtonData
{
public:
	std::uint64_t flag;
	char* name;
	std::byte m_pad0[0x8];
	float value;
	std::byte m_pad1[0x4];
	float when_start;
	float when_end;
	std::byte m_pad2[0x4];
	int command_count;
	std::byte m_pad3[0x60];
}; //Size: 0x0048

class CMoveStepButtons
{
public:
	std::uint64_t nKeyboardPressed; //0x0000
	std::uint64_t nMouseWheelheelPressed; //0x0008
	std::uint64_t nUnPressed; //0x0010
	std::uint64_t nKeyboardCopy; //0x0018
}; //Size: 0x0020

// @credits: www.unknowncheats.me/forum/members/2943409.html
class CExtendedMoveData : public CMoveStepButtons
{
public:
	float flForwardMove; //0x0020
	float flSideMove; //0x0024
	float flUpMove; //0x0028
	std::int32_t nMouseDeltaX; //0x002C
	std::int32_t nMouseDeltaY; //0x0030
	std::int32_t nAdditionalStepMovesCount; //0x0034
	CTinyMoveStepData tinyMoveStepData[12]; //0x0038
	Vector_t vecViewAngle; //0x0158
	std::int32_t nTargetHandle; //0x0164
}; //Size:0x0168

class CCSGOInput
{
public:
	char pad_0000[16]; //0x0000
	CButtonData* buttons_data[63]; //0x0010
	char pad_0208[72]; //0x0208
	bool bBlockShot;
	bool m_bInThirdPerson;
	MEM_PAD(0x6);
	QAngle_t angThirdPersonAngles;
	MEM_PAD(0x14);
	uint64_t nKeyboardPressed;
	uint64_t nMouseWheelPressed;
	uint64_t nUnPressed;
	uint64_t nKeyboardCopy;
	float flForwardMove;
	float flSideMove;
	float flUpMove;
	Vector2D_t nMousePos;
	int32_t SubticksCount;
	CSubtickInput Subticks[32];
	Vector_t vecViewAngle;
	int32_t nTargetHandle;
	MEM_PAD(0x230);
	int32_t nAttackStartHistoryIndex1;
	int32_t nAttackStartHistoryIndex2;
	int32_t nAttackStartHistoryIndex3;
	char pad_061C[4]; //0x061C
	int32_t m_nMessageSize; //0x0620
	char pad_0624[6]; //0x0624
	CCSInputMessage* m_pMessage; //0x0630

	CUserCmd* GetUserCmd()
	{
		typedef uintptr_t(__fastcall* GetLocalControllerByInternalIdFn)(int);
		static GetLocalControllerByInternalIdFn GetLocalControllerByInternalId = (GetLocalControllerByInternalIdFn)memory::FindPattern(CLIENT_DLL, "48 83 ec ?? 83 f9 ?? 75 ?? 48 8b 0d ?? ?? ?? ?? 48 8d 54 24 ?? 48 8b 01 ff 90 ?? ?? ?? ?? 8b 08 48 63 c1 48 8d 0d ?? ?? ?? ?? 48 8b 04 c1 48 83 c4 ?? c3 cc cc cc cc cc cc cc cc cc cc cc cc cc 48 83 ec ?? 83 f9");
		typedef uintptr_t(__fastcall* SetupCmdFn)(uintptr_t);
		static SetupCmdFn SetupCmd = (SetupCmdFn)memory::FindPattern(CLIENT_DLL, "48 83 EC 28 E8 ?? ?? ?? ?? 8B 80");
		typedef uintptr_t(__fastcall* GetControllerCmdFn)(uintptr_t, uintptr_t);
		static GetControllerCmdFn GetControllerCmd = (GetControllerCmdFn)memory::FindPattern(CLIENT_DLL, "40 53 48 83 EC 20 8B DA E8 ?? ?? ?? ?? 4C");

		auto Controller = GetLocalControllerByInternalId(0);
		return (CUserCmd*)GetControllerCmd(Controller, SetupCmd(Controller));
	}

	void SetViewAngle(QAngle_t& angView)
	{
		// xref: str: "bot_takeover" -> the only function in the if statement without a str in the params
		using fnSetViewAngle = std::int64_t(CS_FASTCALL*)(void*, std::int32_t, QAngle_t&);
		static auto oSetViewAngle = reinterpret_cast<fnSetViewAngle>(memory::FindPattern(CLIENT_DLL, XOR("85 D2 75 ? 48 63 81")));

		#ifdef CS_PARANOID
		CS_ASSERT(oSetViewAngle != nullptr);
		#endif

		oSetViewAngle(this, 0, std::ref(angView));
	}

	QAngle_t GetViewAngles()
	{
		using fnGetViewAngles = std::int64_t(CS_FASTCALL*)(CCSGOInput*, std::int32_t);
		static auto oGetViewAngles = reinterpret_cast<fnGetViewAngles>(memory::FindPattern(CLIENT_DLL, XOR("4C 8B C1 85 D2 74 08 48 8D 05 ? ? ? ? C3")));

		#ifdef CS_PARANOID
		CS_ASSERT(oGetViewAngles != nullptr);
		#endif

		return *reinterpret_cast<QAngle_t*>(oGetViewAngles(this, 0));
	}
};
