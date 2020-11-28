#pragma once

#include <memory>
#include <Client/IClient.h>
#include <BloodMoney/Patches/CommonPatches.h>
#include <HF/HackingFrameworkFWD.h>

namespace Hitman::BloodMoney
{
    class Client : public ReHitman::Client::IClient
    {
    public:
        bool OnAttach() override;
        void OnDestroy() override;

    private:
        bool RegisterGameConfigurationForGlacier();
        bool LocateModules();
        void ReleaseModules();
        void RegisterPatches();

    private:
        HF::Win32::ProcessPtr m_selfProcess { nullptr };
        HF::Win32::ModulePtr m_selfModule { nullptr };
        HF::Win32::ModulePtr m_d3d9Module { nullptr };
        CommonPatches::Ptr m_patches { nullptr };
    };
}