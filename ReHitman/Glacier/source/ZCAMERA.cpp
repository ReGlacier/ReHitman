#include <G1ConfigurationService.h>
#include <Glacier/ZCAMERA.h>
#include <cassert>

namespace Glacier
{
	void ZCAMERA::SetFogEnabled(bool* bEnabled)
	{
		assert(G1ConfigurationService::G1API_FunctionAddress_ZCAMERA_SetFogEnabled != G1ConfigurationService::kNotConfiguredOption);

		if (G1ConfigurationService::G1API_FunctionAddress_ZCAMERA_SetFogEnabled != G1ConfigurationService::kNotConfiguredOption)
		{
			((void(__thiscall*)(ZCAMERA*,bool*))G1ConfigurationService::G1API_FunctionAddress_ZCAMERA_SetFogEnabled)(this, bEnabled);
		}
	}

	bool ZCAMERA::IsFogEnabled()
	{
		assert(G1ConfigurationService::G1API_FunctionAddress_ZCAMERA_IsFogEnabled != G1ConfigurationService::kNotConfiguredOption);

		if (G1ConfigurationService::G1API_FunctionAddress_ZCAMERA_IsFogEnabled != G1ConfigurationService::kNotConfiguredOption)
		{
			return ((bool(__thiscall*)(ZCAMERA*))G1ConfigurationService::G1API_FunctionAddress_ZCAMERA_IsFogEnabled)(this);
		}

		return false;
	}
}