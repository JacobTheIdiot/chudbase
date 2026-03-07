#pragma once
#include <algorithm>
#include "cs2/interface/Interfaces.h"
#include "cs2/interface/interfaces/igameresourceservice.h"
#include "cs2/interface/interfaces/ischemasystem.h"
#include "cs2/schema/schema.h"
#include "core/util/log.h"
#include "cs2/datatypes/vector.h"
#include "cs2/datatypes/qangle.h"
#include "cs2/datatypes/transform.h"

#include "entityhandle.h"
#include "../enums.h"
#include "vdata.h"

#include "core/math/math.h"
#include "cs2/datatypes/vector.h"
#include "core/mem/datatypes/CStrongHandle.h"
#include "core/mem/datatypes/CUtlSymbolLarge.h"

//@credit: asphyxia and my cheat
using GameTime_t = std::float_t;
using GameTick_t = std::int32_t;

struct BoneData_t
{
	Vector_t vecPosition;
	float flScale;
	Vector4D_t vecRotation;
};

// forward decleration
class C_CSWeaponBaseGun;
class C_BasePlayerWeapon;
class C_CSWeaponBase;
class CEntityInstance;
class C_CSPlayerPawn;

class CEntityIdentity
{
public:
	// @note: handle index is not entity index
	SCHEMA_ADD_OFFSET(std::uint32_t, nIndex, 0x10);
	SCHEMA_ADD_FIELD(const char*, m_designerName, "CEntityIdentity->m_designerName");
	SCHEMA_ADD_FIELD(std::uint32_t, m_flags, "CEntityIdentity->m_flags");

	[[nodiscard]] bool IsValid()
	{
		return nIndex() != INVALID_EHANDLE_INDEX;
	}

	[[nodiscard]] int GetEntryIndex()
	{
		if (!IsValid())
			return ENT_ENTRY_MASK;

		return nIndex() & ENT_ENTRY_MASK;
	}

	[[nodiscard]] int GetSerialNumber()
	{
		if (nIndex() == INVALID_EHANDLE_INDEX || nIndex() == 0)
			return 0;

		return nIndex() >> NUM_SERIAL_NUM_SHIFT_BITS;
	}

	CEntityInstance* m_pInstance; // 0x00
};

class CEntityInstance
{
public:
	void GetSchemaClassInfo(SchemaClassInfoData_t** pReturn)
	{
		return memory::CallVFunc<void, 44>(this, pReturn);
	}

	[[nodiscard]] CBaseHandle GetRefEHandle()
	{
		CEntityIdentity* pIdentity = m_pEntity();
		if (pIdentity == nullptr)
			return CBaseHandle();

		return CBaseHandle(pIdentity->GetEntryIndex(), pIdentity->GetSerialNumber() - (pIdentity->m_flags() & 1));
	}
	/*
	void OnPreDataChanged(EDataUpdateType nUpdateType)
	{
		return MEM::CallVFunc<void, 4U>(this, nUpdateType);
	}

	void OnDataChanged(EDataUpdateType nUpdateType)
	{
		return MEM::CallVFunc<void, 5U>(this, nUpdateType);
	}

	void PreDataUpdate(EDataUpdateType nUpdateType)
	{
		return MEM::CallVFunc<void, 6U>(this, nUpdateType);
	}
	*/
	void PostDataUpdate()
	{
		return memory::CallVFunc<void, 7U>(this, 1);
	}

	SCHEMA_ADD_FIELD(bool, m_bVisibleinPVS, "CEntityInstance->m_bVisibleinPVS");

	SCHEMA_ADD_FIELD(CEntityIdentity*, m_pEntity, "CEntityInstance->m_pEntity");
};


class CCollisionProperty
{
public:
	std::uint16_t CollisionMask()
	{
		return *reinterpret_cast<std::uint16_t*>(reinterpret_cast<std::uintptr_t>(this) + 0x38);
	}

	CS_CLASS_NO_INITIALIZER(CCollisionProperty);

	SCHEMA_ADD_FIELD(Vector_t, GetMins, "CCollisionProperty->m_vecMins");
	SCHEMA_ADD_FIELD(Vector_t, GetMaxs, "CCollisionProperty->m_vecMaxs");

