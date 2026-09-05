#include <Glacier/GameBase/ZActorCommunication.h>
#include <Glacier/RTP/VirtualTables.h>


namespace Glacier
{
    ZActorCommunication::ZActorCommunication()
        : CBaseEvent<ZBoxPrimitive>()
    {
        // TODO: Finish me
    }

    ZActorCommunication::~ZActorCommunication()
    {
        // TODO: Finish me
    }

    bool ZActorCommunication::PostLoad(ISerializerStream& stream)
    {
        // TODO: Finish me
        return false;
    }

    void ZActorCommunication::PostSave(ISerializerStream& stream)
    {
        // TODO: Finish me
    }

    RTP::ZPropertyInfo& ZActorCommunication::GetProperties() const
    {
        return ZActorCommunication::Info;
    }

    void ZActorCommunication::Init()
    {
        // TODO: Finish me
    }

    void ZActorCommunication::Init2()
    {
        // TODO: Finish me
    }

    void ZActorCommunication::CopyData(const ZEventBase* Source)
    {
        // TODO: Finish me
    }

    void ZActorCommunication::RegisterRadioUser(ZREF rActor, uint32_t iChannel)
    {
        // TODO: Finish me
    }

    bool ZActorCommunication::IsReceiverDead(ZGEOM*) const
    {
        // TODO: Finish me
        return true;
    }

    void ZActorCommunication::CalculateDistances(unsigned int, int, float*, float*)
    {
        // TODO: Finish me
    }

    void ZActorCommunication::SendRadioMessage(unsigned int, int, short unsigned int, void*)
    {
        // TODO: Finish me
    }

    void ZActorCommunication::SendRadioMessageToClosestN(unsigned int, int, int, short unsigned int, void*)
    {
        // TODO: Finish me
    }

    float ZActorCommunication::SendRadioMessageToNeededForce(unsigned int, int, float, short unsigned int, void*)
    {
        // TODO: Finish me
        return 0.0f;
    }

    void ZActorCommunication::SendRangedMessage(unsigned int, float, short unsigned int, void*)
    {
        // TODO: Finish me
    }

    void ZActorCommunication::SendGlobalEvent(unsigned int, short unsigned int, void*)
    {
        // TODO: Finish me
    }

    void ZActorCommunication::SendEventToActorsInBox()
    {
        // TODO: Finish me
    }

    void ZActorCommunication::SendEventToActorsInBox2()
    {
        // TODO: Finish me
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZDataProperty<int> NamespaceItem_1337
        {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "m_iNrOfRadioUsers",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__14,
            .m_Offset = CLASS_PROPERTY(ZActorCommunication, m_iNrOfRadioUsers)
        };
    }

    DEFINE_ROUT_CLASS(ZActorCommunication, ZBoxPrimitive, ActorCommunication, 0, 0, 0x00814A70, cProperties::NamespaceItem_1337, ZBoxPrimitive);
#   pragma endregion
}
