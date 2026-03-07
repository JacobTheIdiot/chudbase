#pragma once
#include "core/mem/memory.h"
#include "cs2/datatypes/vector.h"
#include "cs2/datatypes/qangle.h"
class CViewSetup
{
	char _pad0[0x450]; //maybe 490, not sure yet
public:
	float m_flOrthoLeft; // 0x0494
	float m_flOrthoTop; // 0x0498
	float m_flOrthoRight; // 0x049C
	float m_flOrthoBottom; // 0x04A0
private: char _pad1[0x38]; public:
	float m_flFov; // 0x04D8
	float m_flFovViewmodel; // 0x04DC
	Vector_t m_vecOrigin; // 0x04E0
private: char _pad2[0xC]; public:
	QAngle_t m_angView; // 0x04F8
private: char _pad3[0x14]; public:
	float m_flAspectRatio; // 0x0518
private: char _pad4[0x79]; public:
	BYTE m_nSomeFlags; // 0x0595
};

struct CViewRender
{
	MEM_PAD(0x8); // 0x0000
	CViewSetup m_viewSetup; // 0x0008

	virtual void Function0();
};

class IClientModeShared
{
public:
};