#pragma once

namespace ReHitman
{
    class Core
    {
    public:
        static int EntryPoint(const void*);

    private:
        enum class GameVersion {
            BloodMoney_PC_1_0_USA,
            BloodMoney_PC_1_0_RU_ND, //Russian Build, New Disk retailer
            BloodMoney_PC_1_2_UK,
            BloodMoney_PC_1_2_Steam,
            BloodMoney_PC_1_2_GOG,
            // Not a version
            UnknownBuild = 0xDEAD
        };

        static GameVersion TryToDetectGameVersion();
    };
}