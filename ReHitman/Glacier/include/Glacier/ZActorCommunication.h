#pragma once

#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZBoxPrimitive.h>

namespace Glacier {
    class ZLIST;

    class ZActorCommunication : public CBaseEvent<ZBoxPrimitive> {
    public:
        //structures
        struct RADIOUSER {
            unsigned int rActor;
            unsigned int iChannel;
        };

        //vftable (no changes)
        //data (size is 0x358, base size is 0x30)
        RADIOUSER m_aRadioUsers[100];        //+0x30
        unsigned int m_iListenersCount;      //+0x350
        Glacier::ZLIST* m_pTrackLinkObjects; //+0x354

        //api methods
        void RegisterRadioUser(Glacier::ZREF rActor, unsigned int iChannel);
    };
}