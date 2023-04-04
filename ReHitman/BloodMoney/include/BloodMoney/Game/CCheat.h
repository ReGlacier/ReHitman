#pragma once

#include <Glacier/GlacierFWD.h>

#include <string_view>

namespace Glacier {
    class CInventory;
}

namespace Hitman::BloodMoney {
    class CCheat
    {
    public:
        static void GiveItem(Glacier::ZREF rItem);
        static void GiveItem(Glacier::ZGROUP* pWeaponsGroup, std::string_view sItemName);
        static void GiveItem(Glacier::CInventory* pInventory, std::string_view sItemName, unsigned int iCount = 1);
        static void GiveSome();
    };
}