#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>

namespace Glacier
{
    struct IInputStream;

    // Engine-level descriptor of the saved-game control/data streams used while
    // loading a save game. A single concrete instance (e.g. ZLoadGameInfoPC) is
    // created by the save-game code and registers itself in m_Instance.
    // ZEngineDataBase::AllocSequence() probes CreateControlStream() to detect a
    // load-from-save (m_LoadingGame).
    //
    // Reversed from PC (0x00761450 vftable / 0x008BE150 m_Instance) and the PS2
    // base (which shares this exact base; the platform concrete class there is
    // ZLoadGameInfoPS2 and is intentionally not used here).
    class ZLoadGameInfoBase
    {
    public:
        // static
        STATIC_CLASS_VAR(ZLoadGameInfoBase, ZLoadGameInfoBase*, m_Instance); // PC 0x008BE150

        // vtbl (PC 0x00761450)
        virtual ~ZLoadGameInfoBase();
        virtual IInputStream* CreateStream(const char* pszFilename) = 0;

        // methods
        ZLoadGameInfoBase();
        void SetFilenames(const char* pszControlStreamName, const char* pszDataStreamName);

        // static methods
        static IInputStream* CreateControlStream();
        static IInputStream* CreateDataStream();
        static void Destroy();

        // members
        char m_ControlStreamFilename[256];
        char m_DataStreamFilename[256];
    };
    RE_VERIFY_SIZE(ZLoadGameInfoBase, 0x204);
}
