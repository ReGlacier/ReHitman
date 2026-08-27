#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZBoxPrimitive.h>
#include <Glacier/Runtime/Macro.h>
#include <cstdint>


namespace Glacier
{
    class ZLIST;

    class ZActorCommunication : public CBaseEvent<ZBoxPrimitive>
    {
    public:
        // RTTI
        DECLARE_ROUT_CLASS(ZActorCommunication, ZBoxPrimitive, ActorCommunication, 0, 0);

        // types
        struct sRadioOwner
        {
            ZREF rUser;
            uint32_t iChannel;
        };

        // vtbl
        ~ZActorCommunication() override;
        bool PostLoad(ISerializerStream& stream) override;
        void PostSave(ISerializerStream& stream) override;
        RTP::ZPropertyInfo& GetProperties() const override;
        void Init() override;
        void Init2() override;
        void CopyData(const ZEventBase* Source) override;

        // methods
        ZActorCommunication();
        void RegisterRadioUser(ZREF rActor, uint32_t iChannel);
        bool IsReceiverDead(ZGEOM*) const;
        void CalculateDistances(unsigned int, int, float*, float*);
        void SendRadioMessage(unsigned int, int, short unsigned int, void*);
        void SendRadioMessageToClosestN(unsigned int, int, int, short unsigned int, void*);
        float SendRadioMessageToNeededForce(unsigned int, int, float, short unsigned int, void*);
        void SendRangedMessage(unsigned int, float, short unsigned int, void*);
        void SendGlobalEvent(unsigned int, short unsigned int, void*);
        static void SendEventToActorsInBox();
        static void SendEventToActorsInBox2();

        // members
        sRadioOwner m_aRadioUsers[100];
        int32_t m_iNrOfRadioUsers;
        ZLIST* m_pActorList;
    };
    RE_VERIFY_SIZE(ZActorCommunication, 0x358);
}