	SCHEMA_ADD_FIELD(std::uint8_t, GetSolidFlags, "CCollisionProperty->m_usSolidFlags");
	SCHEMA_ADD_FIELD(std::uint8_t, GetCollisionGroup, "CCollisionProperty->m_CollisionGroup");
};

struct HitboxData_t {
	Vector_t m_vMins;
	Vector_t m_vMaxs;
	Vector_t m_vCenter;
	float m_flRadius;
	int m_nHitboxIndex;
	int m_nShapeType;
	bool m_bMultipoint;


	bool SegmentIntersectsCapsule(Vector_t vStart, Vector_t vPoint) {

		Vector_t capsuleStart = m_vMins;
		Vector_t capsuleEnd = m_vMaxs;

		Vector_t d1 = vPoint - vStart;
		Vector_t d2 = capsuleEnd - capsuleStart;
		Vector_t r = vStart - capsuleStart;

		float a = d1.DotProduct(d1);
		float e = d2.DotProduct(d2);
		float f = d2.DotProduct(r);

		float s, t;
		float c = d1.DotProduct(r);
		float b = d1.DotProduct(d2);
		float denom = a * e - b * b;

		if (denom != 0.0f) {
			s = (b * f - c * e) / denom;
		}
		else {
			s = 0.0f;
		}

		s = std::clamp(s, 0.0f, 1.0f);

		t = (b * s + f) / e;
		t = std::clamp(t, 0.0f, 1.0f);

		Vector_t closestPoint1 = vStart + d1 * s;
		Vector_t closestPoint2 = capsuleStart + d2 * t;

		float distSqr = (closestPoint1 - closestPoint2).LengthSqr();

		return distSqr <= (m_flRadius * m_flRadius);
		// idgaf, im not adding degenerate checks nor AABB handling because thats never the case.
	}

};
class CHitBox
{
public:
	const char* m_name;				// 0x0000
	const char* m_sSurfaceProperty;	// 0x0008
	const char* m_sBoneName;		// 0x0010
	Vector_t m_vecMinBounds;			// 0x0018
	Vector_t m_vecMaxBounds;			// 0x0024
	float m_flShapeRadius;			// 0x0030
	std::uint32_t m_nBoneNameHash;	// 0x0034
	std::int32_t m_nGroupId;		// 0x0038
	std::uint8_t m_nShapeType;		// 0x003C
	bool m_bTranslationOnly; 		// 0x003D
	uint8_t __pad003e[0x2]; // 0x3e
	std::uint32_t m_CRC;			// 0x0040
	Color_t m_cRenderColor;			// 0x0044
	std::uint16_t m_nHitboxIndex;	// 0x0048
	MEM_PAD(0x26);

	HitboxData_t ConstructHitboxData(BoneData_t boneData, bool multipoint) {
		HitboxData_t returnData = {};
		returnData.m_nShapeType = this->m_nShapeType;
		returnData.m_flRadius = this->m_flShapeRadius;
		returnData.m_nHitboxIndex = this->m_nHitboxIndex;
		returnData.m_vMins = boneData.vecRotation.RotateVector(this->m_vecMinBounds * boneData.flScale) + boneData.vecPosition;
		returnData.m_vMaxs = boneData.vecRotation.RotateVector(this->m_vecMaxBounds * boneData.flScale) + boneData.vecPosition;
		returnData.m_vCenter = (returnData.m_vMins + returnData.m_vMaxs) / 2.f;
		returnData.m_bMultipoint = multipoint;
		return returnData;
	}
};

class CHitBoxSets
{
public:
	MEM_PAD(0x20);				// 0x0000
	std::uint32_t m_nNameHash;	// 0x0020
	MEM_PAD(0x4);				// 0x0024
	std::int32_t m_nHitboxCount;// 0x0028
	MEM_PAD(0x4);				// 0x002C
	CHitBox* m_pHitBox;			// 0x0030
	MEM_PAD(0x18);				// 0x0038
};

//struct CHitBoxSets
//{
//	const char* m_name;           // 0x0
//	uint32_t m_nNameHash;         // 0x8
//	CUtlVector<CHitBox> m_HitBoxes; // 0x10
//	const char* m_SourceFilename; // 0x28
//};

class CRenderMesh
{
public:
	MEM_PAD(0x140);						// 0x0000
	CHitBoxSets* m_pHitboxSets;		// 0x108
	std::int32_t	m_nHitboxSets;		// 0x110
}; // Size=0x118

