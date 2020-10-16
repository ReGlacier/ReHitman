#include <BloodMoney/Engine/ZBoidSystem.h>

namespace Hitman::BloodMoney
{
    namespace Consts
    {
        static constexpr std::intptr_t kAddBoidFunctionAddr = 0x00585610;
        static constexpr std::intptr_t kFrameUpdateFunctionAddr = 0x00585490;
    }

    ZBoid* ZBoidSystem::AddBoid(ZBoid* boid)
    {
        using ZBoidSystem_AddBoid_t = ZBoid*(__thiscall* )(ZBoidSystem*, ZBoid*);
        auto func = reinterpret_cast<ZBoidSystem_AddBoid_t>(Consts::kAddBoidFunctionAddr);
        return func(this, boid);
    }

    void ZBoidSystem::FrameUpdate()
    {
        using ZBoidSystem_FrameUpdate_t = void(__thiscall*)(ZBoidSystem*);
        auto func = reinterpret_cast<ZBoidSystem_FrameUpdate_t>(Consts::kFrameUpdateFunctionAddr);
        func(this);
    }
}