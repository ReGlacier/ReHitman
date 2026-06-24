#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZRTTI.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZListNodeBase.h>
#include <Glacier/ZScheduledScript.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/ZSTL/TIMETYPE.h>

namespace Glacier
{
    class ZEventBase : public ZListNode<ZEventBase, 0>
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
        /// === vftable ===
        virtual void Release(bool);
        virtual void PreSave(ISerializerStream*);
        virtual void PostSave(ISerializerStream*);
        virtual void PreLoad(ISerializerStream*);
        virtual bool PostLoad(ISerializerStream*);
        virtual bool PostProcess(const unsigned int, const unsigned int);
        virtual void LoadSave(Glacier::ZPackedInput*, bool);
        virtual void LoadObject(IOutputSerializerStream*);
        virtual void SaveObject(int&);
        virtual void ExchangeObject(Glacier::ZPackedInput*);
        virtual void SetToDefault();
        virtual unsigned int GetTypeID();
        virtual RTP::ZPropertyInfo* GetProperties();
        virtual EEventPriority GetEventPriority();
        virtual void Init();
        virtual void Init2();
        virtual void PostInit();
        virtual void PostInit2();
        virtual void CopyData(const ZEventBase*);
        virtual const char* EventName();
        virtual void ExpandBounds(float*, float*, float*, Glacier::ZEntityLocator*);
        virtual void PreSaveGame();
        virtual void RegisterInstance();
        virtual void CheckPointSave(int&);
        virtual void CheckPointLoad(int&);
        virtual void Reset();
        virtual void TimeUpdate();
        virtual void FrameUpdate();
        virtual int Command(Glacier::ZMSGID command, Glacier::ZDATA data);
        virtual int DoEvent(int, int, void*);
        virtual void End();
        virtual void EditorCommand(Glacier::ZMSGID command, Glacier::ZDATA data);
        virtual void Remove();
        virtual void SchedUpdate();

        // api
        void ActivateFrameUpdate(bool a1);
        void DeactivateFrameUpdate();
        void ChangeEventActivity();
        void ActivateTimeUpdate(float);

        // static helpers
        static int* GetDefaultStatus();

        // members
        uint32_t m_Ref;
        float m_TimerInterval;
        TIMETYPE m_fTimePassed;
        uint32_t m_lRoutCases;
        uint32_t m_lEventLists;
        uint8_t m_ClassCall;
        EStatus m_Status;
        unsigned __int16 m_lEventAllocSize;
        ZGEOM* m_pBaseGeom;
        ZScheduledEvent* m_pScheduleEvent;
    }; //Size: 0x002С
    RE_VERIFY_SIZE(ZEventBase, 0x2C);
    RE_VERIFY_OFFSET(ZEventBase, m_Ref, 0xC);
    RE_VERIFY_OFFSET(ZEventBase, m_pBaseGeom, 0x24);
    RE_VERIFY_OFFSET(ZEventBase, m_pScheduleEvent, 0x28);

    /// <summary>
    /// Dummy class to check that offset is OK, do not use in your code!
    /// </summary>
    struct DummyEventChld : public ZEventBase { int m_Dummy; };
    RE_VERIFY_OFFSET(DummyEventChld, m_Dummy, 0x2C);
}