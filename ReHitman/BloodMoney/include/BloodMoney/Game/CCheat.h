#pragma once

#include <Glacier/GlacierFWD.h>

#include <string_view>

namespace Hitman::BloodMoney {
    class CCheat
    {
    public:
        static void GiveItem(Glacier::ZREF rItem);
        static void GiveItem(Glacier::ZGROUP* pWeaponsGroup, std::string_view sItemName);
        static void GiveSome();
    };
}