class CRenderMeshes
{
public:
	CRenderMesh* m_pMeshes;
};

class CModel
{
public:
	CHitBox* GetHitBox(const std::int32_t nIndex);
	const char* GetHitBoxName(const std::int32_t nIndex);
	int GetHitBoxCount();
public:
	MEM_PAD(0x70);					// 0x0000
	std::int32_t m_nRendermeshCount;// 0x0070
	MEM_PAD(0x4);					// 0x0074
	CRenderMeshes* m_pRenderMeshes;	// 0x0078
};

class CModelState
{
public:
	MEM_PAD(0x80);
	BoneData_t* m_pBoneArray;
	MEM_PAD(0x18);
	CStrongHandle<CModel> m_modelHandle;
public:
	SCHEMA_ADD_FIELD(CUtlSymbolLarge, m_ModelName, "CModelState->m_ModelName");
	SCHEMA_ADD_FIELD(CModel*, m_hModel, "CModelState->m_hModel");
	SCHEMA_ADD_FIELD_OFFSET(CModel*, m_hNewModel, "CModelState->m_hModel", 0x10);
};
class CSkeletonInstance;
class CGameSceneNode
{
public:
	CS_CLASS_NO_INITIALIZER(CGameSceneNode);

	SCHEMA_ADD_FIELD(CTransform, GetNodeToWorld, "CGameSceneNode->m_nodeToWorld");
	SCHEMA_ADD_FIELD(CEntityInstance*, GetOwner, "CGameSceneNode->m_pOwner");

	SCHEMA_ADD_FIELD(Vector_t, GetAbsOrigin, "CGameSceneNode->m_vecAbsOrigin");
	SCHEMA_ADD_FIELD(Vector_t, GetRenderOrigin, "CGameSceneNode->m_vRenderOrigin");

	SCHEMA_ADD_FIELD(QAngle_t, GetAngleRotation, "CGameSceneNode->m_angRotation");
	SCHEMA_ADD_FIELD(QAngle_t, GetAbsAngleRotation, "CGameSceneNode->m_angAbsRotation");

	SCHEMA_ADD_FIELD(bool, IsDormant, "CGameSceneNode->m_bDormant");

	CSkeletonInstance* GetSkeletonInstance()
	{
		return memory::CallVFunc<CSkeletonInstance*, 8U>(this);
	}
};

class C_BaseEntity : public CEntityInstance
{
public:
	CS_CLASS_NO_INITIALIZER(C_BaseEntity);

	[[nodiscard]] bool IsBasePlayerController()
	{
		SchemaClassInfoData_t* pClassInfo;
		GetSchemaClassInfo(&pClassInfo);
		if (pClassInfo == nullptr)
			return false;

		return FNV1A::Hash(pClassInfo->szName) == FNV1A::HashConst("C_CSPlayerController");
	}

	[[nodiscard]] bool IsPlayerPawn()
	{
		SchemaClassInfoData_t* pClassInfo = nullptr;
		GetSchemaClassInfo(&pClassInfo);
		if (!pClassInfo)
			return false;

		return FNV1A::Hash(pClassInfo->szName) == FNV1A::HashConst("C_CSPlayerPawn");
	}

	[[nodiscard]] bool IsWeapon()
	{
		static SchemaClassInfoData_t* pWeaponBaseClass = nullptr;
		if (pWeaponBaseClass == nullptr)
			Interface::SchemaSystem->FindTypeScopeForModule(XOR("client.dll"))->FindDeclaredClass(&pWeaponBaseClass, XOR("C_CSWeaponBase"));


		SchemaClassInfoData_t* pClassInfo;
		GetSchemaClassInfo(&pClassInfo);
		if (pClassInfo == nullptr)
			return false;

		return (pClassInfo->InheritsFrom(pWeaponBaseClass));
	}

	static C_BaseEntity* GetLocalPlayer();

	// get entity origin on scene
	[[nodiscard]] const Vector_t& GetSceneOrigin();

