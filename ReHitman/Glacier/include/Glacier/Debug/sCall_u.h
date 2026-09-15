#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Debug/ZDebug.h>
#include <cstdint>


namespace Glacier
{
    enum uChannelEnums
    {
        _chReserved = 0,
        chAnimation = 1,
        chAudio = 2,
        chCutScene = 3,
        chGame = 4,
        chGUI = 5,
        chIPC = 6,
        chNet = 7,
        chPathfinder = 8,
        chRender = 9,
        chRobot = 10,
        chScene = 11,
        chScript = 12,
        chSystem = 13,
        chStatistics = 14,
        chTool = 15,
        chPhysics = 16,
        chCollision = 17,
        chDeprecated = 18,
        _chLast = 19,
    };

    class sCall_u
    {
    public:
        // methods
        sCall_u(uint32_t iLine, const char* szFile, ZDebug::uSeverityEnums eSeverity, bool bDisplayOnce, bool bIsConsoleLine);
        void operator->();

        void _uPrint(const char* fmt, ...);
        void _uMsg(uChannelEnums eChannel, const char* fmt, ...);

        // members
        bool m_bIsConsoleLine;
        bool m_bDisplayOnce;
        RE_ADD_PADDING(2);
        const char* m_szFile;
        uint32_t m_iLine;
        ZDebug::uSeverityEnums m_eSeverity;
    };
    RE_VERIFY_SIZE(sCall_u, 0x10); // Verified all platforms
}