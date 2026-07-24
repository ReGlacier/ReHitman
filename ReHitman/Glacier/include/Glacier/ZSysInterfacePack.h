#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZDllBase;

    class ZSysInterfacePack : public ZSysInterface
    {
    public:
        // vtbl
        ~ZSysInterfacePack() override;
        void ParseOptions() override;
        void SetEngineData(ZEngineDataBase* pEngineData) override;

        // methods
        ZSysInterfacePack(int hInstance);

        // members (starts at +0xB48)
        MYSTR m_sPackDestinationPath; // 0xB48 - Approved by Ctor
        MYSTR m_sPackFile; // 0xBC8
        MYSTR m_sPackPlatform; // 0xC48
        MYSTR m_sTextureExtension; // 0xCC8
        bool m_sAlwaysPack; // 0xD48
        bool m_bStillFrame; // 0xD49
        bool m_bUseTryCatch; // 0xD5A
        RE_ADD_PADDING(1);
        int32_t m_iNumExcept; // 0xD4C
        int32_t m_lReplayBufferSize; // 0xD50
        ZDllBase* m_pMainDll; // 0xD54
        char* m_pReplayBuffer; // 0xD58
        char* m_pReplayPointer; // 0xD5C
        MYSTR m_sErrorLog; // 0xD60
    };
    RE_VERIFY_SIZE(ZSysInterfacePack, 0xDE0); // TODO: Need verify it carefuly
    RE_VERIFY_OFFSET(ZSysInterfacePack, m_sPackDestinationPath, 0xB48);
    RE_VERIFY_OFFSET(ZSysInterfacePack, m_sPackFile, 0xBC8);
    RE_VERIFY_OFFSET(ZSysInterfacePack, m_sPackPlatform, 0xC48);
}