	SCHEMA_ADD_FIELD(CGameSceneNode*, GetGameSceneNode, "C_BaseEntity->m_pGameSceneNode");
	SCHEMA_ADD_FIELD(CCollisionProperty*, GetCollision, "C_BaseEntity->m_pCollision");
	SCHEMA_ADD_FIELD(std::uint8_t, GetTeam, "C_BaseEntity->m_iTeamNum");
	SCHEMA_ADD_FIELD(CBaseHandle, GetOwnerHandle, "C_BaseEntity->m_hOwnerEntity");
	SCHEMA_ADD_FIELD(Vector_t, GetBaseVelocity, "C_BaseEntity->m_vecBaseVelocity");
	SCHEMA_ADD_FIELD(Vector_t, GetAbsVelocity, "C_BaseEntity->m_vecAbsVelocity");
	SCHEMA_ADD_FIELD(bool, IsTakingDamage, "C_BaseEntity->m_bTakesDamage");
	SCHEMA_ADD_FIELD(std::uint32_t, GetFlags, "C_BaseEntity->m_fFlags");
	SCHEMA_ADD_FIELD(std::int32_t, GetEflags, "C_BaseEntity->m_iEFlags");
	SCHEMA_ADD_FIELD(std::uint8_t, GetMoveType, "C_BaseEntity->m_nActualMoveType"); // m_nActualMoveType returns CSGO style movetype, m_nMoveType returns bitwise shifted move type
	SCHEMA_ADD_FIELD(std::uint8_t, GetLifeState, "C_BaseEntity->m_lifeState");
	SCHEMA_ADD_FIELD(std::int32_t, GetHealth, "C_BaseEntity->m_iHealth");
	SCHEMA_ADD_FIELD(std::int32_t, GetMaxHealth, "C_BaseEntity->m_iMaxHealth");
	SCHEMA_ADD_FIELD(float, GetWaterLevel, "C_BaseEntity->m_flWaterLevel");
	SCHEMA_ADD_FIELD_OFFSET(void*, GetVData, "C_BaseEntity->m_nSubclassID", 0x8);
};

class CGlowProperty
{
public:
	SCHEMA_ADD_FIELD(bool, m_bFlashing, "CGlowProperty->m_bFlashing");
	SCHEMA_ADD_FIELD(bool, m_bGlowing, "CGlowProperty->m_bGlowing");

	SCHEMA_ADD_FIELD(std::int32_t, m_iGlowType, "CGlowProperty->m_iGlowType");
	SCHEMA_ADD_FIELD(std::int32_t, m_iGlowTeam, "CGlowProperty->m_iGlowTeam");
	SCHEMA_ADD_FIELD(std::int32_t, m_nGlowRange, "CGlowProperty->m_nGlowRange");
	SCHEMA_ADD_FIELD(std::int32_t, m_nGlowRangeMin, "CGlowProperty->m_nGlowRangeMin");

	SCHEMA_ADD_FIELD(std::float_t, m_flGlowTime, "CGlowProperty->m_flGlowTime");
	SCHEMA_ADD_FIELD(std::float_t, m_flGlowStartTime, "CGlowProperty->m_flGlowStartTime");

	SCHEMA_ADD_FIELD(Vector_t, m_fGlowColor, "CGlowProperty->m_fGlowColor");

	SCHEMA_ADD_FIELD(Color_t, m_glowColorOverride, "CGlowProperty->m_glowColorOverride");


	MEM_PAD(0x18);
	C_CSPlayerPawn* m_pOwner; //0x0018
};

class C_BaseModelEntity : public C_BaseEntity
{
public:
	CS_CLASS_NO_INITIALIZER(C_BaseModelEntity);

	SCHEMA_ADD_FIELD(CCollisionProperty, GetCollisionInstance, "C_BaseModelEntity->m_Collision");
	SCHEMA_ADD_FIELD(CGlowProperty, GetGlowProperty, "C_BaseModelEntity->m_Glow");
	SCHEMA_ADD_FIELD(Vector_t, GetViewOffset, "C_BaseModelEntity->m_vecViewOffset");
	SCHEMA_ADD_FIELD(GameTime_t, GetCreationTime, "C_BaseModelEntity->m_flCreateTime");
	SCHEMA_ADD_FIELD(GameTick_t, GetCreationTick, "C_BaseModelEntity->m_nCreationTick");
	SCHEMA_ADD_FIELD(CBaseHandle, GetMoveParent, "C_BaseModelEntity->m_hOldMoveParent");
	SCHEMA_ADD_FIELD(std::float_t, GetAnimTime, "C_BaseModelEntity->m_flAnimTime");
	SCHEMA_ADD_FIELD(std::float_t, GetSimulationTime, "C_BaseModelEntity->m_flSimulationTime");
};

