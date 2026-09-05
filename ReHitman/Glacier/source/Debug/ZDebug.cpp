#include <Glacier/Debug/ZDebug.h>
#include <cstring>
#include <cstdlib>


namespace Glacier
{
    const char* ZDebug::pszChannelStrings[20] {
        "_chReserved",
        "chAnimation",
        "chAudio",
        "chCutscene",
        "chGame",
        "chGUI",
        "chIPC",
        "chNet",
        "chRender",
        "chRobot",
        "chScene",
        "chScript",
        "chSystem",
        "chStatistics",
        "chTool",
        "chPhysics",
        "chCollision",
        "chDeprecated",
        "_chLast"
    };

    ZDebug::uSeverityEnums ZDebug::g_eChannelSeverity[19]
    {
        eSEV_MSG, eSEV_MSG, eSEV_MSG, 
        eSEV_MSG, eSEV_MSG, eSEV_MSG, 
        eSEV_MSG, eSEV_MSG, eSEV_MSG, 
        eSEV_MSG, eSEV_MSG, eSEV_MSG, 
        eSEV_MSG, eSEV_MSG, eSEV_MSG, 
        eSEV_MSG, eSEV_MSG, eSEV_MSG, 
        eSEV_MSG
    };

    void ZDebug::SetDebugSeverity(const char* pszSeverity)
    {
        if (!pszSeverity)
            return;

        uSeverityEnums eSeverity = static_cast<uSeverityEnums>(atol(pszSeverity) + 1);

        for (int i = 0; i < 19; ++i)
        {
            g_eChannelSeverity[i] = eSeverity;
        }
    }

    void ZDebug::SetDebugChannels(const char* pszChannels)
    {
        if (!pszChannels || !*pszChannels)
            return;

        char cleanBuffer[1024]; 
        char* pDst = cleanBuffer;

        const char* pSrc = pszChannels;
        while (*pSrc != '\0')
        {
            if (*pSrc != ' ')
            {
                *pDst = *pSrc;
                ++pDst;
            }
            ++pSrc;
        }
        *pDst = '\0';

        for (int i = 0; i < 19; ++i)
        {
            const char* pFound = strstr(cleanBuffer, pszChannelStrings[i]);
            if (pFound)
            {
                uSeverityEnums eSeverity = eSEV_WARN3;

                const char* pColon = strchr(pFound, ':');
                if (pColon)
                {
                    eSeverity = static_cast<uSeverityEnums>(atol(pColon + 1) + 1);
                }

                g_eChannelSeverity[i] = eSeverity;
            }
        }
    }
}