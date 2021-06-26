#include <BloodMoney/Game/ZHM3Actor.h>
#include <BloodMoney/BMConfigurationService.h>

#include <cassert>

namespace Hitman::BloodMoney
{
    void ZHM3Actor::PreparePath()
    {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3Actor_PreparePath != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3Actor_PreparePath != BMConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(Hitman::BloodMoney::ZHM3Actor*))(BMConfigurationService::BMAPI_FunctionAddress_ZHM3Actor_PreparePath))(this);
        }
    }

    void ZHM3Actor::ActivateBloodSpurt(const Glacier::Vector3* pInvDir,
                                       const Glacier::Vector3* pCollisionPos,
                                       EBloodSpurtType type)
    {
        assert(BMConfigurationService::BMAPI_FunctionAddress_ZHM3Actor_ActivateBloodSpurt != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_FunctionAddress_ZHM3Actor_ActivateBloodSpurt != BMConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(Hitman::BloodMoney::ZHM3Actor*, const Glacier::ZVector3*, const Glacier::ZVector3*, Hitman::BloodMoney::EBloodSpurtType))
            (BMConfigurationService::BMAPI_FunctionAddress_ZHM3Actor_ActivateBloodSpurt))(this, pInvDir, pCollisionPos, type);
        }
    }
}