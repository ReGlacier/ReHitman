#pragma once

namespace Hitman::BloodMoney
{
    enum EHM3Menu : unsigned int {
        Briefing = 0,
        InGameMenu = 1,
        UnknownDemoMapWindow = 2,
        Map = 3,
        TutorialMenu = 4,
        InventoryMenu = 5,
        LoadResultMenu = 6,
        LoadSuccessMenu = LoadResultMenu,
        LoadErrorMenu = LoadResultMenu
    };
}