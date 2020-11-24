#include <BloodMoney/Patches/All/EnableCheatsPatch.h>
#include <BloodMoney/Delegates/IInputDelegate.h>

#include <array>

namespace Hitman::BloodMoney
{
    namespace Consts
    {
        static constexpr std::array<uint8_t, 4> kFirstPatchPayload = { 0x90, 0x90, 0x90, 0x90 };
        static constexpr size_t kFirstPatchAddr = 0x00448312;
        static constexpr size_t kFirstPatchSize = kFirstPatchPayload.size();

        static constexpr std::array<uint8_t, 7> kSecondPatchPayload = { 0xC6, 0x05, 0x89, 0xCA, 0x8A, 0x00, 0x01 };
        static constexpr size_t kSecondPatchAddr = 0x00448316;
        static constexpr size_t kSecondPatchSize = kSecondPatchPayload.size();
    }

    std::string_view EnableCheatsPatch::GetName() const { return "CheatsEnabler Patch"; }

    bool EnableCheatsPatch::Apply(const ModPack& modules)
    {
        if (auto process = modules.process.lock())
        {
            m_firstPatch.reserve(Consts::kFirstPatchSize);
            m_secondPatch.reserve(Consts::kSecondPatchSize);

            process->readMemory(Consts::kFirstPatchAddr, Consts::kFirstPatchSize, m_firstPatch.data());
            process->readMemory(Consts::kSecondPatchAddr, Consts::kSecondPatchSize, m_secondPatch.data());

            process->writeMemory(Consts::kFirstPatchAddr, Consts::kFirstPatchSize, Consts::kFirstPatchPayload.data());
            process->writeMemory(Consts::kSecondPatchAddr, Consts::kSecondPatchSize, Consts::kSecondPatchPayload.data());

            return BasicPatch::Apply(modules);
        }

        return false;
    }

    void EnableCheatsPatch::Revert(const ModPack& modules)
    {
        if (auto process = modules.process.lock())
        {
            process->writeMemory(Consts::kFirstPatchAddr, Consts::kFirstPatchSize, m_firstPatch.data());
            m_firstPatch.clear();

            process->writeMemory(Consts::kSecondPatchAddr, Consts::kSecondPatchSize, m_secondPatch.data());
            m_secondPatch.clear();
        }
    }
}