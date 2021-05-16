#include <BloodMoney/Game/OnLevel/CKeycardReader.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney
{
    Glacier::ZEntityLocator* CKeycardReader::GetNearestDoor(unsigned int iDoorsCount, Glacier::ZEntityLocator** ppDoors) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_GetNearestDoor != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_GetNearestDoor != BMConfigurationService::kNotConfiguredOption)
        {
            return ((Glacier::ZEntityLocator*(__thiscall*)(CKeycardReader*, unsigned int, Glacier::ZEntityLocator**))BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_GetNearestDoor)(this, iDoorsCount, ppDoors);
        }
        return nullptr;
    }

    void CKeycardReader::ValidateKeyCard(Glacier::ZREF cardREF) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_ValidateKeyCard != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_ValidateKeyCard != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(CKeycardReader*, Glacier::ZREF))BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_ValidateKeyCard)(this, cardREF);
        }
    }

    void CKeycardReader::InvalidateKeyCard(Glacier::ZREF cardREF) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_InvalidateKeyCard != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_InvalidateKeyCard != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(CKeycardReader*, Glacier::ZREF))BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_InvalidateKeyCard)(this, cardREF);
        }
    }

    void CKeycardReader::DetermineLnkObjLocation(sQueryLnkObjLocation* pQuery) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_DetermineLnkObjLocation != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_DetermineLnkObjLocation != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(CKeycardReader*, sQueryLnkObjLocation*))BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_DetermineLnkObjLocation)(this, pQuery);
        }
    }

    Glacier::ZREF CKeycardReader::GetKeyCard(Glacier::ZREF playerREF, bool* hasCard) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_GetKeyCard != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_GetKeyCard != BMConfigurationService::kNotConfiguredOption) {
            return ((Glacier::ZREF(__thiscall*)(CKeycardReader*, Glacier::ZREF, bool*))BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_GetKeyCard)(this, playerREF, hasCard);
        }

        return 0;
    }

    Glacier::ZREF CKeycardReader::GetItemTemplateFromName(const char* itemName) {
        assert(BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_GetItemTemplateFromName != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_GetItemTemplateFromName != BMConfigurationService::kNotConfiguredOption) {
            return ((Glacier::ZREF(__thiscall*)(CKeycardReader*, const char*))BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_GetItemTemplateFromName)(this, itemName);
        }
        return 0;
    }

    bool CKeycardReader::IsKeyCardValid(BloodMoney::ZHM3Item* pItem) {
        if (!pItem) return false;

        assert(BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_IsKeyCardValid != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_IsKeyCardValid != BMConfigurationService::kNotConfiguredOption) {
            return ((bool(__thiscall*)(CKeycardReader*, BloodMoney::ZHM3Item*))BMConfigurationService::BMAPI_FunctionAddress_CKeycardReader_IsKeyCardValid)(this, pItem);
        }

        return false;
    }
}