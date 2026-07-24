#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZTreeGroup.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/ZBaseConRout.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/CHUNKFILE.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/Serializer/IOutputSerializerStream.h>
#include <Glacier/ZSysInterface.h>
#include <Glacier/ZEngineDataBase.h>
#include <cstring>

namespace Glacier
{
    namespace
    {
        uint32_t striwcmp(const char* str, const char* wildcard)
        {
            const char* s = str;
            const char* w = wildcard;

            if (!*w) return false;

            while (1)
            {
                if (*w == '*') 
                {
                    const char* next_chunk = w + 1;
                    const char* next_star = strchr(next_chunk, '*');
                    w = next_star;

                    if (!next_star) 
                    {
                        size_t str_len = strlen(s);
                        size_t chunk_len = strlen(next_chunk);
                        
                        if (str_len >= chunk_len) 
                        {
                            return stricmp(&s[str_len - chunk_len], next_chunk) == 0;
                        }
                        return false;
                    }

                    size_t chunk_len = next_star - next_chunk;
                    if (next_star == next_chunk) return false;

                    size_t str_len = strlen(s);
                    int max_search_pos = str_len - chunk_len;
                    if (max_search_pos < 0) return false;

                    int checked = 0;
                    while (memicmp(next_chunk, s, chunk_len) != 0) 
                    {
                        checked++;
                        s++;
                        if (checked == max_search_pos + 1) return false;
                    }
                }
                else 
                {
                    const char* next_star = strchr(w, '*');
                    
                    if (!next_star) 
                    {
                        return stricmp(w, s) == 0;
                    }

                    size_t chunk_len = next_star - w;
                    if (memicmp(w, s, chunk_len) != 0) return false;

                    s += chunk_len;
                    w = next_star;
                }

                if (!*w) return true;
            }
        }
    }

    const char* g_pGeomName = nullptr;
    ZBaseGeom* g_pBaseGeom = nullptr;

    ZGEOM::~ZGEOM()
    {
        if (BaseGeom()->Control() & ZCDYNAMIC)
        {
            BaseGeom()->DetachFromDynamicContainer(nullptr);
            BaseGeom()->FreeRoomList();
        }

        FreeEvents();
        FreeExData();

        if ((BaseGeom()->Control() & 0x80000000) == 0)
        {
            BaseGeom()->SetControl(0x80000000, 0);
            BaseGeom()->~ZBaseGeom(); // I'm not sure do we need to cleanup mem?
        }
    }

    void ZGEOM::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        ZSerializable::LoadSave(stream, bSaving);

        CalcCenSize();

        const uint32_t lControl = BaseGeom()->Control();
        ZASSERT((lControl & ZCNONRUNTIME) || ((lControl & ZCCHKLIGHT) == 0) || BaseGeom()->m_uListID);

