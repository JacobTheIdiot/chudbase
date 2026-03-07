#include "pch.h"
#include "schema.h"

// used: [stl] vector
#include <vector>
// used: [stl] find_if
#include <algorithm>

// used: ischemasystem
#include "cs2/interface/interfaces.h"
#include "cs2/interface/interfaces/ischemasystem.h"

// used: l_print
#include "core/util/log.h"

struct SchemaData_t
{
	FNV1A_t uHashedFieldName = 0x0ULL;
	std::uint32_t uOffset = 0x0U;
};

static std::vector<SchemaData_t> vecSchemaData;

bool schema::Setup(const wchar_t* wszFileName, const char* szModuleName)
{
	CSchemaSystemTypeScope* pTypeScope = Interface::SchemaSystem->FindTypeScopeForModule(szModuleName);
	if (pTypeScope == nullptr)
		return false;

	int nClassesFound = 0;

	for (int i = 0; i < 256; i++)
	{
		SchemaBucket& bucket = pTypeScope->m_Buckets[i];

		for (SchemaListElement* element = bucket.pFirst; element != nullptr; element = element->pNext)
		{
			CSchemaClassBinding* pClassBinding = element->pBinding;
			if (pClassBinding == nullptr)
				continue;

			SchemaClassInfoData_t* pDeclaredClassInfo;
			pTypeScope->FindDeclaredClass(&pDeclaredClassInfo, pClassBinding->szBinaryName);

			if (pDeclaredClassInfo == nullptr)
				continue;

			if (pDeclaredClassInfo->nFieldSize == 0)
				continue;

			nClassesFound++;

#ifdef _DEBUG
			L_PRINT(LOG_INFO) << XOR("dumped \"") << pDeclaredClassInfo->szName << XOR("\" (total: ") << pDeclaredClassInfo->nFieldSize << XOR(" fields)");
#endif
		}
	}

	L_PRINT(LOG_INFO) << XOR("found \"") << nClassesFound << XOR("\" schema classes in module");


	return true;
}

std::uint32_t schema::GetOffset(const FNV1A_t uHashedFieldName)
{
	if (const auto it = std::ranges::find_if(vecSchemaData, [uHashedFieldName](const SchemaData_t& data)
		{ return data.uHashedFieldName == uHashedFieldName; });
		it != vecSchemaData.end())
		return it->uOffset;

	L_PRINT(LOG_ERROR) << XOR("failed to find offset for field with hash: ") << L::AddFlags(LOG_MODE_INT_FORMAT_HEX | LOG_MODE_INT_SHOWBASE) << uHashedFieldName;
	CS_ASSERT(false); // schema field not found
	return 0U;
}

std::uint32_t schema::GetForeignOffset(const char* szModulenName, const FNV1A_t uHashedClassName, const FNV1A_t uHashedFieldName)
{
	CSchemaSystemTypeScope* pTypeScope = Interface::SchemaSystem->FindTypeScopeForModule(szModulenName);
	if (pTypeScope == nullptr)
		return 0;

	std::uint32_t uOffset = 0x0;

	// Iterate through all 256 buckets
	for (int i = 0; i < 256; i++)
	{
		SchemaBucket& bucket = pTypeScope->m_Buckets[i];

		// Iterate through linked list in this bucket
		for (SchemaListElement* element = bucket.pFirst; element != nullptr; element = element->pNext)
		{
			CSchemaClassBinding* pClassBinding = element->pBinding;
			if (pClassBinding == nullptr)
				continue;

			SchemaClassInfoData_t* pDeclaredClassInfo;
			pTypeScope->FindDeclaredClass(&pDeclaredClassInfo, pClassBinding->szBinaryName);

			if (pDeclaredClassInfo == nullptr)
				continue;

			if (pDeclaredClassInfo->nFieldSize == 0)
				continue;

			for (auto j = 0; j < pDeclaredClassInfo->nFieldSize; j++)
			{
				SchemaClassFieldData_t* pFields = pDeclaredClassInfo->pFields;
				if (pFields == nullptr)
					continue;

				SchemaClassFieldData_t field = pFields[j];
				if (FNV1A::Hash(pClassBinding->szBinaryName) == uHashedClassName && FNV1A::Hash(field.szName) == uHashedFieldName)
				{
					uOffset = field.nSingleInheritanceOffset;
					goto found; // Exit both loops
				}
			}
		}
	}

found:
	if (uOffset == 0x0)
		L_PRINT(LOG_WARNING) << XOR("failed to find offset for field with hash: ") << L::AddFlags(LOG_MODE_INT_FORMAT_HEX | LOG_MODE_INT_SHOWBASE) << uHashedFieldName;

	return uOffset;
}