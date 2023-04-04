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

    Core::GameVersion Core::TryToDetectGameVersion()
    {
        struct VersionDef
        {
            std::string_view Id;
            std::intptr_t StrAddr;
        };

        static constexpr size_t kMaxVersionLen = 32;
        static constexpr std::intptr_t kUnknownAddr = 0xDEADB33F;

        static std::pair<Core::GameVersion, VersionDef> PossibleGameVersions[] = {
                { Core::GameVersion::BloodMoney_PC_1_2_GOG, { "blood-build3-20060616-26123", 0x007562C0 } },
                { Core::GameVersion::BloodMoney_PC_1_2_Steam, { "blood-build3-20060601-26089", kUnknownAddr } },
                { Core::GameVersion::BloodMoney_PC_1_2_UK, { "blood-build3-20060616-26123", kUnknownAddr } },
                { Core::GameVersion::BloodMoney_PC_1_0_USA, { "blood-build4-20060508-25990", kUnknownAddr } },
                { Core::GameVersion::BloodMoney_PC_1_0_RU_ND, { "blood-build3-20060713-26152", 0x007562C0 } }
        };

        for (const auto& [gameVersion, versionInfo] : PossibleGameVersions)
        {
            if (versionInfo.StrAddr == kUnknownAddr) continue; // SKip, unable to check it

            const auto valueInGame = std::string_view { reinterpret_cast<const char*>(versionInfo.StrAddr) };
            if (valueInGame == versionInfo.Id)
                return gameVersion;
        }

        return Core::GameVersion::UnknownBuild;
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

        const auto gameVersion = TryToDetectGameVersion();
        bool isOkVersion = false;

        switch (gameVersion)
        {
            case GameVersion::BloodMoney_PC_1_2_GOG:
                spdlog::info("Core::EntryPoint() | Detected GOG version. Version is OK");
                isOkVersion = true;
                break;
            case GameVersion::BloodMoney_PC_1_0_RU_ND:
                spdlog::info("Core::EntryPoint() | Detected Russian version. This version isn't supported yet.");
                break;
            case GameVersion::BloodMoney_PC_1_0_USA:
                spdlog::info("Core::EntryPoint() | Detected USA 1.0 version. This version isn't supported yet.");
                break;
            case GameVersion::BloodMoney_PC_1_2_UK:
                spdlog::info("Core::EntryPoint() | Detected UK 1.2 version. This version isn't supported yet.");
                break;
            case GameVersion::BloodMoney_PC_1_2_Steam:
                spdlog::error("Core::EntryPoint() Sorry, the Steam version not supported because their executable was packed by developers\n"
                              "If you know how to unpack it please refer to this issue on GitHub: https://github.com/ReGlacier/ReHitman/issues/1");
                break;
            case GameVersion::UnknownBuild:
                spdlog::error("Core::EntryPoint() Failed. Unknown or invalid game version");
                break;
        }

        if (!isOkVersion)
        {
            spdlog::error(" *** The game version was rejected. ReHitman will shutdown, the game will work normally! *** ");

            MessageBox(nullptr, "Sorry, but this game not supported yet", "ReHitman| Game not supported", MB_ICONEXCLAMATION | MB_OK);
            delete g_ClientInterface;
            ReHitman::Logger::Shutdown();
            return EXIT_FAILURE;
        }

        if (!g_ClientInterface->OnAttach())
        {
            RH_ASSERT2(g_ClientInterface->OnAttach(), "g_ClientInterface->OnAttach() failed! See console for details!");
            ReHitman::Logger::Shutdown();
            return EXIT_FAILURE;
        }

        g_ClientInterface->Run();

		while (true)
		{
		}

//        while (g_ClientInterface->IsOnline())
//        {
//        }

        g_ClientInterface->OnDestroy();

        spdlog::info("Core::EntryPoint() shutdown...");

        ReHitman::Logger::Shutdown();

        return EXIT_SUCCESS;
    }
}