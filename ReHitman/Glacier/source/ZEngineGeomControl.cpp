#include <Glacier/ZEngineGeomControl.h>
#include <G1ConfigurationService.h>
#include <Glacier/Glacier.h>

namespace Glacier
{
    ZEngineGeomControl* ZEngineGeomControl::GetInstance() {
        if (G1ConfigurationService::G1API_InstanceAddress_ZEngineGeomControl != G1ConfigurationService::kNotConfiguredOption) {
            return reinterpret_cast<ZEngineGeomControl*>(G1ConfigurationService::G1API_InstanceAddress_ZEngineGeomControl);
        }

        return nullptr;
    }
}