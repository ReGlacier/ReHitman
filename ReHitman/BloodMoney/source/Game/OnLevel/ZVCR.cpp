#include <BloodMoney/Game/OnLevel/ZVCR.h>
#include <BloodMoney/BMConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney
{
    void ZVCR::SetTapeStolen(bool value) {
        assert(BMConfigurationService::BMAPI_GlobalVariableAddress_ZVCR_m_msgStealTape != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_GlobalVariableAddress_ZVCR_m_msgStealTape != BMConfigurationService::kNotConfiguredOption)
        {
            auto pVariable = reinterpret_cast<bool*>(BMConfigurationService::BMAPI_GlobalVariableAddress_ZVCR_m_msgStealTape);
            *pVariable = value;
        }
    }

    bool ZVCR::TapeWasStolen() {
        assert(BMConfigurationService::BMAPI_GlobalVariableAddress_ZVCR_m_msgStealTape != BMConfigurationService::kNotConfiguredOption);
        if (BMConfigurationService::BMAPI_GlobalVariableAddress_ZVCR_m_msgStealTape != BMConfigurationService::kNotConfiguredOption)
        {
            auto pVariable = reinterpret_cast<bool*>(BMConfigurationService::BMAPI_GlobalVariableAddress_ZVCR_m_msgStealTape);
            return *pVariable;
        }

        return false;
    }
}