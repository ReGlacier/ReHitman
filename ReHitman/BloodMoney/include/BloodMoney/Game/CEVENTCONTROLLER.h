#pragma once

#include <Glacier/ZSTL/LINKREFTAB.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ZSTL/REFTAB.h>


namespace Hitman::BloodMoney 
{
    struct SActor 
    {
        int32_t lType;
        Glacier::ZGEOM* pActor;
        Glacier::TIMETYPE vLastCheckTime;
        struct SActor* m_pNext;
        struct SActor* m_pPrev;
    };
    RE_VERIFY_SIZE(SActor, 0x14); // Verified

    struct CEVENTCONTROLLER 
    {
        //data (total size is 0x18)
        Glacier::REFTAB* m_prtEventList;
        Glacier::LINKREFTAB* m_prtActorList;
        int32_t m_lUpdateCount;
        SActor* m_pActors;
        SActor* m_pNextOnScreen;
        SActor* m_pNextOffScreen;
    };
    RE_VERIFY_SIZE(CEVENTCONTROLLER, 0x18); // Verified
}