class CPlayer_ItemServices;
class CPlayer_CameraServices;

class CPlayer_WeaponServices //meow
{
public:
	SCHEMA_ADD_FIELD(CHandle<C_CSWeaponBase>, m_hActiveWeapon, "CPlayer_WeaponServices->m_hActiveWeapon");
	SCHEMA_ADD_FIELD(C_NetworkUtlVectorBase<CHandle<C_BasePlayerWeapon>>, m_hMyWeapons, "CPlayer_WeaponServices->m_hMyWeapons");
};

class CCSPlayer_WeaponServices : public CPlayer_WeaponServices
{
public:
	SCHEMA_ADD_FIELD(GameTime_t, GetNextAttack, "CCSPlayer_WeaponServices->m_flNextAttack");
};

class C_BasePlayerPawn : public C_BaseModelEntity
{
public:
	CS_CLASS_NO_INITIALIZER(C_BasePlayerPawn);

	SCHEMA_ADD_FIELD(CBaseHandle, GetControllerHandle, "C_BasePlayerPawn->m_hController");
	SCHEMA_ADD_FIELD(CCSPlayer_WeaponServices*, GetWeaponServices, "C_BasePlayerPawn->m_pWeaponServices");
	SCHEMA_ADD_FIELD(CPlayer_ItemServices*, GetItemServices, "C_BasePlayerPawn->m_pItemServices");
	SCHEMA_ADD_FIELD(CPlayer_CameraServices*, GetCameraServices, "C_BasePlayerPawn->m_pCameraServices");

	[[nodiscard]] Vector_t GetEyePosition()
	{
		Vector_t vecEyePosition = Vector_t(0.0f, 0.0f, 0.0f);
		// Credit: https://www.unknowncheats.me/forum/4258133-post6228.html
		memory::CallVFunc<void, 169U>(this, &vecEyePosition);
		return vecEyePosition;
	}
};

class CCSPlayer_ViewModelServices;

class C_CSPlayerPawnBase : public C_BasePlayerPawn
{
public:
	CS_CLASS_NO_INITIALIZER(C_CSPlayerPawnBase);

	SCHEMA_ADD_FIELD(CCSPlayer_ViewModelServices*, GetViewModelServices, "C_CSPlayerPawnBase->m_pViewModelServices");
	SCHEMA_ADD_FIELD(float, GetLowerBodyYawTarget, "C_CSPlayerPawnBase->m_flLowerBodyYawTarget");
	SCHEMA_ADD_FIELD(float, GetFlashMaxAlpha, "C_CSPlayerPawnBase->m_flFlashMaxAlpha");
	SCHEMA_ADD_FIELD(float, GetFlashDuration, "C_CSPlayerPawnBase->m_flFlashDuration");
	SCHEMA_ADD_FIELD(Vector_t, GetLastSmokeOverlayColor, "C_CSPlayerPawnBase->m_vLastSmokeOverlayColor");
	SCHEMA_ADD_FIELD(int, GetSurvivalTeam, "C_CSPlayerPawnBase->m_nSurvivalTeam"); // danger zone
};

class C_CSPlayerPawn : public C_CSPlayerPawnBase
{
public:
	CS_CLASS_NO_INITIALIZER(C_CSPlayerPawn);

	[[nodiscard]] bool IsOtherEnemy(C_CSPlayerPawn* pOther);
	[[nodiscard]] int GetAssociatedTeam();
	[[nodiscard]] bool CanAttack(const float flServerTime);
	[[nodiscard]] std::uint32_t GetOwnerHandleIndex();
	[[nodiscard]] std::uint16_t GetCollisionMask();

