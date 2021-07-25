#include <BloodMoney/Patches/CommonPatches.h>
#include <HF/HackingFramework.hpp>
#include <spdlog/spdlog.h>


namespace Hitman::BloodMoney
{
    CommonPatches::CommonPatches(
            const HF::Win32::ProcessPtr& process,
            const HF::Win32::ModulePtr& selfModule,
            const HF::Win32::ModulePtr& d3d9
    )
        : m_process(process)
        , m_selfMod(selfModule)
        , m_d3d9Mod(d3d9)
    {
    }

    bool CommonPatches::Setup()
    {
        if (m_isInited)
        {
            spdlog::warn("CommonPatches::Setup(). Module is already inited!");
            return false;
        }

        ModPack modPack { m_process.lock(), m_selfMod.lock(), m_d3d9Mod.lock() };
        for (const auto& patch : m_patches)
        {
            if (!patch->IsApplied())
            {
                if (!patch->Apply(modPack))
                {
                    spdlog::warn("Failed to apply patch {}", patch->GetName().data());
                }
                else
                {
                    spdlog::info("Patch {} applied!", patch->GetName().data());
                    if (!patch->IsApplied())
                    {
                        spdlog::error("BAD PATCH IMPLEMENTATION! PATCH {} MUST BE APPLIED BUT IT DOESN'T!", patch->GetName());
                        continue;
                    }
                }
            }
        }

        m_isInited = true;
        return true;
    }

    void CommonPatches::Release()
    {
        if (m_isInited)
        {
            ModPack modPack { m_process.lock(), m_selfMod.lock(), m_d3d9Mod.lock() };

            for (const auto& patch : m_patches)
            {
                if (patch->IsApplied())
                {
                    patch->Revert(modPack);
                }

                if (patch->IsApplied())
                {
                    spdlog::error("BAD PATCH IMPLEMENTATION! PATCH {} MUST BE REVERTED BUT IT DOESN'T!", patch->GetName().data());
                }
            }

            m_isInited = false;
        }
    }
}