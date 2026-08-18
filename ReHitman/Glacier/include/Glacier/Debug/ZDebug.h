#pragma once


namespace Glacier
{
    class ZDebug
    {
    public:
        // types
        enum uSeverityEnums
        {
            eSEV_PRINT = 0x0,
            eSEV_ERROR = 0x1,
            eSEV_WARN1 = 0x2,
            eSEV_WARN2 = 0x3,
            eSEV_WARN3 = 0x4,
            eSEV_MSG   = 0x5,  
        };

        enum eType
        {
            _UPRINTF = 0x0,
            _MEMORY = 0x1,
            _UWATCH = 0x2,
            _UTIMER = 0x3,
            _UVAR = 0x4,
            _SCRIPTFLIGHTRECORDER = 0x5,
            _NUM_TYPES = 0x6,
            _SYSTEM = 0xFF,
        };

        enum eTypeType
        {
            _NONE = 0x1,
            _MINIMIZE = 0x2,
            _RESTORE = 0x3,
            _MAXIMIZE = 0x4,
            _FRAME = 0x0,
            _ERROR = 0x1,
            _WARNING = 0x2,
            _MESSAGE = 0x3,
            _UNKNOWN = 0x4,
            _WARNING_ONCE = 0x5,
            _MESSAGE_ONCE = 0x6,
            _UNKNOWN_ONCE = 0x7,
            _CLEAR = 0x8,
            _NEW = 0x1,
            _FREE = 0x2,
            _SCRIPTMESSAGE = 0x1,
            _SCRIPTSWITCHSTATE = 0x2,
        };

        // static
        static const char* pszChannelStrings[20];
        static uSeverityEnums g_eChannelSeverity[19];

        // methods
        static void SetDebugSeverity(const char* pszSeverity);
        static void SetDebugChannels(const char* pszChannels);
    };
}