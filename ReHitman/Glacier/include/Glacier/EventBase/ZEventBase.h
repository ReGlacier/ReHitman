#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZRTTI.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZList.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/Serializer/ZSerializable.h>
#include <Glacier/Geom/ZBaseGeom.h> // ZBaseGeom
#include <Glacier/ZUniMemory.h>

namespace Glacier
{
    // fwds
    class ZScheduledEvent;
    struct ZCheckPointBuffer;

    static constexpr uint32_t EV_FRMUPD = 0x8;
    static constexpr uint32_t EV_TIMEUPD = 0x10;
    
    class ZEventBase : public RTP::cBase, public ZListNode<ZEventBase, 0>
    {
    public: // Types
        enum EEventPriority : uint32_t
        {
            PRIORITY_Normal = 0x0,
            PRIORITY_AboveNormal = 0x1,
            PRIORITY_Elevator = 0x2,
            PRIORITY_High = 0x3,
            PRIORITY_VeryHigh = 0x4,
            PRIORITY_SuperHigh = 0x5,
            PRIORITY_BaseCamera = 0x6,
            PRIORITY_PostFilter = 0x7,
            PRIORITY_PostFilterHigh = 0x8,
            INACTIVE_LIST = 0x9,
            NUMBER_OF_EVENT_LISTS = 0xA,
        };

        enum EStatus : uint8_t
        {
            STATUS_New = 0x0,
            STATUS_Init = 0x1,
            STATUS_Init2 = 0x2,
            STATUS_Loaded = 0x3,
            STATUS_PostInit = 0x4,
            STATUS_PostInit2 = 0x5,
            STATUS_OK = 0x5,
            STATUS_Remove = 0x6,
            STATUS_End = 0x7,
        };

    public:
        // static
        STATIC_CLASS_VAR(ZEventBase, RTP::ZPropertyInfo, Info);
        STATIC_CLASS_VAR(ZEventBase, uint32_t, m_DirectRef);
        STATIC_CLASS_VAR(ZEventBase, bool, m_LockCreation);
        STATIC_CLASS_VAR(ZEventBase, EStatus, m_DefaultStatus);
        
        /// === vftable ===
        virtual ~ZEventBase() override = default;
        // -> ZSerializable
        // [Nothing to override]
        // -> RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;
        // -> ZEventBase
        virtual EEventPriority GetEventPriority();
        virtual void Init();
        virtual void Init2();
        virtual void PostInit();
        virtual void PostInit2();
        virtual void CopyData(const ZEventBase*);
        virtual const char* EventName();
        virtual void ExpandBounds(ZMat3x3& mMat, ZVector3& vCen, ZVector3& vSize, ZBaseGeom* pBaseGeom);
        virtual void PreSaveGame();
        virtual void RegisterInstance();
        virtual void CheckPointSave(ZCheckPointBuffer&);
        virtual void CheckPointLoad(ZCheckPointBuffer&);
        virtual void Reset();
        virtual void TimeUpdate();
        virtual void FrameUpdate();
        virtual int Command(Glacier::ZMSGID command, Glacier::ZDATA data);
        virtual int DoEvent(int lType, uint16_t lParam, void* pData);
        virtual void End();
        virtual void EditorCommand(Glacier::ZMSGID command, Glacier::ZDATA data);
        virtual void Remove();
        virtual void SchedUpdate();

        // custom operators
        static void* operator new(std::size_t size);
        static void operator delete(void* ptr) noexcept;

        // static methods
        static ZEventBase* RefToPtr(ZREF rRef);
        static void SetPreferedStatus(EStatus eStatus);

        // methods
        ZEventBase();
        void ActivateFrameUpdate(bool run_when_pause);
        void DeactivateFrameUpdate();
        void ChangeEventActivity();
        void ActivateTimeUpdate(float time_interval);
        void ChangeStatus(EStatus status, bool allowed_to_destroy);
        void Delete();
        void DetachFromGeom();
        bool SkipSave() const;
        bool IsAttached() const;
        bool IsWorking() const;
        bool IsPendingForRemoval() const;
        uint32_t GetRef() const;
        void DoInit();
        void DeactivateTimeUpdate();
        void DeactivateScheduleUpdate();
        int32_t Call(uint32_t Case, void* Param1, uint16_t Param2);
        void AttachToGeom(ZGEOM* pGeom);
        void OnlyUpdateMe(bool bOnlyUpdateMe);

        // members
        uint32_t m_Ref; // +0xC
        float m_TimerInterval; // +0x10
        TIMETYPE m_fTimePassed; // +0x14
        uint32_t m_lRoutCases; // +0x18
        uint32_t m_lEventLists; // +0x1C | Related to EV_FRMUPD, EV_TIMEUPD masks
        uint8_t m_ClassCall; // +0x20
        EStatus m_Status; // +0x21
        uint16_t m_lEventAllocSize; // +0x22
        ZGEOM* m_pBaseGeom; // +0x2C
        ZScheduledEvent* m_pScheduleEvent; // +0x30
    }; //Size: 0x002С
    RE_VERIFY_SIZE(ZEventBase, 0x2C);
    RE_VERIFY_OFFSET(ZEventBase, m_Ref, 0xC);
    RE_VERIFY_OFFSET(ZEventBase, m_ClassCall, 0x20);
    RE_VERIFY_OFFSET(ZEventBase, m_Status, 0x21);
    RE_VERIFY_OFFSET(ZEventBase, m_lEventAllocSize, 0x22);
    RE_VERIFY_OFFSET(ZEventBase, m_pBaseGeom, 0x24);
    RE_VERIFY_OFFSET(ZEventBase, m_pScheduleEvent, 0x28);

    /// <summary>
    /// Dummy class to check that offset is OK, do not use in your code!
    /// </summary>
    struct DummyEventChld : public ZEventBase { int m_Dummy; };
    RE_VERIFY_OFFSET(DummyEventChld, m_Dummy, 0x2C);
}