	SCHEMA_ADD_FIELD(bool, IsScoped, "C_CSPlayerPawn->m_bIsScoped");
	SCHEMA_ADD_FIELD(bool, IsDefusing, "C_CSPlayerPawn->m_bIsDefusing");
	SCHEMA_ADD_FIELD(bool, IsGrabbingHostage, "C_CSPlayerPawn->m_bIsGrabbingHostage");
	SCHEMA_ADD_FIELD(bool, IsWaitForNoAttack, "C_CSPlayerPawn->m_bWaitForNoAttack");
	SCHEMA_ADD_FIELD(int, GetShotsFired, "C_CSPlayerPawn->m_iShotsFired");
	SCHEMA_ADD_FIELD(std::int32_t, GetArmorValue, "C_CSPlayerPawn->m_ArmorValue");
	SCHEMA_ADD_FIELD(QAngle_t, GetAimPuchAngle, "C_CSPlayerPawn->m_aimPunchAngle");

	SCHEMA_ADD_FIELD(bool, IsSpotted, "EntitySpottedState_t->m_bSpotted");
};

class CBasePlayerController : public C_BaseModelEntity
{
public:
	CS_CLASS_NO_INITIALIZER(CBasePlayerController);

	SCHEMA_ADD_FIELD(std::uint64_t, GetSteamId, "CBasePlayerController->m_steamID");
	SCHEMA_ADD_FIELD(std::uint32_t, GetTickBase, "CBasePlayerController->m_nTickBase");
	SCHEMA_ADD_FIELD(CBaseHandle, GetPawnHandle, "CBasePlayerController->m_hPawn");
	SCHEMA_ADD_FIELD(bool, IsLocalPlayerController, "CBasePlayerController->m_bIsLocalPlayerController");
};


class CCSPlayerController : public CBasePlayerController
{
public:
	CS_CLASS_NO_INITIALIZER(CCSPlayerController);

	[[nodiscard]] static CCSPlayerController* GetLocalPlayerController();

	// @note: always get origin from pawn not controller
	[[nodiscard]] const Vector_t& GetPawnOrigin();

	SCHEMA_ADD_FIELD(std::uint32_t, GetPing, "CCSPlayerController->m_iPing");
	SCHEMA_ADD_FIELD(const char*, GetPlayerName, "CCSPlayerController->m_sSanitizedPlayerName");
	SCHEMA_ADD_FIELD(std::int32_t, GetPawnHealth, "CCSPlayerController->m_iPawnHealth");
	SCHEMA_ADD_FIELD(std::int32_t, GetPawnArmor, "CCSPlayerController->m_iPawnArmor");
	SCHEMA_ADD_FIELD(bool, IsPawnHasDefuser, "CCSPlayerController->m_bPawnHasDefuser");
	SCHEMA_ADD_FIELD(bool, IsPawnHasHelmet, "CCSPlayerController->m_bPawnHasHelmet");
	SCHEMA_ADD_FIELD(bool, IsPawnAlive, "CCSPlayerController->m_bPawnIsAlive");
	SCHEMA_ADD_FIELD(CHandle<C_CSPlayerPawn>, m_hPlayerPawn, "CCSPlayerController->m_hPlayerPawn");
	SCHEMA_ADD_OFFSET(const char*, m_sSanitizedPlayerName, 0x778);
};

class CBaseAnimGraph : public C_BaseModelEntity
{
public:
	CS_CLASS_NO_INITIALIZER(CBaseAnimGraph);

	SCHEMA_ADD_FIELD(bool, IsClientRagdoll, "CBaseAnimGraph->m_bClientRagdoll");
};

class C_BaseFlex : public C_BaseModelEntity
{
public:
	CS_CLASS_NO_INITIALIZER(C_BaseFlex);
	/* not implemented */
};

class C_BaseToggle : public C_BaseModelEntity
{
public:
};

class C_BaseTrigger : public C_BaseToggle
{
	SCHEMA_ADD_FIELD(bool, m_bDisabled, "C_BaseTrigger->m_bDisabled");
	SCHEMA_ADD_FIELD(bool, m_bClientSidePredicted, "C_BaseTrigger->m_bClientSidePredicted");
};

class C_PostProcessingVolume : public C_BaseTrigger
{
public:
	SCHEMA_ADD_FIELD(bool, m_bExposureControl, "C_PostProcessingVolume->m_bExposureControl");

	SCHEMA_ADD_FIELD(std::float_t, m_flFadeDuration, "C_PostProcessingVolume->m_flFadeDuration");
	SCHEMA_ADD_FIELD(std::float_t, m_flMinExposure, "C_PostProcessingVolume->m_flMinExposure");
	SCHEMA_ADD_FIELD(std::float_t, m_flMaxExposure, "C_PostProcessingVolume->m_flMaxExposure");

