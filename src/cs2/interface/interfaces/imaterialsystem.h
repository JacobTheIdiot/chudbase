#pragma once
#include "core/mem/memory.h"
#include "cs2/datatypes/color.h"
#include "cs2/datatypes/vector.h"
#include "cs2/datatypes/matrix.h"
#include "cs2/schema/schema.h"

struct MaterialParamater_t
{
	Vector4D_t m_vecValue;
	void* m_pTextureValue;
	MEM_PAD(0x10);
	const char* m_szParamName;
	const char* m_szValue;
	std::int64_t m_nValue;
};

class CMaterial2
{
public:
	const char* GetName()
	{
		return memory::CallVFunc<const char*, 0U>(this);
	}

	const char* GetShaderName()
	{
		return memory::CallVFunc<const char*, 1U>(this);
	}

	MaterialParamater_t* FindParamater(const char* szName)
	{
		using fnFindParamater = MaterialParamater_t * (__fastcall*)(CMaterial2*, const char*);
		static const auto oFindParamater = reinterpret_cast<fnFindParamater>(memory::FindPattern(MATERIAL_SYSTEM2_DLL, XOR("48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC 20 48 8B 59 18")));

		// spoofing crashes
		return oFindParamater(this, szName);
		//ReturnAddressSpoofer::SpoofCall(ReturnAddressSpoofGadgets::m_pMaterialSystemGadget, oFindParamater, this, szName);
	}

	void UpdateParamater()
	{
		using fnUpdateParamater = MaterialParamater_t * (__fastcall*)(CMaterial2*);
		//static const auto oUpdateParamater = reinterpret_cast<fnUpdateParamater>(Memory::GetAbsoluteAddress(Memory::FindPattern(MATERIAL_SYSTEM2_DLL, X("E8 ? ? ? ? 80 63 0B FB")), 0x1, 0x0));
		static const auto oUpdateParamater = reinterpret_cast<fnUpdateParamater>(memory::FindPattern(MATERIAL_SYSTEM2_DLL, XOR("48 89 7C 24 ? 41 56 48 83 EC ? 8B 81")));

		// spoofing crashes
		oUpdateParamater(this);
		//ReturnAddressSpoofer::SpoofCall(ReturnAddressSpoofGadgets::m_pMaterialSystemGadget, oUpdateParamater, this);
	}

	void SetVecParamater(const char* szName, Vector4D_t vecValue)
	{
		const auto pParamater = FindParamater(szName);
		if (!pParamater)
			return;

		pParamater->m_vecValue = vecValue;
	}

	void SetTextureParameter(const char* szName, void* szParam)
	{
		const auto pParameter = FindParamater(szName);
		if (!pParameter)
			return;

		if (!pParameter->m_pTextureValue)
			return;

		pParameter->m_pTextureValue = szParam;
	}
};

struct MaterialArray_t
{
	uint64_t m_iCount;
	CMaterial2*** m_pResource;
	uint64_t pad_0010[3]; //MEM_PAD(0x18);
};

class CObjectInfo
{
	MEM_PAD(0xB0);
	int m_nId;
};

// https://www.unknowncheats.me/forum/4099577-post10.html
class CSceneObject {
public:
	void* vftable;              // 0x0
	void* heapPtr;              // 0x8
	void* m_pMeshInstanceData;  // 0x10
	void* m_pDesc;              // 0x18
	void* m_pRefData;           // 0x20
private:
	MEM_PAD(0x8);

public:
	Matrix3x4_t m_transform;  // 0x30
private:
	MEM_PAD(0x26);

public:
	uint16_t m_nObjectTypeFlags;    // 0x86
	uint16_t m_nGameRenderCounter;  // 0x88
	uint8_t m_clr[3];               // 0x8a
private:
	MEM_PAD(0x3);

public:
	void* m_pPVS;        // 0x90
	void* m_pExtraData;  // 0x98
private:
	MEM_PAD(0x10);

public:
	void* m_pWorld;                          // 0xb0
	CHandle<C_BaseEntity> m_hExternalOwner;  // 0xb8
private:
	MEM_PAD(0x14);
};  // Size: 0xd0

class CAnimatableSceneObjectDesc;
class CSceneAnimatableObject
{
public:

	SCHEMA_ADD_OFFSET(int, GetBoneCount, 0xD0);
	SCHEMA_ADD_OFFSET(Matrix3x4_t*, GetRenderBones, 0xD8);

	MEM_PAD(16);
	void* m_pMeshInstance{ };
	void* m_pDesc{ };
	MEM_PAD(16);
	Matrix3x4_t m_matTranslationMatrix{ };
	MEM_PAD(88);
	CHandle<C_BaseEntity> m_hOwnerHandle{ };
	MEM_PAD(80);
	CSkeletonInstance* m_pSkeleton{ };
	MEM_PAD(24);
};

class CSceneData
{
public:
	char pad_0000[0x18];
	CSceneAnimatableObject* m_pSceneObject; //0x0018
	CMaterial2* m_pMaterial; //0x0020
	CMaterial2* m_pMaterial2; //0x0028
	char pad_0030[32]; //0x0030
	Color_t m_cColor; //0x0050
	char pad_0054[16]; //0x0054
};

class CMeshPrimitiveOutputBuffer
{
public:
	CSceneData* m_pMeshPrimitive;
	char __1[0x4];
	int m_nCount;

