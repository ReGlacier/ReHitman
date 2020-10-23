#include <DLL/ReHitman.h>
#include <DLL/Logger.h>
#include <DLL/DebugConsole.h>
#include <DLL/CrashHandlerReporter.h>
#include <Client/IClient.h>
#ifdef REHITMAN_BLOOD_MONEY_PROJECT
#include <BloodMoney/Client.h>
#endif

#include <Windows.h>

#include <spdlog/spdlog.h>

namespace ReHitman
{
    Client::IClient* g_ClientInterface = nullptr;

    Client::IClient* CreateClientInterface()
    {
#ifdef REHITMAN_BLOOD_MONEY_PROJECT
        return new Hitman::BloodMoney::Client();
#endif
        return nullptr;
    }

    int Core::EntryPoint(const void*)
    {
        ReHitman::DebugConsole::Create("ReHitman | Developer Console");
        ReHitman::Logger::Setup();

        CrashHandlerReporter crashHandlerReporter;

        // Setup client core
        g_ClientInterface = CreateClientInterface();
        RH_ASSERT2(g_ClientInterface != nullptr, "Bad g_ClientInterface interface! Probably CreateClientInterface() failed!");
        if (!g_ClientInterface)
            return EXIT_FAILURE;

        spdlog::info("Core::EntryPoint() run ...");

        if (!g_ClientInterface->OnAttach())
        {
            RH_ASSERT2(g_ClientInterface->OnAttach(), "g_ClientInterface->OnAttach() failed! See console for details!");
            ReHitman::Logger::Shutdown();
            return EXIT_FAILURE;
        }

        g_ClientInterface->Run();

        while (g_ClientInterface->IsOnline())
        {
        }

        g_ClientInterface->OnDestroy();

        spdlog::info("Core::EntryPoint() shutdown...");

        ReHitman::Logger::Shutdown();

        return EXIT_SUCCESS;
    }
}