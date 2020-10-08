#include <BloodMoney/Client.h>
#include <BloodMoney/Debug/BMDebugView.h>
#include <BloodMoney/Debug/BMDebugModel.h>

#include <HF/HackingFramework.hpp>

#include <spdlog/spdlog.h>

namespace Hitman::BloodMoney
{
    static constexpr std::string_view kDirectX9DllName = "d3d9.dll";
    static constexpr std::string_view kProcessName = "HitmanBloodMoney.exe";

    std::shared_ptr<HF::Hook::Trampoline<HF::X86::DEFAULT_JMP_SIZE>> g_trampoline = nullptr;

    void __stdcall OnNewSession(DWORD pInstance)
    {
        spdlog::info("New game session at {:08X}", pInstance);
    }

    bool Client::LocateModules()
    {
        auto PrintModInfo = [](const HF::Win32::Module::Ptr& mod)
        {
            if (!mod) {
                spdlog::error("BAD MODULE");
                return;
            }

            spdlog::info("Module: {} base address at {:08x} of size {:08x}", mod->getName(), mod->getBaseAddress(), mod->getSize());
        };

        m_selfProcess = std::make_shared<HF::Win32::Process>(kProcessName);
        spdlog::info("Trying to locate modules...");

        if (!m_selfProcess->isValid())
        {
            spdlog::error("Failed to locate {} process", kProcessName);
            return false;
        }

        m_selfModule = m_selfProcess->getSelfModule();
        if (!m_selfModule)
        {
            spdlog::error("Failed to locate self module!");
            return false;
        }

        m_d3d9Module = m_selfProcess->getModule(kDirectX9DllName);
        if (!m_d3d9Module)
        {
            spdlog::error("Failed to locate {} module!", kDirectX9DllName);
            return false;
        }

        PrintModInfo(m_selfModule);
        PrintModInfo(m_d3d9Module);

        spdlog::info("All required modules located!");

        return true;
    }

    void Client::ReleaseModules()
    {
        g_trampoline->remove();
        g_trampoline = nullptr;

        m_selfModule = nullptr;
        m_d3d9Module = nullptr;
        m_selfProcess = nullptr;
    }

    bool Client::OnAttach()
    {
        spdlog::info("----------[ WELCOME TO RE:HITMAN PROJECT ]----------");

        if (!LocateModules()) return false;

        // TODO: It is debug, remove later
        const auto kAddr = 0x00604CF1;

        HF::Hook::MoveInstructions<4>(m_selfProcess, kAddr, kAddr + 5);
        HF::Hook::FillMemoryByNOPs(m_selfProcess, kAddr, 5);

        g_trampoline = HF::Hook::HookFunction(
                m_selfProcess,
                kAddr,
                &OnNewSession,
                {
                        HF::X86::PUSH_ECX,
                        HF::X86::PUSH_EAX,
                        HF::X86::PUSH_EAX
                },
                {
                        HF::X86::POP_EAX,
                        HF::X86::POP_EAX
                });

        if (g_trampoline->setup())
            spdlog::info("Hook inited at {:08X}", g_trampoline->at());
        else
            spdlog::error("Failed to setup hook at {:08X}", g_trampoline->at());


        return true;
    }

    void Client::OnDestroy()
    {
        ReleaseModules();
    }

    ReHitman::Client::IDebugView * Client::GetDebugView()
    {
        if (!m_debugView)
        {
            auto view = new DebugUI::BMDebugView();
            view->SetModel(new DebugUI::BMDebugModel); ///TODO: Take infos from ZHM3GameData here
            m_debugView = view;
        }

        return m_debugView;
    }
}