#pragma once
#include "pch.h"
// used: schema field
#include "cs2/schema/schema.h"
// used: rop
#include "core/mem/memory.h"


using CFiringModeFloat = std::float_t[2];
using CSkillFloat = std::float_t[4];

class CBasePlayerVData
{
public:
	CS_CLASS_NO_INITIALIZER(CBasePlayerVData);

	//CResourceNameTyped< CWeakHandle< InfoForResourceTypeCModel > > m_sModelName = 0x28
	//CSkillFloat m_flHeadDamageMultiplier = 0x108
	//CSkillFloat m_flChestDamageMultiplier = 0x118
	//CSkillFloat m_flStomachDamageMultiplier = 0x128
	//CSkillFloat m_flArmDamageMultiplier = 0x138
	//CSkillFloat m_flLegDamageMultiplier = 0x148
	//float32 m_flHoldBreathTime = 0x158
	//float32 m_flDrowningDamageInterval = 0x15C
	//int32 m_nDrowningDamageInitial = 0x160
	//int32 m_nDrowningDamageMax = 0x164
	//int32 m_nWaterSpeed = 0x168
	//float32 m_flUseRange = 0x16C
	//float32 m_flUseAngleTolerance = 0x170
	//float32 m_flCrouchTime = 0x174
};
class CEntitySubClassVDataBase
{
public:
	template <typename T> requires std::derived_from<T, CEntitySubClassVDataBase>
	inline T* As()
	{
		return (T*)this;
	}
};
class CBasePlayerWeaponVData
{
public:
	CS_CLASS_NO_INITIALIZER(CBasePlayerWeaponVData);

	SCHEMA_ADD_FIELD(bool, IsFullAuto, "CBasePlayerWeaponVData->m_bIsFullAuto");
	SCHEMA_ADD_FIELD(std::int32_t, GetMaxClip1, "CBasePlayerWeaponVData->m_iMaxClip1");
};

class CCSWeaponBaseVData : public CBasePlayerWeaponVData
{
public:
	CS_CLASS_NO_INITIALIZER(CCSWeaponBaseVData);

	SCHEMA_ADD_FIELD_OFFSET(CEntitySubClassVDataBase*, m_pVDataBase, "C_BaseEntity->m_nSubclassID", 0x8);

	SCHEMA_ADD_FIELD(std::int32_t, GetWeaponType, "CCSWeaponBaseVData->m_WeaponType");
	SCHEMA_ADD_FIELD(float, GetRange, "CCSWeaponBaseVData->m_flRange");
};
