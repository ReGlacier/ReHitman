#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/EventBase/ZEventBuffer.h>
#include <Glacier/EventBase/ZEventList.h>
#include <Glacier/EventBase/ZScheduledEvent.h>
#include <Glacier/EventBase/ZScheduledUpdate.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZSkipSaveGroup.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier 
{
    void ISerializerStream::Exchange(const ZToken token, ZEventBase& data)
    {
        const EPropertyType propertyType = GetPropertyType(static_cast<ZSerializable&>(data));
        ExchangeHeader(token, propertyType);
        ExchangeData(&data);
        ExchangeFooter(propertyType);
    }

    void* ZEventBase::operator new(std::size_t size) 
    {
        auto* pEvent = ZEventBuffer::Instance().AllocEventRam(size);
        ZASSERT(pEvent);

        return pEvent;
    }

    void ZEventBase::operator delete(void* ptr) noexcept 
    {
        printf("%s:%d Attempted manual delete!\n", __FILE__, __LINE__);
        ZASSERT(false);
    }

    ZEventBase* ZEventBase::RefToPtr(ZREF rRef)
    {
        return ZEventBuffer::Instance().ConvEventRefToPtr(rRef);
    }

    void ZEventBase::SetPreferedStatus(EStatus eStatus)
    {
        ZEventBase::m_DefaultStatus = eStatus;
    }

    ZEventBase::ZEventBase()
    {
        ZASSERT(!ZEventBase::m_LockCreation);

        m_lRoutCases = 0;
        m_lEventLists = 0;
        m_TimerInterval = 0.1f;
        m_fTimePassed.secs = g_pSysInterface->FrameTime.secs - static_cast<int>(g_pEngineData->m_fEvenOutTimers * m_TimerInterval * -TIMETYPE::kTicksPerSecond);

        g_pEngineData->m_fEvenOutTimers += 0.1f;
        if (g_pEngineData->m_fEvenOutTimers >= 1.0f)
        {
            g_pEngineData->m_fEvenOutTimers = 0.0f;
        }

        m_ClassCall = 0;
        m_Status = EStatus::STATUS_New;
        m_pBaseGeom = nullptr;
        m_pScheduleEvent = nullptr;

        g_pEngineData->NewEventClass(this);
        g_pEngineData->m_EventList.AddEvent(this);

        if (ZEventBase::m_DirectRef)
        {
            m_Ref = ZEventBuffer::Instance().AllocRefDirect(this, ZEventBase::m_DirectRef - 1);
            ZEventBase::m_DirectRef = 0;
        }
        else
        {
            m_Ref = ZEventBuffer::Instance().AllocRef(this);
        }
    }

    const RTP::ZPropertyInfo& ZEventBase::GetProperties() const
    {
        return ZEventBase::Info;
    }

    ZEventBase::EEventPriority ZEventBase::GetEventPriority()
    {
        return PRIORITY_Normal;
    }

    void ZEventBase::Init() 
    {
        // Do nothing
    }

    void ZEventBase::Init2() 
    {
        // Do nothing
    }

    void ZEventBase::PostInit() 
    {
        // Do nothing
    }

    void ZEventBase::PostInit2() 
    {
        // Do nothing
    }

    void ZEventBase::CopyData(const ZEventBase*)
    {
        // Do nothing
    }

    const char* ZEventBase::EventName()
    {
        // It's ok
        return nullptr;
    }

    void ZEventBase::ExpandBounds(ZMat3x3& mMat, ZVector3& vCen, ZVector3& vSize, ZBaseGeom* pBaseGeom) 
    {
        // Do nothing
    }

    void ZEventBase::PreSaveGame() 
    {
        // Do nothing
    }

    void ZEventBase::RegisterInstance()
    {
        // Do nothing
    }
    
    void ZEventBase::CheckPointSave(ZCheckPointBuffer&) 
    {
        // Do nothing
    }
    void ZEventBase::CheckPointLoad(ZCheckPointBuffer&)
    {
        // Do nothing
    }

    void ZEventBase::Reset()
    {
        // Do nothing
    }

    void ZEventBase::TimeUpdate() 
    {
        // Do nothing
    }
    
    void ZEventBase::FrameUpdate() 
    {
        // Do nothing
    }

    int ZEventBase::Command(Glacier::ZMSGID, Glacier::ZDATA)
    {
        return 0;
    }

    int ZEventBase::DoEvent(int lType, uint16_t lParam, void* pData)
    {
        return 0;
    }

    void ZEventBase::End() 
    {
        // Do nothing
    }

    void ZEventBase::EditorCommand(ZMSGID command, ZDATA args) 
    {
        // Do nothing, but original game contains weird print "INEDITOR"
        printf("engine/geomsbase/baseevent.h:247 | INEDITOR\n");
    }

    void ZEventBase::Remove() 
    {
        ChangeStatus(EStatus::STATUS_Remove, false);
    }

    void ZEventBase::SchedUpdate() 
    {
        // Do nothing
    }

    void ZEventBase::ActivateFrameUpdate(bool run_when_pause) 
    {
        uint32_t iMask = m_lRoutCases | 0x100;

        if (!run_when_pause)
        {
            iMask &= 0xFFFFFEFF;
        }

        m_lRoutCases = iMask;


        if ((iMask & 0x10) == 0)
        {
            m_lRoutCases = iMask | 0x10;
        }

        ChangeEventActivity();
    }

    void ZEventBase::DeactivateFrameUpdate() 
    {
        m_lRoutCases &= ~0x10;
        ChangeEventActivity();
    }

    void ZEventBase::ChangeEventActivity() 
    {
        uint32_t lRoutCases = m_lRoutCases;

        // Inactive geoms are not allowed to receive frame/time callbacks. The original
        // code masks the requested rout cases locally, but keeps m_lRoutCases intact.
        if (m_pBaseGeom != nullptr && !m_pBaseGeom->BaseGeom()->Active())
        {
            lRoutCases &= 0xEFE7u;
        }

        // Bits 0x10 and 0x08 share the same active event list. Moving an event between
        // active and inactive lists is only needed when the tracked list bits change.
        if ((lRoutCases & 0x18u) != 0)
        {
            if ((m_lEventLists & 0x18u) == 0)
            {
                g_pSysInterface->m_pEngineData->m_EventList.ActivateFrameUpdate(this);
            }
        }
        else if ((m_lEventLists & 0x18u) != 0)
        {
            g_pSysInterface->m_pEngineData->m_EventList.DeactivateFrameUpdate(this);
        }

        // Scheduled updates are owned by ZScheduledUpdate, not by ZEventList. The event
        // keeps m_pScheduleEvent while bit 0x1000 is reflected in m_lEventLists.
        if ((lRoutCases & 0x1000u) != 0)
        {
            if ((m_lEventLists & 0x1000u) == 0)
            {
                g_pSysInterface->m_pEngineData->GetEventScheduler().AddEvent(this);
            }
        }
        else if ((m_lEventLists & 0x1000u) != 0)
        {
            g_pSysInterface->m_pEngineData->m_pScheduledUpdate->RemoveEvent(this);
        }

        // Mirror only the list-owned activity bits. Other rout flags, for example the
        // run-while-paused bit 0x100, stay in m_lRoutCases and are checked at dispatch.
        m_lEventLists = (m_lEventLists & ~0x1018u) | (lRoutCases & 0x1018u);
    }

    void ZEventBase::ActivateTimeUpdate(float timer_interval) 
    {
        if (timer_interval > 0.0f)
        {
            m_TimerInterval = timer_interval;
        }

        ZEngineDataBase* pEngineData = g_pSysInterface->m_pEngineData;

        m_fTimePassed.secs = g_pSysInterface->FrameTime.secs - static_cast<int>(pEngineData->m_fEvenOutTimers * m_TimerInterval * -TIMETYPE::kTicksPerSecond);

        pEngineData->m_fEvenOutTimers += 0.1f;
        if (pEngineData->m_fEvenOutTimers >= 1.0f)
        {
            pEngineData->m_fEvenOutTimers = 0.0f;
        }

        if ((m_lRoutCases & 8) == 0)
        {
            m_lRoutCases |= 8u;
            ChangeEventActivity();
        }
    }

    void ZEventBase::ChangeStatus(EStatus status, bool allowed_to_destroy)
    {
        std::ignore = allowed_to_destroy; // always allowed to destroy

        if (m_Status >= EStatus::STATUS_PostInit2 && m_Status < EStatus::STATUS_End)
        {
            do
            {
                const auto eNextStatus = static_cast<EStatus>(m_Status + 1);

                switch (m_Status)
                {
                    case EStatus::STATUS_New:
                    {
                        Init();
                    }
                    break;
                    case EStatus::STATUS_Init:
                    {
                        Init2();
                    }
                    break;
                    case EStatus::STATUS_Loaded:
                    {
                        PostInit2();
                    }
                    break;
                    case EStatus::STATUS_PostInit:
                    {
                        PostInit2();
                    }
                    break;
                    case EStatus::STATUS_Remove:
                    {
                        End();
                    }
                    break;
                    default: break;
                }

                if (m_Status < eNextStatus)
                {
                    m_Status = eNextStatus;
                }
            }
            while (m_Status < EStatus::STATUS_End);
        }
        else
        {
            m_Status = EStatus::STATUS_End;
        }

        if (m_Status == EStatus::STATUS_End)
        {
            // Call self dtor
            this->~ZEventBase();

            // Release memory from me
            ZEventBuffer::Instance().FreeEventRam(this);
        }
    }

    void ZEventBase::Delete()
    {
        ChangeStatus(EStatus::STATUS_End, true);
    }

    void ZEventBase::DetachFromGeom()
    {
        m_pBaseGeom = nullptr;
    }

    bool ZEventBase::SkipSave() const
    {
        for (ZBaseGeom* pBaseGeom = m_pBaseGeom->m_baseGeom; pBaseGeom; pBaseGeom = pBaseGeom->Parent())
        {
            if (pBaseGeom->GetGeom()->IsDerivedFrom<ZSkipSaveGroup>())
            {
                return true;
            }
        }

        return false;
    }

    bool ZEventBase::IsAttached() const
    {
        return m_pBaseGeom != nullptr;
    }

    bool ZEventBase::IsWorking() const
    {
        return m_Status > EStatus::STATUS_New && m_Status < EStatus::STATUS_Remove;
    }

    bool ZEventBase::IsPendingForRemoval() const
    {
        return m_Status >= EStatus::STATUS_Remove;
    }

    uint32_t ZEventBase::GetRef() const
    {
        return m_Ref;
    }

    void ZEventBase::DoInit()
    {
        ChangeStatus(ZEventBase::m_DefaultStatus, true);
    }

    void ZEventBase::DeactivateTimeUpdate()
    {
        m_lRoutCases &= ~8u;
        ChangeEventActivity();
    }

    void ZEventBase::DeactivateScheduleUpdate()
    {
        m_lRoutCases &= ~0x1000u;
        ChangeEventActivity();
    }

    int32_t ZEventBase::Call(uint32_t Case, void* Param1, uint16_t Param2)
    {
        ZASSERT(RefToPtr(m_Ref));

        if (IsWorking())
        {
            ZASSERT(m_pBaseGeom);

            DoEvent(Case, Param2, Param1);

            if (RefToPtr(m_Ref))
            {
                if (IsPendingForRemoval())
                {
                    ChangeStatus(EStatus::STATUS_End, true);
                }

                return 1;
            }

            return 0;
        }
        else
        {
            MYSTR sGeomFullName = m_pBaseGeom->CalcTotalName(true);
            printf("IMPORTANT: Trying to call event %s on geom %s when event has not been initialized yet or has been deleted.  Needs to be fixed!!!", 
                EventName(), 
                sGeomFullName.String);
        }

        return 1;
    }

    void ZEventBase::AttachToGeom(ZGEOM* pGeom)
    {
        m_pBaseGeom = pGeom;
    }

    void ZEventBase::OnlyUpdateMe(bool bOnlyUpdateMe)
    {
        if (bOnlyUpdateMe)
        {
            g_pEngineData->SetOnlyEventUpdate(this);
        }
        else if (g_pEngineData->GetOnlyEventUpdate() == this)
        {
            g_pEngineData->SetOnlyEventUpdate(nullptr);
        }
    }

    // TODO: Finish VirtualTable reconstruction!
    // TODO: Move to separated RTTI!
    namespace cProperties
    {
        static RTP::ZDataProperty<uint8_t> NamespaceItem_3654 {
            .m_Node {
                .m_Next = nullptr,
                .m_Name = "m_ClassCall",
                .m_Filter = 2
            },
            .m_VirtualTable = nullptr,
            .m_Offset = (uint8_t*)offsetof(ZEventBase, m_ClassCall)
        };

        static RTP::ZDataProperty<uint32_t> NamespaceItem_3653 {
            .m_Node {
                .m_Next = &NamespaceItem_3654.m_Node,
                .m_Name = "m_lRoutCases",
                .m_Filter = 2
            },
            .m_VirtualTable = nullptr,
            .m_Offset = (uint32_t*)offsetof(ZEventBase, m_lRoutCases)
        };

        static RTP::ZDataProperty<TIMETYPE> NamespaceItem_3652 {
            .m_Node {
                .m_Next = &NamespaceItem_3653.m_Node,
                .m_Name = "m_fTimePassed",
                .m_Filter = 2
            },
            .m_VirtualTable = nullptr,
            .m_Offset = (TIMETYPE*)offsetof(ZEventBase, m_fTimePassed)
        };

        static RTP::ZDataProperty<float> NamespaceItem_3651 {
            .m_Node {
                .m_Next = &NamespaceItem_3652.m_Node,
                .m_Name = "m_TimeInterval",
                .m_Filter = 2
            },
            .m_VirtualTable = nullptr,
            .m_Offset = (float*)offsetof(ZEventBase, m_TimerInterval)
        };
    }

    STATIC_CLASS_VAR_IMPL(ZEventBase, RTP::ZPropertyInfo, Info, 0x008068E4, (RTP::ZPropertyInfo {
        .First = &cProperties::NamespaceItem_3651.m_Node,
        .Super = &RTP::cBase::Info,
        .Name = "ZEventBase"
    }));

    STATIC_CLASS_VAR_IMPL(ZEventBase, uint32_t, m_DirectRef, 0x009725AC, 0);
    STATIC_CLASS_VAR_IMPL(ZEventBase, bool, m_LockCreation, 0x009725B0, false);
    STATIC_CLASS_VAR_IMPL(ZEventBase, ZEventBase::EStatus, m_DefaultStatus, 0x009725B4, ZEventBase::EStatus::STATUS_New);
}
