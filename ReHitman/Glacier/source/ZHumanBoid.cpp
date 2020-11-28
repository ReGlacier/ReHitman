#include <G1ConfigurationService.h>
#include <Glacier/ZHumanBoid.h>
#include <cassert>

namespace Glacier
{
    void ZHumanBoid::SetTarget(Glacier::ZVector3* position, Glacier::Vector3* rotation, float a4, bool force)
    {
        assert(G1ConfigurationService::GetInstance().G1API_FunctionAddress_ZHumanBoid_SetTarget != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::GetInstance().G1API_FunctionAddress_ZHumanBoid_SetTarget != G1ConfigurationService::kNotConfiguredOption)
        {
            using ZHumanBoid_SetTarget = void(__thiscall*)(ZHumanBoid*, Glacier::ZVector3*, Glacier::ZVector3*, float, bool);
            auto ZHumanBoid_SetTargetImpl = (ZHumanBoid_SetTarget)G1ConfigurationService::GetInstance().G1API_FunctionAddress_ZHumanBoid_SetTarget;
            ZHumanBoid_SetTargetImpl(this, position, rotation, a4, force);
        }
    }

}