	SCHEMA_ADD_FIELD(std::float_t, m_flExposureFadeSpeedUp, "C_PostProcessingVolume->m_flExposureFadeSpeedUp");
	SCHEMA_ADD_FIELD(std::float_t, m_flExposureFadeSpeedDown, "C_PostProcessingVolume->m_flExposureFadeSpeedDown");
};

class C_EconItemView
{
public:
	CS_CLASS_NO_INITIALIZER(C_EconItemView);

	SCHEMA_ADD_FIELD(std::uint16_t, GetItemDefinitionIndex, "C_EconItemView->m_iItemDefinitionIndex");
	SCHEMA_ADD_FIELD(std::uint64_t, GetItemID, "C_EconItemView->m_iItemID");
	SCHEMA_ADD_FIELD(std::uint32_t, GetItemIDHigh, "C_EconItemView->m_iItemIDHigh");
	SCHEMA_ADD_FIELD(std::uint32_t, GetItemIDLow, "C_EconItemView->m_iItemIDLow");
	SCHEMA_ADD_FIELD(std::uint32_t, GetAccountID, "C_EconItemView->m_iAccountID");
	SCHEMA_ADD_FIELD(char[161], GetCustomName, "C_EconItemView->m_szCustomName");
	SCHEMA_ADD_FIELD(char[161], GetCustomNameOverride, "C_EconItemView->m_szCustomNameOverride");
};

class CAttributeManager
{
public:
	CS_CLASS_NO_INITIALIZER(CAttributeManager);
	virtual ~CAttributeManager() = 0;
};
static_assert(sizeof(CAttributeManager) == 0x8);

class C_AttributeContainer : public CAttributeManager
{
public:
	CS_CLASS_NO_INITIALIZER(C_AttributeContainer);

	SCHEMA_ADD_PFIELD(C_EconItemView, GetItem, "C_AttributeContainer->m_Item");
};

class C_EconEntity : public C_BaseFlex
{
public:
	CS_CLASS_NO_INITIALIZER(C_EconEntity);

	SCHEMA_ADD_PFIELD(C_AttributeContainer, GetAttributeManager, "C_EconEntity->m_AttributeManager");
	SCHEMA_ADD_FIELD(std::uint32_t, GetOriginalOwnerXuidLow, "C_EconEntity->m_OriginalOwnerXuidLow");
	SCHEMA_ADD_FIELD(std::uint32_t, GetOriginalOwnerXuidHigh, "C_EconEntity->m_OriginalOwnerXuidHigh");
	SCHEMA_ADD_FIELD(std::int32_t, GetFallbackPaintKit, "C_EconEntity->m_nFallbackPaintKit");
	SCHEMA_ADD_FIELD(std::int32_t, GetFallbackSeed, "C_EconEntity->m_nFallbackSeed");
	SCHEMA_ADD_FIELD(std::int32_t, GetFallbackWear, "C_EconEntity->m_flFallbackWear");
	SCHEMA_ADD_FIELD(std::int32_t, GetFallbackStatTrak, "C_EconEntity->m_nFallbackStatTrak");
	SCHEMA_ADD_FIELD(CBaseHandle, GetViewModelAttachmentHandle, "C_EconEntity->m_hViewmodelAttachment");
};

class C_EconWearable : public C_EconEntity
{
public:
	CS_CLASS_NO_INITIALIZER(C_EconWearable);

	SCHEMA_ADD_FIELD(std::int32_t, GetForceSkin, "C_EconWearable->m_nForceSkin");
	SCHEMA_ADD_FIELD(bool, IsAlwaysAllow, "C_EconWearable->m_bAlwaysAllow");
};

class C_BasePlayerWeapon : public C_EconEntity
{
public:
	CS_CLASS_NO_INITIALIZER(C_BasePlayerWeapon);


