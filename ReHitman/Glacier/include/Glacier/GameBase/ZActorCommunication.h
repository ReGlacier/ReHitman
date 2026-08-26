#pragma once

#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZBoxPrimitive.h>

namespace Glacier {
    class ZLIST;

    class ZActorCommunication : public CBaseEvent<ZBoxPrimitive> {
    public:
        //structures
        struct sRadioOwner {
            ZREF rUser;
            unsigned int iChannel;
        };

        //vftable (no changes)
        //data (size is 0x358, base size is 0x30)
        sRadioOwner m_aRadioUsers[100];
        unsigned int m_iNrOfRadioUsers;
        ZLIST* m_pActorList;

        //api methods
        void RegisterRadioUser(ZREF rActor, unsigned int iChannel);
    };
}