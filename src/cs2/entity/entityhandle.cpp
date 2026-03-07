#include "entityhandle.h"
#include "pch.h"
#include "cs2/interface/Interfaces.h"
#include "cs2/interface/interfaces/igameresourceservice.h"
#include "cs2/interface/interfaces/cgameentitysystem.h"
#include "cs2/entity/entity.h"
C_BaseEntity* CBaseHandle::Get() const
{
    if (!IsValid())
        return nullptr;

    C_BaseEntity* pEntity = Interface::GameResourceService->pGameEntitySystem->Get(GetEntryIndex());
    if (!pEntity || pEntity->GetRefEHandle() != *this)
        return nullptr;

    return pEntity;
}