	SCHEMA_ADD_FIELD(GameTick_t, GetNextPrimaryAttackTick, "C_BasePlayerWeapon->m_nNextPrimaryAttackTick");
	SCHEMA_ADD_FIELD(float, GetNextPrimaryAttackTickRatio, "C_BasePlayerWeapon->m_flNextPrimaryAttackTickRatio");
	SCHEMA_ADD_FIELD(GameTick_t, GetNextSecondaryAttackTick, "C_BasePlayerWeapon->m_nNextSecondaryAttackTick");
	SCHEMA_ADD_FIELD(float, GetNextSecondaryAttackTickRatio, "C_BasePlayerWeapon->m_flNextSecondaryAttackTickRatio");
	SCHEMA_ADD_FIELD(std::int32_t, GetClip1, "C_BasePlayerWeapon->m_iClip1");
	SCHEMA_ADD_FIELD(std::int32_t, GetClip2, "C_BasePlayerWeapon->m_iClip2");
	SCHEMA_ADD_FIELD(std::int32_t[2], GetReserveAmmo, "C_BasePlayerWeapon->m_pReserveAmmo");
};

class C_CSWeaponBase : public C_BasePlayerWeapon
{
public:
	CS_CLASS_NO_INITIALIZER(C_CSWeaponBase);

	SCHEMA_ADD_FIELD(bool, IsInReload, "C_CSWeaponBase->m_bInReload");

	CCSWeaponBaseVData* GetWeaponVData()
	{
		return static_cast<CCSWeaponBaseVData*>(GetVData());
	}
};

class C_CSWeaponBaseGun : public C_CSWeaponBase
{
public:
	CS_CLASS_NO_INITIALIZER(C_CSWeaponBaseGun);

	SCHEMA_ADD_FIELD(std::int32_t, GetZoomLevel, "C_CSWeaponBaseGun->m_zoomLevel");
	SCHEMA_ADD_FIELD(std::int32_t, GetBurstShotsRemaining, "C_CSWeaponBaseGun->m_iBurstShotsRemaining");
	SCHEMA_ADD_FIELD(bool, IsBurstMode, "C_CSWeaponBase->m_bBurstMode");
	SCHEMA_ADD_FIELD(float, GetPostponeFireReadyFrac, "C_CSWeaponBase->m_flPostponeFireReadyFrac");

	[[nodiscard]] bool CanPrimaryAttack(const int nWeaponType, const float flServerTime);
	[[nodiscard]] bool CanSecondaryAttack(const int nWeaponType, const float flServerTime);
};

class C_BaseCSGrenade : public C_CSWeaponBase
{
public:
	SCHEMA_ADD_FIELD(bool, IsHeldByPlayer, "C_BaseCSGrenade->m_bIsHeldByPlayer");
	SCHEMA_ADD_FIELD(bool, IsPinPulled, "C_BaseCSGrenade->m_bPinPulled");
	SCHEMA_ADD_FIELD(GameTime_t, GetThrowTime, "C_BaseCSGrenade->m_fThrowTime");
	SCHEMA_ADD_FIELD(float, GetThrowStrength, "C_BaseCSGrenade->m_flThrowStrength");
};

class C_BaseGrenade : public C_BaseFlex
{
public:
	CS_CLASS_NO_INITIALIZER(C_BaseGrenade);
};



class CSkeletonInstance : public CGameSceneNode
{
public:
	/*
	MEM_PAD(0x1CC); //0x0000
	int nBoneCount; //0x01CC
	MEM_PAD(0x18); //0x01D0
	int nMask; //0x01E8
	MEM_PAD(0x4); //0x01EC
	Matrix2x4_t* pBoneCache; //0x01F0
	*/

	MEM_PAD(0x1E0); //0x01EC 
	Matrix2x4_t* m_pBoneCache; //0x01F0  //mb 0x210
	SCHEMA_ADD_FIELD(CModelState, m_modelState, "CSkeletonInstance->m_modelState");
	SCHEMA_ADD_FIELD(std::uint8_t, m_nHitboxSet, "CSkeletonInstance->m_nHitboxSet");

};
class CSmokeGrenadeProjectile
{
public:
	SCHEMA_ADD_FIELD(Vector_t, m_vSmokeColor, "C_SmokeGrenadeProjectile->m_vSmokeColor");
	SCHEMA_ADD_FIELD(bool, m_bDidSmokeEffect, "C_SmokeGrenadeProjectile->m_bDidSmokeEffect");
};

class C_EnvSky
{
public:
	SCHEMA_ADD_FIELD(Color_t, m_vTintColor, "C_EnvSky->m_vTintColor");
	SCHEMA_ADD_FIELD(bool, m_bEnabled, "C_EnvSky->m_bEnabled");

};