	CSceneData* GetPrimitive(int nIndex) noexcept
	{
		return &m_pMeshPrimitive[nIndex];
	}
};

class CAnimatableSceneObjectDesc
{
public:
	CSceneAnimatableObject* CreateSceneAnimatableObject()
	{
		return memory::CallVFunc<CSceneAnimatableObject*, 21>(this);
	}

	void* CreateBaseSceneObjectDesc(bool bAllocate)
	{
		return memory::CallVFunc<void*, 22>(this, bAllocate);
	}
};

class c_aggregate_object_data
{
public:
	char pad_0000[4]; //0x0000
	int count; //0x0004
	char pad_0008[40]; //0x0008
	int index; //0x0030
}; //Size: 0x0034

class c_aggregate_object_array
{
public:
	void* object; //0x0000
	c_aggregate_object_data* data; //0x0008
};

class CAggregateSceneObjectDataWorld {
private:
	char pad_0000[0x38]; // 0x0
public:
	unsigned char r; // 0x38
	unsigned char g; // 0x39
	unsigned char b; // 0x3A
private:
	char pad_0038[0x9];
};

class CAggregateSceneObjectWorld {
private:
	char pad_0000[0x120];
public:
	int count; // 0x120
private:
	char pad_0120[0x4];
public:
	CAggregateSceneObjectDataWorld* array; // 0x128
};

class CAggregateSceneObject
{
public:
	unsigned char pad_0[0xE4]; // 0x0 - 0xE3
	//Color_t m_col; // 0xE4
	float red; // 0xE4
	float green; // 0xE8
	float blue; // 0xEC
};


class C_AggregateSceneObjectData
{
private:
	char pad_0000[0x38]; // 0x0
public:
	std::uint8_t r; // 0x38
	std::uint8_t g; // 0x39
	std::uint8_t b; // 0x3A
private:
	char pad_0038[0x9];
};

class C_AggregateSceneObject
{
private:
	char pad_0000[0x120];
public:
	int m_nCount; // 0x120
private:
	char pad_0120[0x4];
public:
	C_AggregateSceneObjectData* m_pData; // 0x128

};

class C_AggregateObjectData {
private:
	char pad_0000[4];
public:
	int count;
private:
	char pad_00008[40];
public:
	int index;
};

class C_AggregateObjectArray {
public:
	void* object;
	C_AggregateObjectData* data;
};

class CDrawMesh
{
public:
	/*void SetShaderType(const char* szShaderName)
	{
		#ifdef CS_PARANOID
		CS_ASSERT(Functions::fnSetMaterialShaderType != nullptr);
		#endif

		MaterialKeyVar_t shaderVar(X("shader"), 0x162C1777);
		ReturnAddressSpoofer::SpoofCall(ReturnAddressSpoofGadgets::m_pParticlesGadget, Functions::fnSetMaterialShaderType, this, shaderVar, szShaderName, 0x18);
	}

	void SetMaterialFunction(const char* szFunctionName, int nValue)
	{
		#ifdef CS_PARANOID
		CS_ASSERT(Functions::fnSetMaterialFunction != nullptr);
		#endif

		MaterialKeyVar_t functionVar(szFunctionName, true);
		ReturnAddressSpoofer::SpoofCall(ReturnAddressSpoofGadgets::m_pParticlesGadget, Functions::fnSetMaterialFunction, this, functionVar, nValue, 0x18);
	}*/

	MEM_PAD(0x18); // 0x0
	CSceneAnimatableObject* m_pSceneAnimatableObject; // 0x18
	CMaterial2* m_pMaterial; // 0x20
	MEM_PAD(0x28); // 0x28
	Color_t m_colValue; // 0x48
	MEM_PAD(0x4); // 0x4C
	CObjectInfo* m_pObjectInfo; // 0x50
	MEM_PAD(0x4); // 0x58
	std::int32_t m_nDrawFlag; // 0x60

	//uint32_t m_nSortKey;
	//PAD(0x4);
	//union {
	//	CMeshInstance* m_pMeshInstance;
	//	void* m_pPayload;
	//};
	//PAD(0x8);
	//const CSceneAnimatableObject* m_pObject;
	//const CMaterial2* m_pMaterial;
	//const CSceneSystemStencilState* m_pStencilStateOverride;
	//const CSceneSystemBakedLightingInfo* m_pBakedLightingInfo;
	//uint16_t m_perInstanceBakedLightingParams[4];
	//Color_t m_rgba;
	//matrix3x4_t* m_pTransform;
	//uint16_t m_nRequiredTextureSize;
	//uint8_t m_nDrawCall;
	//uint8_t m_nObjectClassSettings;
	//uint16_t m_nBatchFlags;
	//PAD(0x2);
};

class IMaterialSystem2
{
public:
	CMaterial2*** FindOrCreateFromResource(CMaterial2*** pOutMaterial, const char* szMaterialName)
	{
		return memory::CallVFunc<CMaterial2***, 14U>(this, pOutMaterial, szMaterialName);
	}

	CMaterial2** CreateMaterial(CMaterial2*** pOutMaterial, const char* szMaterialName, CDrawMesh* pData)
	{
		return memory::CallVFunc<CMaterial2**, 29U>(this, pOutMaterial, szMaterialName, pData, 0, 0, 0, 0, 0, 1);
	}

	void SetCreateDataByMaterial(const void* pData, CMaterial2*** const pInMaterial)
	{
		return memory::CallVFunc<void, 37U>(this, pInMaterial, pData);
	}
};