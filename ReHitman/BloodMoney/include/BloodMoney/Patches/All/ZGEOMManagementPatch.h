#pragma once

#include <BloodMoney/Patches/BasicPatch.h>
#include <HF/HackingFramework.hpp>

namespace Hitman::BloodMoney
{
    class ZGEOMManagementPatch final : public BasicPatch
    {
    private:
        static constexpr std::intptr_t CtorPatchSize = 7;
        static constexpr std::intptr_t DtorPatchSize = 6;

        HF::Hook::TrampolinePtr<CtorPatchSize>  m_ZGEOM_Ctor;
        HF::Hook::TrampolinePtr<DtorPatchSize>  m_ZGEOM_Dtor;

    public:
        std::string_view GetName() const override;
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;
    };
}