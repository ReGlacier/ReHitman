#pragma once

#include <memory>
#include <Client/IClient.h>
#include <Client/IDebugView.h>

namespace HF::Win32 {
    class Module; using ModulePtr = std::shared_ptr<Module>;
    class Process; using ProcessPtr = std::shared_ptr<Process>;
}

namespace Hitman::BloodMoney
{
    class Client : public ReHitman::Client::IClient
    {
    public:
        bool OnAttach() override;
        void OnDestroy() override;

        ReHitman::Client::IDebugView* GetDebugView() override;

    private:
        bool LocateModules();
        void ReleaseModules();

    private:
        ReHitman::Client::IDebugView* m_debugView { nullptr };
        HF::Win32::ProcessPtr m_selfProcess { nullptr };
        HF::Win32::ModulePtr m_selfModule { nullptr };
        HF::Win32::ModulePtr m_d3d9Module { nullptr };
    };
}