        stream.Exchange("GeomControl", m_lGeomControl);
    }

    void ZGEOM::LoadObject(IInputSerializerStream& stream)
    {
        RTP::cBase::LoadObject(stream);

        if (stream.TestStreamFilter(ISerializerStream::CONTENT_SavedGame))
        {
            BaseGeom()->LoadSave(stream, false);

            uint32_t lControl = 0;
            stream.Exchange("Control", lControl);
            BaseGeom()->m_lControl = lControl & ~(ZCOWNERDRAW | ZCRENDERATTACHED);

            if (BaseGeom()->m_lControl & ZCHASDYNAMICPARENT)
            {
                stream.Exchange("DynamicParentNr", BaseGeom()->m_iDynamicParentNr);
            }

            uint32_t lEventCount = 0;
            stream.ExchangeContainer("Events", lEventCount);

            if (lEventCount)
            {
                if (m_pExData)
                {
                    if (reinterpret_cast<std::intptr_t>(m_pExData) != static_cast<std::intptr_t>(-2))
                    {
                        const uint32_t lExistingEventCount = m_pExData->_Events.Count();
                        for (uint32_t i = 0; i < lExistingEventCount; ++i)
                        {
                            if (auto* pEvent = ZEventBase::RefToPtr(m_pExData->_Events.GetValueNr(i)))
                            {
                                pEvent->DetachFromGeom();
                            }
                        }
                    }

                    m_pExData->_Events.Clear();
                }
                else
                {
                    CreateExData();
                }

                auto* aEventRefs = static_cast<uint32_t*>(alloca(sizeof(uint32_t) * lEventCount));
                stream.ExchangeArray("EventREFs", aEventRefs, lEventCount);

                for (uint32_t i = 0; i < lEventCount; ++i)
                {
                    auto* pEvent = ZEventBase::RefToPtr(aEventRefs[i]);
                    ZASSERT(pEvent);

                    AttachEvent(*pEvent);
                    pEvent->DoInit();
                    stream.Exchange(ZToken::Void, *pEvent);
                }

                ChangeEventsActivity();
            }
            else if (m_pExData)
            {
                m_pExData->_Events.Clear();
            }

            if ((BaseGeom()->Control() & (ZCHASDYNAMICPARENT | ZCDYNAMIC)) != 0 && BaseGeom()->m_uListID)
            {
                BaseGeom()->SetControlDirect(0x2000000u, 0u);
            }
        }
    }

    void ZGEOM::SaveObject(IOutputSerializerStream& stream)
    {
        RTP::cBase::SaveObject(stream);

        if (stream.TestStreamFilter(ISerializerStream::CONTENT_SavedGame))
        {
            BaseGeom()->LoadSave(stream, true);
            stream.Exchange("Control", BaseGeom()->m_lControl);

            if (BaseGeom()->m_lControl & ZCHASDYNAMICPARENT)
            {
                stream.Exchange("DynamicParentNr", BaseGeom()->m_iDynamicParentNr);
            }

            uint32_t lEventCount = 0;
            if (m_pExData && reinterpret_cast<std::intptr_t>(m_pExData) != static_cast<std::intptr_t>(-2))
            {
                const uint32_t lTotalEventCount = m_pExData->_Events.Count();
                for (uint32_t i = 0; i < lTotalEventCount; ++i)
                {
                    if (ZEventBase::RefToPtr(m_pExData->_Events.GetValueNr(i)))
                    {
                        ++lEventCount;
                    }
                }
            }

            stream.ExchangeContainer("Events", lEventCount);

            if (lEventCount)
            {
                auto* aEventRefs = static_cast<uint32_t*>(alloca(sizeof(uint32_t) * lEventCount));
                uint32_t lWrittenEventCount = 0;

                const uint32_t lTotalEventCount = m_pExData->_Events.Count();
                for (uint32_t i = 0; i < lTotalEventCount; ++i)
                {
                    const uint32_t lEventRef = m_pExData->_Events.GetValueNr(i);
                    if (ZEventBase::RefToPtr(lEventRef))
                    {
                        aEventRefs[lWrittenEventCount++] = lEventRef;
                    }
                }

                ZASSERT(lWrittenEventCount == lEventCount);

                stream.ExchangeArray("EventREFs", aEventRefs, lEventCount);

                for (uint32_t i = 0; i < lEventCount; ++i)
                {
                    auto* pEvent = ZEventBase::RefToPtr(aEventRefs[i]);
                    ZASSERT(pEvent);
                    ZASSERT(pEvent->m_pBaseGeom == this);

                    stream.Exchange(ZToken::Void, *pEvent);
                }
            }
        }
    }

    const RTP::ZPropertyInfo& ZGEOM::GetProperties() const
    {
        return ZGEOM::Info;
    }

    uint32_t ZGEOM::GetObjectId() const
    {
        return ZGEOM::m_Id;
    }

    void ZGEOM::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZGEOM::m_Id;
        mask = ZGEOM::m_Mask;
    }

    ZGEOMCLASSINFO* ZGEOM::GetOldClassInfo() const
    {
        // Filled by static initialized ZGEOM_Producer at CRT
        return ZGEOM::m_OldClassInfo;
    }

    bool ZGEOM::DoInit()
    {
        ZREF selfRef = ZGeomBuffer::Instance().GeomPtrToRef(this);

        while (m_eStatus < ZGEOM::m_PreferedStatus)
        {
            EStatus nextStatus = static_cast<EStatus>(m_eStatus + 1);

            switch (m_eStatus)
            {
                case STATUS_New:
                    ClassInit();
                    break;

                case STATUS_Init:
                    ClassInit2();
                    break;

                case STATUS_Init2:
                    break;

                case STATUS_Loaded:
                    PostClassInit();
                    break;

                case STATUS_PostInit:
                    PostClassInit2();
                    break;

                default:
                    break;
            }

            // If we preferred to die in process - ok
            if (!ZGeomBuffer::Instance().GeomRefToPtr(selfRef))
            {
                return false;
            }

            m_eStatus = nextStatus;
        }

        return true;
    }

    void ZGEOM::PreSaveGame()
    {
        if (!m_pExData) return;

        if (m_pExData->_Events.ChkEvents())
        {
            ZGeomEventListBuffers::ValueRun sRun {};
            m_pExData->_Events.InitValueRun(sRun);

            auto valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);
            while (valueFromRun != 1)
            {
                auto* event = ZEventBase::RefToPtr(valueFromRun);
                ZASSERT(event);

                event->PreSaveGame();

                m_pExData->_Events.NextValueRun(sRun);
                valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);
            }
        }
    }

    void ZGEOM::CheckPointSave(ZCheckPointBuffer&) {}
    void ZGEOM::CheckPointLoad(ZCheckPointBuffer&) {}

    void ZGEOM::Activate(bool bActive)
    {
        if (!m_pExData) return;

        if (m_pExData->_Events.ChkEvents())
        {
            const auto iInitialCount = m_pExData->_Events.Count();
            uint32_t* pEventRefs = (uint32_t*)alloca(sizeof(uint32_t) * iInitialCount); // I hate alloca
            int i = 0;

            if (reinterpret_cast<std::intptr_t>(m_pExData) != static_cast<std::intptr_t>(-2)) // weirtd code from PS2
            {
                ZGeomEventListBuffers::ValueRun sRun {};
                m_pExData->_Events.InitValueRun(sRun);

                auto valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);

                while (valueFromRun != 1)
                {
                    pEventRefs[i] = valueFromRun;

                    m_pExData->_Events.NextValueRun(sRun);
                    valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);
                }
            }

            for (i = 0; i < iInitialCount; ++i)
            {
                auto* event = ZEventBase::RefToPtr(pEventRefs[i]);
                if (event)
                {
                    event->DoInit();
                }
            }
        }

        ChangeEventsActivity();
    }

    void ZGEOM::MakeInactive()
    {
        BaseGeom()->MakeInactive();
    }

    void ZGEOM::MakeActive()
    {
        BaseGeom()->MakeActive();
    }
    
    eGlobalTreeType ZGEOM::GetBoundTreeType() const
    {
        return eGlobalTreeType::GT_None;
    }

    int ZGEOM::AnimCallBackToId(ActiveAnimCB pCallback)
    {
        ZASSERT(false && "Failed to map Animation callback to an id");
        return -1;
    }

    ZGEOM::ActiveAnimCB ZGEOM::AnimCallBackFromId(int id)
    {
        return nullptr;
    }

    void ZGEOM::CameraMessages(bool bEnabled)
    {
        if (bEnabled)
        {
            if (!m_pExData)
            {
                CreateExData();
            }

            m_pExData->_lControl |= ZCEXWANTCAMERAMSG;
        }
        else
        {
            if (!m_pExData)
            {
                return;
            }

            m_pExData->_lControl &= ~ZCEXWANTCAMERAMSG;
        }
    }
    
    void ZGEOM::CreateExData()
    {
        if (!m_pExData)
        {
            m_pExData = ZUniMemory::New<ExGeomData>();
        }
    }

    void ZGEOM::FreeExData()
    {
        if (m_pExData)
        {
            FreeEvents();

            if (m_pExData->_ExtraInitData && !g_pEngineData->CheckInPackBuffer(m_pExData->_ExtraInitData))
            {
                ZUniMemory::Free(m_pExData->_ExtraInitData);
            }

            if (m_pExData)
            {
                ZUniMemory::Delete(m_pExData);
            }

            m_pExData = nullptr;
        }
    }
    
    void ZGEOM::CopyExData(const ZGEOM* Source)
    {
        FreeExData();

        if (Source->m_pExData)
        {
            CreateExData();

            m_pExData->_ExtraInitData = Source->m_pExData->_ExtraInitData;
            if (m_pExData->_ExtraInitData)
            {
                ZASSERT(g_pEngineData->CheckInPackBuffer(m_pExData->_ExtraInitData));
                const auto iTotalExtraInitSize = Source->m_pExData->_ExtraInitData->GetTotalSizeAligned();

                m_pExData->_ExtraInitData = (CHUNKFILE*)ZUniMemory::Allocate(sizeof(uint8_t) * iTotalExtraInitSize);
                std::memcpy(m_pExData->_ExtraInitData, Source->m_pExData->_ExtraInitData, iTotalExtraInitSize);
            }

            CopyEvents(Source);
        }
    }

    CHUNKFILE* ZGEOM::GetChunkData(int Name) const
    {
        if (m_pExData && m_pExData->_ExtraInitData)
        {
            return m_pExData->_ExtraInitData->FindChild(Name);
        }

        return nullptr;
    }

    void ZGEOM::RecurGetNext(ZBaseGeom** ppNext)
    {
        *ppNext = nullptr;
    }

    bool ZGEOM::DisableParentBoundAdjust()
    {
        return false;
    }

    void ZGEOM::ExpandBounds(ZMat3x3& mMat, ZVector3& vCen, ZVector3& vSize, ZBaseGeom* pBaseGeom)
    {
        if (m_pExData->_Events.ChkEvents() && reinterpret_cast<std::intptr_t>(m_pExData) != static_cast<std::intptr_t>(-2))
        {
            ZGeomEventListBuffers::ValueRun sRun {};
            m_pExData->_Events.InitValueRun(sRun);

            auto valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);
            while (valueFromRun != 1)
            {
                auto* event = ZEventBase::RefToPtr(valueFromRun);
                ZASSERT(event);

                event->ExpandBounds(mMat, vCen, vSize, pBaseGeom);

                m_pExData->_Events.NextValueRun(sRun);
                valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);
            }
        }
    }

    void ZGEOM::SetMat(const Glacier::ZMat3x3& mMat)
    {
        BaseGeom()->SetMat(mMat.Get());
    }

    void ZGEOM::SetPos(const Glacier::ZVector3& vPos)
    {
        BaseGeom()->SetPos(vPos.Get());
    }
    
    void ZGEOM::SetMatPos(const Glacier::ZMat3x3& mat, const Glacier::Vector3& pos)
    {
        BaseGeom()->SetMatPos(mat.Get(), pos.Get());
    }

    void ZGEOM::SetWorldPosition(const Glacier::Vector3& pos)
    {
        BaseGeom()->SetRootPos(pos.Get());
    }
    
    void ZGEOM::SetRootTM(const ZMat3x3& RTMat, const ZVector3& RTPos)
    {
        BaseGeom()->SetRootTM(RTMat, RTPos);
    }

    void ZGEOM::Display(bool OnOff)
    {
        constexpr uint16_t DisplayFlag = 0x1;
        constexpr uint16_t DisplayBlockMask = 0xE;

        if (OnOff)
        {
            m_lGeomControl |= DisplayFlag;
        }
        else if ((m_lGeomControl & DisplayBlockMask) == 0)
        {
            m_lGeomControl &= ~DisplayFlag;
        }
    }
    
    void ZGEOM::Select(bool OnOff)
    {
        constexpr uint16_t DisplayFlag = 0x1;
        constexpr uint16_t SelectFlag = 0x8;

        if (OnOff)
        {
            m_lGeomControl |= DisplayFlag | SelectFlag;
        }
        else
        {
            m_lGeomControl = static_cast<uint16_t>(m_lGeomControl & ~SelectFlag);
        }
    }

    void ZGEOM::Hide(bool OnOff)
    {
        BaseGeom()->Hide(OnOff);
    }

    void ZGEOM::UNKNOWN_FUCKING_METHOD() {}  // Really nothing here

    void ZGEOM::HideRecursive(bool bHide)
    {
        BaseGeom()->HideRecursive(bHide);
    }

    void ZGEOM::Freeze(bool OnOff)
    {
        if (OnOff)
        {
            m_lGeomControl |= 0x10u;
        }
        else
        {
            m_lGeomControl &= ~0x10u;
        }
    }

    void ZGEOM::DispBound(bool OnOff)
    {
        if (OnOff)
        {
            if ((m_lGeomControl & 4) == 0)
            {
                m_lGeomControl |= 0x5u;
            }
        }
        else if (m_lGeomControl & 4)
        {
            m_lGeomControl &= ~4;
        }
    }

    void ZGEOM::SetMoving(bool bMoving)
    {
        if (bMoving)
        {
            CreateExData();

            m_pExData->_lControl |= ZCEXMOVINGOBJ;
        }
        else
        {
            if (!m_pExData)
            {
                return;
            }

            m_pExData->_lControl &= ~ZCEXMOVINGOBJ;
        }
    }
    
    bool ZGEOM::RequestCustomDraw() const
    {
        return (m_lGeomControl & 0x20u) != 0;
    }

    bool ZGEOM::HasOwnerDraw() const
    {
        return false;
    }

    void ZGEOM::OwnerDraw(ZBaseGeom *,uint,ZDrawBuffer *,ZCAMERA *,float const*,float const*,void const*)
    {}

    void ZGEOM::CorrectOwnerDrawMatrix(ZMat3x3& mMat, ZVector3& vPos, ZBaseGeom *pOwnerBaseGeom, uint32_t lBoneId)
    {}

    void ZGEOM::CorrectOwnerDrawPartMatrix(ZMat4x4*,ZBaseGeom*)
    {}

    bool ZGEOM::WantDrawBufferControl() const
    {
        return false;
    }

    void ZGEOM::DrawUpdate()
    {}

    bool ZGEOM::DrawBufferViewUpdate(ZDrawBuffer* pDrawBuffer,ZCameraSpace* pCameraSpace)
    {
        return false;
    }

    bool ZGEOM::DrawBufferViewUpdate(ZDrawBuffer* pDrawBuffer, ZBaseGeom* pOwnerBaseGeom, uint32_t lBoneId)
    {
        return false;
    }

    bool ZGEOM::WantViewPrimHideUnhideRequest() const
    {
        return false;
    }

    bool ZGEOM::WantViewPrimHide(uint8_t lLODControl) const
    {
        return false;
    }
    
    bool ZGEOM::WantViewPrimHideMirrors(uint8_t lLODControl) const
    {
        return false;
    }
    
    bool ZGEOM::WantViewUseAllLOD() const
    {
        return false;
    }
    
    uint8_t ZGEOM::GetLODMaskOverride() const
    {
        return 0u;
    }

    uint8_t ZGEOM::GetSortPriority() const
    {
        return 7;
    }

    void ZGEOM::CopyEvents(const ZGEOM* Source)
    {
        FreeEvents();

        if (m_pExData->_Events.ChkEvents())
        {
            CreateExData();

            if (reinterpret_cast<std::intptr_t>(m_pExData) != static_cast<std::intptr_t>(-2))
            {
                ZGeomEventListBuffers::ValueRun sRun {};
                m_pExData->_Events.InitValueRun(sRun);

                auto valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);

                while (valueFromRun != 1)
                {
                    auto* event = ZEventBase::RefToPtr(valueFromRun);
                    if (event)
                    {
                        CopyEvent(event);
                    }
                    else
                    {
                        m_pExData->_Events.RunRemoveEvent(sRun);
                    }

                    m_pExData->_Events.NextValueRun(sRun);
                    valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);
                }
            }
        }
    }

    ZEventBase* ZGEOM::FindEvent(char const* pEventName) const
    {
        if (m_pExData->_Events.ChkEvents() && reinterpret_cast<std::intptr_t>(m_pExData) != static_cast<std::intptr_t>(-2))
        {
            ZGeomEventListBuffers::ValueRun sRun {};
            m_pExData->_Events.InitValueRun(sRun);

            auto valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);

            while (valueFromRun != 1)
            {
                auto* event = ZEventBase::RefToPtr(valueFromRun);
                if (event)
                {
                    if (auto* pCurrentEventName = event->EventName(); pCurrentEventName && striwcmp(pCurrentEventName, pEventName) == 0)
                    {
                        return event;
                    }
                }
                else
                {
                    m_pExData->_Events.RunRemoveEvent(sRun);
                }

                m_pExData->_Events.NextValueRun(sRun);
                valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);
            }
        }

        return nullptr;
    }
    
    void* ZGEOM::GetEventData(const char* pEventName) const
    {
        return FindEvent(pEventName);
    }

    ZBaseConRout* ZGEOM::AddEvent(char const* pEventName)
    {
        auto* pEvent = ZBaseConRout::GetFactory().Create(pEventName);
        if (!pEvent)
        {
            printf("ZGEOM::AddEvent: Unable to create event %s\n", pEventName);
            return nullptr;
        }

        AttachEvent(*pEvent);
        pEvent->DoInit();

        return pEvent;
    }
    
    void ZGEOM::AttachEvent(ZEventBase& event)
    {
        event.AttachToGeom(this);

        CreateExData();

        m_pExData->_Events.Add(event.GetRef());
        event.ChangeEventActivity();
    }

    void ZGEOM::EnableClassCall(uint32_t Cases)
    {
        ZEventBase* pFoundEvent = 0u;

        if (m_pExData->_Events.ChkEvents() && reinterpret_cast<std::intptr_t>(m_pExData) != static_cast<std::intptr_t>(-2))
        {
            ZGeomEventListBuffers::ValueRun sRun {};
            m_pExData->_Events.InitValueRun(sRun);

            auto valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);

            while (valueFromRun != 1)
            {
                auto* event = ZEventBase::RefToPtr(valueFromRun);
                if (event)
                {
                    if (event->m_ClassCall)
                    {
                        pFoundEvent = event;
                        break;
                    }
                }
                else
                {
                    m_pExData->_Events.RunRemoveEvent(sRun);
                }

                m_pExData->_Events.NextValueRun(sRun);
                valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);
            }
        }

        if (pFoundEvent)
        {
            pFoundEvent->m_fTimePassed = g_pSysInterface->FrameTime;
            pFoundEvent->m_lRoutCases |= (Cases & 0x1118);
        }
        else
        {
            pFoundEvent = g_pEngineData->AllocGeomCallEvent(this);
            pFoundEvent->m_lRoutCases = (Cases & 0x1118);
            m_pExData->_Events.Add(pFoundEvent->GetRef());
            pFoundEvent->m_pBaseGeom = this; // Assign
            pFoundEvent->ChangeEventActivity();
        }
    }

    void ZGEOM::DisableClassCall(uint32_t Cases)
    {
        REFTAB32 aFoundEvents;
        const uint32_t lMask = Cases & 0x1018;

        if (m_pExData->_Events.ChkEvents() && reinterpret_cast<std::intptr_t>(m_pExData) != static_cast<std::intptr_t>(-2))
        {
            ZGeomEventListBuffers::ValueRun sRun {};
            m_pExData->_Events.InitValueRun(sRun);

            auto valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);

            while (valueFromRun != 1)
            {
                aFoundEvents.Add(valueFromRun);
                m_pExData->_Events.NextValueRun(sRun);
                valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);
            }

            for (ZREF rEvent : aFoundEvents)
            {
                auto* pEvent = ZEventBase::RefToPtr(rEvent);
                if (pEvent && pEvent->m_ClassCall && (pEvent->m_lRoutCases & lMask))
                {
                    ZASSERT(pEvent->m_pBaseGeom == this);
                    pEvent->Delete();
                }
            }
        }
        else
        {
            const MYSTR sName = CalcTotalName(true);
            printf("WARNING: Unable to disable class-call when class-call was not set up on %s\n", sName.String);
        }
    }
    
    void ZGEOM::SetClassTimerInterval(float fInterval)
    {
        ZEventBase* pTimerEvent = nullptr;

        if (m_pExData->_Events.ChkEvents() && reinterpret_cast<std::intptr_t>(m_pExData) != static_cast<std::intptr_t>(-2))
        {
            ZGeomEventListBuffers::ValueRun sRun {};
            m_pExData->_Events.InitValueRun(sRun);

            auto valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);

            while (valueFromRun != 1)
            {
                auto* pEvent = ZEventBase::RefToPtr(valueFromRun);
                if (pEvent && pEvent->m_ClassCall && (pEvent->m_lRoutCases & 0x8))
                {
                    pTimerEvent = pEvent;
                    break;
                }

                m_pExData->_Events.NextValueRun(sRun);
                valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);
            }
        }

        if (pTimerEvent)
        {
            pTimerEvent->m_TimerInterval = fInterval;
        }
        else
        {
            const MYSTR sName = CalcTotalName(true);
            printf("ERROR: Trying to set classtimerinterval for %s when timer update isn't enabled", sName.String);
        }
    }

    int32_t ZGEOM::CallEvents(int Type, void* Param1, int16_t Param2, ZGEOM* pCaller)
    {
        if (m_pExData->_Events.ChkEvents() && reinterpret_cast<std::intptr_t>(m_pExData) != static_cast<std::intptr_t>(-2))
        {
            REFTAB32 aFoundEvents;

            ZGeomEventListBuffers::ValueRun sRun {};
            m_pExData->_Events.InitValueRun(sRun);

            auto valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);

            while (valueFromRun != 1)
            {
                auto* pEvent = ZEventBase::RefToPtr(valueFromRun);
                if (pEvent)
                {
                    aFoundEvents.Add(valueFromRun);
                }
                else
                {
                    m_pExData->_Events.RunRemoveEvent(sRun);
                }

                m_pExData->_Events.NextValueRun(sRun);
                valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);
            }

            for (ZREF rEvent : aFoundEvents)
            {
                auto* pEvent = ZEventBase::RefToPtr(rEvent);
                if (
                    pEvent && 
                    !pEvent->m_ClassCall && 
                    ((pEvent->m_lRoutCases & Type) != 0 || Type == 0x20u) &&
                    !pEvent->IsPendingForRemoval() && pEvent->IsWorking()
                )
                {
                    ZREF rSelf = GetRef();
                    pEvent->DoEvent(Type, Param2, Param1);

                    // ???
                    if (!ZBaseGeom::RefToPtr(rSelf))
                    {
                        return 0;
                    }

                    if (pEvent->IsPendingForRemoval())
                    {
                        ZASSERT(pEvent->m_pBaseGeom == this);
                        pEvent->Delete();
                    }
                }
            }
        }

        return 1;
    }

    void ZGEOM::SendCommand(ZMSGID Msg, void* pData, ZGEOM* pTarget)
    {
        ZASSERT(pTarget);

        ZGEOM::s_pLastSendCommandCaller = this;
        ClassCommand(Msg, pData);
        CallEvents(0x20, pData, Msg, pTarget);
    }

    void ZGEOM::SendCommand(ZREF TargetRef, ZMSGID Msg, void* pData)
    {
        ZGEOM* pTargetGeom = RefToPtr(TargetRef);
        if (pTargetGeom)
        {
            SendCommand(pTargetGeom, Msg, pData);
        }
    }

    void ZGEOM::SendCommand(ZGEOM* pTarget, ZMSGID Msg, void* pData)
    {
        ZASSERT(pTarget);

        ZGEOM::s_pLastSendCommandCaller = this;
        pTarget->ClassCommand(Msg, pData);
        pTarget->CallEvents(0x20, pData, Msg, this);
        ZGEOM::s_pLastSendCommandCaller = nullptr;
    }

    void ZGEOM::SendCommandRecursive(ZREF rTarget, ZMSGID Msg, void* pData)
    {
        auto* pTarget = RefToPtr(rTarget);
        ZASSERT(pTarget);

        SendCommandRecursive(Msg, pData, pTarget);
    }

    void ZGEOM::SendCommandRecursive(ZMSGID Msg, void* pData, ZGEOM* pTarget)
    {
        SendCommand(Msg, pData, pTarget);
    }

    void ZGEOM::FreeEvents()
    {
        if (m_pExData && BaseGeom()->ChkEvents() && m_pExData->_Events.ChkEvents())
        {
            while (m_pExData->_Events.Count())
            {
                ZREF rEvent = m_pExData->_Events.GetValueNr(0);
                auto* pEvent = ZEventBase::RefToPtr(rEvent);

                ZASSERT(pEvent);
                ZASSERT(pEvent->m_pBaseGeom == this);

                if (pEvent)
                {
                    pEvent->Delete();
                }                
            }
        }
    }
    
    void ZGEOM::FreeEvent(char const* pEventName)
    {
        auto* pEvent = FindEvent(pEventName);
        if (pEvent)
        {
            FreeEvent(pEvent);
        }
    }

    void ZGEOM::FreeEvent(ZEventBase* pEvent)
    {
        if (m_pExData->_Events.ChkEvents() && reinterpret_cast<std::intptr_t>(m_pExData) != static_cast<std::intptr_t>(-2))
        {
            ZGeomEventListBuffers::ValueRun sRun {};
            m_pExData->_Events.InitValueRun(sRun);

            auto valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);

            while (valueFromRun != 1)
            {
                auto* pCurrentEvent = ZEventBase::RefToPtr(valueFromRun);
                if (pCurrentEvent == pEvent)
                {
                    ZASSERT(pCurrentEvent->m_pBaseGeom == this);
                    pCurrentEvent->Delete();
                    break;
                }

                m_pExData->_Events.NextValueRun(sRun);
                valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);
            }
        }

        if (!m_pExData->_Events.Count())
        {
            FreeEvents();
        }
    }

    void ZGEOM::RemoveEventFromList(ZEventBase* pEvent)
    {
        if (m_pExData->_Events.ChkEvents() && reinterpret_cast<std::intptr_t>(m_pExData) != static_cast<std::intptr_t>(-2))
        {
            ZGeomEventListBuffers::ValueRun sRun {};
            m_pExData->_Events.InitValueRun(sRun);

            auto valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);

            while (valueFromRun != 1)
            {
                auto* pCurrentEvent = ZEventBase::RefToPtr(valueFromRun);
                if (pCurrentEvent == pEvent)
                {
                    m_pExData->_Events.RunRemoveEvent(sRun);
                    break;
                }

                m_pExData->_Events.NextValueRun(sRun);
                valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);
            }
        }

        if (!m_pExData->_Events.Count())
        {
            FreeEvents();
        }
    }
    
    void ZGEOM::RegisterInstance(uint32_t lInstanceCount)
    {}

    void ZGEOM::ClassInit()
    {}

    void ZGEOM::ClassInit2()
    {}

    void ZGEOM::PostClassInit()
    {}

    void ZGEOM::PostClassInit2()
    {}

    void ZGEOM::ClassDisplay()
    {}

    void ZGEOM::ClassFrameUpdate()
    {}

    void ZGEOM::ClassTimeUpdate()
    {}

    int32_t ZGEOM::ClassCommand(ZMSGID Msg, void* pData)
    {
        return 0;
    }

    void ZGEOM::ClassPushPopScene(bool)
    {
    }

    bool ZGEOM::ChkLineColi(COLI* pColi, bool bWantInvisible)
    {
        return false;
    }

    bool ZGEOM::CheckPointInside(ZVector3& pPoint, float fDotDist)
    {
        return BaseGeom()->CheckPointInside(pPoint, fDotDist);
    }

    bool ZGEOM::CheckPointInsideBound(ZVector3& pPoint)
    {
        ZVector3 vCen {};
        BaseGeom()->GetCen(vCen);

        if (fabsf(pPoint.x) > vCen.x || fabsf(pPoint.y) > vCen.y || fabsf(pPoint.z) > vCen.z)
        {
            return false;
        }

        return true;
    }
    
    bool ZGEOM::CheckBoxInside(const ZMat3x3& mMat, const ZVector3& vPos, const float* s0)
    {
        return false;
    }
    
    float ZGEOM::GetPointInsideDistance(const ZVector3& vPos)
    {
        return 9.9999997e37f; // lol
    }

    void ZGEOM::Visible() 
    {}

    void ZGEOM::Invisible() 
    {}
    
    void ZGEOM::PushState() 
    {}
    
    bool ZGEOM::DuplicateData(ZGEOM* pFromGeom)
    {
        const ZGEOMCLASSINFO* pSelfInfo = GetOldClassInfo();
        const ZGEOMCLASSINFO* pFromInfo = pFromGeom->GetOldClassInfo();

        if (pFromInfo->IsDerivedFrom(pSelfInfo) || pSelfInfo->IsDerivedFrom(pFromInfo))
        {
            pFromGeom->BaseGeom()->CopyData(BaseGeom());
            return true;
        }
        else
        {
            // Some warning
            printf("Cant duplicate %s::%s -> %s::%s different base classes\n", 
                // me
                pSelfInfo->ClassInfoName(),
                Name(),
                // and Michael
                pFromInfo->ClassInfoName(),
                pFromGeom->Name()
                // Me and Michael, solid as they come
            );
        }

        return false;
    }

    ZGEOM* ZGEOM::Duplicate(ZGROUP* DestGroup, const char* DupName, bool Recursive)
    {
        if (BaseGeom()->ParentGroup())
        {
            DupName = DupName ? DupName : Name();

            const auto* pClassInfo = GetOldClassInfo();
            const auto lType = pClassInfo->Type();

            auto* pGeom = DestGroup->CreateGeom(DupName, lType, true);
            if (DuplicateData(pGeom))
            {
                return pGeom;
            }

            // Otherwise
            pGeom->Delete();
        }

        return nullptr;
    }

    ZGEOM* ZGEOM::DuplicateInit(ZGROUP* DestGroup, const Glacier::ZMat3x3* pMat, const Glacier::ZVector3* pPos, char const* DupName, bool Recursive)
    {
        auto* pNewGeom = Duplicate(DestGroup, DupName, Recursive);
        if (pNewGeom)
        {
            if (pMat && pPos)
            {
                pNewGeom->SetMatPos(*pMat, *pPos);
            }

            // Possible leak of pNewGeom if DoInit failed!!! NEED BUGFIX 
            return pNewGeom->DoInit() ? pNewGeom : nullptr;
        }

        return pNewGeom;
    }

    ZGEOM* ZGEOM::DuplicateToResource(ZGROUP* DestGroup, uint32_t lGeomResourceId, const char* DupName, bool Recursive)
    {
        if (g_pEngineData->ResourcesDisabled())
        {
            return Duplicate(DestGroup, DupName, Recursive);
        }
        else if (BaseGeom()->ParentGroup())
        {
            DupName = DupName ? DupName : Name();

            const auto* pSelfClassInfo = GetOldClassInfo();
            auto* pResourceGeom = DestGroup->CreateResourceGeom(DupName, lGeomResourceId, pSelfClassInfo->Type(), true);
            if (pResourceGeom)
            {
                if (!DuplicateData(pResourceGeom))
                {
                    pResourceGeom->Delete();
                    return nullptr;
                }
            }

            return pResourceGeom;
        }

        return nullptr;
    }

    ZGEOM* ZGEOM::DuplicateToResourceInit(ZGROUP* DestGroup, uint32_t lGeomResourceId, const ZMat3x3* mMat, const ZVector3* vPos, const char* DupName, bool Recursive)
    {
        if (g_pEngineData->ResourcesDisabled())
        {
            return DuplicateInit(DestGroup, mMat, vPos, DupName, Recursive);
        }
        
        auto* pGeom = DuplicateToResource(DestGroup, lGeomResourceId, DupName, Recursive);
        if (!pGeom) return nullptr;

        if (mMat && vPos)
        {
            pGeom->SetMatPos(*mMat, *vPos);
        }

        // Possible leak of pNewGeom if DoInit failed!!! NEED BUGFIX 
        return pGeom->DoInit() ? pGeom : nullptr;
    }

    void ZGEOM::CopyData(const ZGEOM* Source)
    {
        CopyExData(Source);
    }

    void ZGEOM::AddActionMap(SInputActionDefinition*, char*)
    {
        // Nothing here
    }

    void ZGEOM::RemoveActionMap(char*)
    {
        // Nothing here
    }
    
    void ZGEOM::CheckAction(int,void *)
    {
        // Nothing here   
    }
    
    void ZGEOM::CheckAction(char *,void *)
    {
        // Nothing here
    }

    void ZGEOM::CheckAction(SInputAction *,void *)
    {
        // Nothing here
    }
    
    int ZGEOM::GetActionID(const char* psActionName)
    {
        return -1;
    }

    SInputAction* ZGEOM::GetActionPtr(int iActionId)
    {
        return nullptr;
    }

    SInputAction* ZGEOM::GetActionPtr(const char* psActionName)
    {
        return nullptr;
    }

    bool ZGEOM::IsAttached() const
    {
        return false;
    }

    void ZGEOM::SetAttachedTo(ZLNKOBJ* pAttachedTo)
    {
        // Do nothing
    }

    ZLNKOBJ* ZGEOM::GetAttachedTo() const
    {
        return nullptr;
    }
    
    bool ZGEOM::IsLinkBaseObj() const
    {
        return false;
    }
    
    void ZGEOM::OnCameraEnter()
    {
        // Do nothing here
    }


    ZGEOM::ZGEOM(const char* pName, ZBaseGeom* pBaseGeom)
    {
        pBaseGeom->m_pExtraGeom = this;
        m_baseGeom = pBaseGeom;
        pBaseGeom->SetName(pName);
        m_pExData = nullptr;
        m_lGeomControl = 0;
    }

    bool ZGEOM::IsInitialized() const
    {
        // Original code looks like shit, this not better
        return m_eStatus == STATUS_OK;
    }
    
    void ZGEOM::GetRootTM(ZMat3x3& mat, ZVector3& pos) const
    {
        m_baseGeom->GetRootTM(mat, pos);
    }

    void ZGEOM::GetMatPos(ZMat3x3& mat, ZVector3& pos) const
    {
        m_baseGeom->GetMatPos(mat, pos);
    }

    void ZGEOM::GetRootPoint(Glacier::ZVector3& pos) const 
    {
        m_baseGeom->GetRootPoint(pos);
    }

    void ZGEOM::GetCen(Glacier::ZVector3& pos) const 
    {
        m_baseGeom->GetCen(pos);
    }

    void ZGEOM::GetRootMatPos(Glacier::ZMat3x3& mat, Glacier::ZVector3& pos) const
    {
        m_baseGeom->GetRootMatPos(mat, pos);
    }

    void ZGEOM::GetLocalPoint(Glacier::ZVector3& pos) const
    {
        m_baseGeom->GetLocalPoint(pos);
    }

    void ZGEOM::Zvmmul(Glacier::ZVector3& pos) const
    {
        TransformRootVector(pos, m_baseGeom->m_mMat);
    }

    ZGEOM* ZGEOM::RefToPtr(unsigned int REF)
    {
        return ZGeomBuffer::Instance().GeomRefToPtr(REF);
    }

    ZFactory<ZGEOM>& ZGEOM::GetFactory()
    {
        return ZGEOM::m_Factory;
    }

    ZGEOM* ZGEOM::AllocExtraGeomHelper(uint32_t iExtraGeomBlockSize)
    {
        return ZGeomBuffer::Instance().AllocExtraGeom(iExtraGeomBlockSize);
    }

    const RTP::ZPropertyInfo& ZGEOM::Properties()
    {
        return ZGEOM::Info;
    }

    uint32_t ZGEOM::GetClassId()
    {
        return ZGEOM::m_Id;
    }

    void ZGEOM::GetClassIdAndMask(uint32_t& lClassId, uint32_t& lMask)
    {
        lClassId = ZGEOM::m_Id;
        lMask = ZGEOM::m_Mask;
    }

    void ZGEOM::SetPreferedStatus(EStatus status)
    {
        ZGEOM::m_PreferedStatus = status;
    }

    uint32_t ZGEOM::GetGeomType() const
    {
        return GetOldClassInfo()->m_lType;
    }

    ZREF ZGEOM::GetRef() const
    {
        return ZGeomBuffer::Instance().GeomPtrToRef(this);
    }

    MYSTR ZGEOM::CalcTotalName(bool bRoot) const
    {
        MYSTR str {};
        CalcTotalNameRecur(str, bRoot);
        return str;
    }

    void ZGEOM::CalcTotalNameRecur(MYSTR& sName, bool bRoot) const
    {
        ZBaseGeom* parent = m_baseGeom->Parent();

        if (parent)
        {
            // really reinterpret_cast? I though we've may cast ZROOM -> ZTreeGroup -> ZGROUP -> ZGEOM, but ok
            if (parent && (bRoot || parent->GetGeom() != reinterpret_cast<ZGEOM*>(g_pEngineData->m_pRoot)))
            {
                parent->GetGeom()->CalcTotalNameRecur(sName, bRoot);
                sName += "\\";
            }
        }

        sName += Name();
    }

    void ZGEOM::GetMatrix(float (&mMat)[9])
    {
        std::memcpy(&mMat[0], &m_baseGeom->m_mMat.data[0], sizeof(float) * 9);
    }

    void ZGEOM::SetMatrix(const float (&mMat)[9])
    {
        m_baseGeom->SetMat(&mMat[0]);
    }

    void ZGEOM::GetPosition(float (& vPos)[3])
    {
        vPos[0] = BaseGeom()->m_vPos.x;
        vPos[1] = BaseGeom()->m_vPos.y;
        vPos[2] = BaseGeom()->m_vPos.z;
    }

    void ZGEOM::SetPosition(float const (&vPos)[3])
    {
        // It's unsafe to ZBaseGeom, but game do it
        BaseGeom()->m_vPos = ZVector3 { vPos[0], vPos[1], vPos[2] };
    }

    void ZGEOM::GetInactive(bool&)
    {
        // Do nothing
    }

    void ZGEOM::SetInactive(bool const&)
    {
        // Do nothing
    }

    void ZGEOM::GetPrim(uint32_t&)
    {
        // Do nothing
    }

    void ZGEOM::SetPrim(uint32_t const&)
    {
        // Do nothing
    }

    void ZGEOM::Delete()
    {
        if (auto* pParent = m_baseGeom->ParentGroup())
        {
            pParent->DetachGeom(m_baseGeom, false);
        }

        ZGeomBuffer::Instance().FreeGeom(this);
        this->~ZGEOM(); // Need call manually here
    }

    ZGROUP* ZGEOM::Parent() const
    {
        return BaseGeom()->ParentGroup();
    }

    ZTreeGroup* ZGEOM::GetTreeGroup() const
    {
        return BaseGeom()->GetTreeGroup();
    }
    
    ZTreeGroup* ZGEOM::GetDynamicTreeGroup() const
    {
        return BaseGeom()->GetDynamicTreeGroup();
    }

    void ZGEOM::ChangeEventsActivity()
    {
        if (m_pExData->_Events.ChkEvents() && reinterpret_cast<std::intptr_t>(m_pExData) != static_cast<std::intptr_t>(-2))
        {
            ZGeomEventListBuffers::ValueRun sRun {};
            m_pExData->_Events.InitValueRun(sRun);

            auto valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);

            while (valueFromRun != 1)
            {
                auto* event = ZEventBase::RefToPtr(valueFromRun);
                if (event)
                {
                    event->ChangeEventActivity();
                }
                else
                {
                    m_pExData->_Events.RunRemoveEvent(sRun);
                }

                m_pExData->_Events.NextValueRun(sRun);
                valueFromRun = m_pExData->_Events.GetValueFromValueRun(sRun);
            }
        }
    }

    void ZGEOM::CopyEvent(const ZEventBase* pEvent)
    {
        if (pEvent->m_ClassCall)
        {
            EnableClassCall(pEvent->m_lRoutCases);
        }
        else
        {
            const auto* pClassInfo = reinterpret_cast<const ZBaseConRout*>(pEvent)->GetRoutClassInfo();
            auto* pCopy = ZBaseConRout::GetFactory().Create(pClassInfo->ClassInfoName());

            AttachEvent(*pCopy);
            pCopy->CopyData(pEvent);
            pCopy->DoInit();
        }
    }

    void ZGEOM::VerifyPointers() const
    {
        // This method was reconstructed from PS2 build
        // I guess it should be useful to debug weird shit

        ZASSERT(m_baseGeom);
        ZASSERT(m_baseGeom->GetGeom() == this);

        ZREF rThis = GetRef();
        ZASSERT(GREFTOPTR(rThis) == this);
        ZASSERT(GREFTOBASEPTR(rThis) == m_baseGeom);
        ZASSERT(!Parent() || Parent()->IsDerivedFrom<ZGROUP>());
    }

    bool ZGEOM::Active() const
    {
        return BaseGeom()->Active();
    }

    uint32_t ZGEOM::Prim() const
    {
        return BaseGeom()->Prim();
    }

    const ZMat3x3& ZGEOM::Mat() const
    {
        return BaseGeom()->m_mMat;
    }

    const ZVector3& ZGEOM::Pos() const
    {
        return BaseGeom()->m_vPos;
    }

    const ZVector3& ZGEOM::Cen() const
    {
        return BaseGeom()->m_vCen;
    }

    const ZVector3& ZGEOM::Size() const
    {
        return BaseGeom()->m_vSize;
    }

    const float ZGEOM::Radius() const
    {
        return BaseGeom()->Radius();
    }

    const char* ZGEOM::Name() const
    {
        return BaseGeom()->Name();
    }

    ZGROUP* ZGEOM::GetOwner(bool bCheckWorldGroups)  const
    {
        return BaseGeom()->GetOwner(bCheckWorldGroups);
    }

    bool ZGEOM::ChkEvents() const
    {
        return BaseGeom()->ChkEvents();
    }

    void ZGEOM::SetName(char const* pName)
    {
        ZASSERT(pName != nullptr);
        BaseGeom()->SetName(pName);
    }

    void ZGEOM::SetMatSimple(const ZMat3x3& mMat)
    {
        BaseGeom()->SetMatSimple(mMat.Get());   
    }

    void ZGEOM::SetPosSimple(const ZVector3& vPos)
    {
        BaseGeom()->SetPosSimple(vPos.Get());
    }

    void ZGEOM::SetMatPosSimple(const ZMat3x3& mMat, const ZVector3& vPos)
    {
        BaseGeom()->SetMatSimple(mMat.Get());
        BaseGeom()->SetPosSimple(mMat.Get());
    }

    void ZGEOM::SetRootPos(const ZVector3& vPos)
    {
        SetPos(vPos); // Like in PS2
    }

    void ZGEOM::SetAutoRoomAssign(bool bAutoAssign)
    {
        BaseGeom()->SetAutoRoomAssign(bAutoAssign);
    }

    void ZGEOM::MakeDynamic(bool bDynamic)
    {
        BaseGeom()->MakeDynamic(bDynamic);
    }

    uint16_t ZGEOM::GeomControl() const
    {
        return m_lGeomControl;
    }

    void ZGEOM::SetGeomControl(uint16_t lBitsAdd, uint16_t lBitsRem)
    {
        m_lGeomControl &= ~lBitsRem;
        m_lGeomControl |= lBitsAdd;
    }

    void ZGEOM::SetControl(uint32_t lBitsAdd, uint32_t lBitsRem)
    {
        BaseGeom()->SetControl(lBitsAdd, lBitsRem);
    }

    STATIC_CLASS_VAR_IMPL(ZGEOM, ZFactory<ZGEOM>, m_Factory, 0x00972930, ZFactory<ZGEOM>{});
    STATIC_CLASS_VAR_IMPL(ZGEOM, const char*, FactoryName, 0x00769CF4, "ZGEOM");
    DECLARE_ID_AND_MASK_IMPL(ZGEOM, 0x009728C4, 0x009728C8);
    STATIC_CLASS_VAR_IMPL(ZGEOM, ZGEOMCLASSINFO*, m_OldClassInfo, 0x00972928, nullptr);
    STATIC_CLASS_VAR_IMPL(ZGEOM, ZGEOM::EStatus, m_PreferedStatus, 0x009728C0, ZGEOM::EStatus::STATUS_New);
    STATIC_CLASS_VAR_IMPL_EZ(ZGEOM, ZGEOM*, s_pLastSendCommandCaller, nullptr);
}
