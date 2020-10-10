#pragma once

#include <memory>
#include <vector>
#include <BloodMoney/Patches/BasicPatch.h>
#include <HF/HackingFrameworkFWD.h>

namespace Hitman::BloodMoney
{
    class CommonPatches
    {
    private:
        HF::Win32::ProcessRef m_process;
        HF::Win32::ModuleRef m_selfMod;
        HF::Win32::ModuleRef m_d3d9Mod;
        bool m_isInited { false };
        std::vector<BasicPatch::Ptr> m_patches;
    public:
        using Ptr = std::shared_ptr<CommonPatches>;

        CommonPatches(
                const HF::Win32::ProcessPtr& process,
                const HF::Win32::ModulePtr& selfModule,
                const HF::Win32::ModulePtr& d3d9
        );

        bool Setup();
        void Release();

        template <typename PatchType, typename... Args>
        void RegisterPatch(Args&&... args) requires (std::is_constructible_v<PatchType, Args...> && std::is_base_of_v<BasicPatch, PatchType>)
        {
            //TODO: May be emplace_back?
            auto patch = std::make_shared<PatchType>(std::forward<Args>(args)...);
            m_patches.push_back(patch);
        }
    };

}