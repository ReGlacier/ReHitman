#pragma once

#include <BloodMoney/Patches/BasicPatch.h>
#include <HF/HackingFramework.hpp>


namespace Hitman::BloodMoney {
    class FreeFileSystemPatch final : public BasicPatch {
        HF::Hook::TrampolinePtr<10> m_FsZipConstructor;
    public:
        FreeFileSystemPatch() = default;

        std::string_view GetName() const override;
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;
    };
}