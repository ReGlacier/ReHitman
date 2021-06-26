#include <Glacier/CConfiguration.h>
#include <G1ConfigurationService.h>

namespace Glacier
{
    bool CConfiguration::CanShowSubtitles() {
        if (G1ConfigurationService::G1API_InstanceAddress_CConfiguration_bSubtitles != G1ConfigurationService::kNotConfiguredOption) {
            return *reinterpret_cast<bool*>(G1ConfigurationService::G1API_InstanceAddress_CConfiguration_bSubtitles);
        }
        return false;
    }

    void CConfiguration::SetCanShowSubtitles(bool value) {
        if (G1ConfigurationService::G1API_InstanceAddress_CConfiguration_bSubtitles != G1ConfigurationService::kNotConfiguredOption) {
            *reinterpret_cast<bool*>(G1ConfigurationService::G1API_InstanceAddress_CConfiguration_bSubtitles) = value;
        }
    }
}