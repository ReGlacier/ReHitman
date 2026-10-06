#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Geom/ZLIST.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/ZDistCheck.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/ZSTL/REFTAB32.h>

#include <cstdint>

namespace Glacier
{
    struct ISerializerStream;
    struct IInputSerializerStream;
    struct IOutputSerializerStream;

    class ZAction;

    // Scene-wide action dispatcher event (created as the "ZLIST_ActionController"
    // event on a ZLIST geometry). It tracks a fixed set of ZDistCheck tables and a
    // set of registered ZAction objects, updating which actions are currently valid
    // for the player every frame.
    class ZActionController : public CBaseEvent<ZLIST>
    {
    public:
        // RTTI
        DECLARE_ROUT_CLASS(ZActionController, ZLIST, ActionController, 0, 0);

        // static
        static REFTAB32* m_pDeleteActions; // PC 0x99CB74

        // vtbl
        ~ZActionController() override;
        const RTP::ZPropertyInfo& GetProperties() const override;
        void Init() override;
        void End() override;
        void LoadObject(IInputSerializerStream& stream) override;
        void SaveObject(IOutputSerializerStream& stream) override;
        void FrameUpdate() override;
        int Command(Glacier::ZMSGID command, Glacier::ZDATA data) override;

        // The base ZEventBase::Remove() (no argument) stays as an inherited slot;
        // these Add/Remove overloads manage the controller's own ZAction list.
        virtual void Add(ZAction* pAction);
        virtual void Remove(ZAction* pAction);

        // static methods
        static ZActionController* GetCurrentController(
            bool bCreateActionControllerIfItNotCreatedYet);

        // methods
        ZActionController();

        bool UpdateSingleObject(ZAction* pAction);
        void AddEvaluator(unsigned int lIndex);
        void RemoveEvaluator(unsigned int lIndex);

        // members (data begins at +0x30, total size 0x10A0)
        int32_t m_iEvaluators;         // +0x30
        int32_t m_alNextAction[4];     // +0x34
        ZDistCheck* m_apDistCheck[4];  // +0x44
        ZDistCheck m_DistCheck;        // +0x54
        ZMSGID m_msgAddEvaluator;      // +0x109c
        ZMSGID m_msgRemoveEvaluator;   // +0x109e
    };
    RE_VERIFY_SIZE(ZActionController, 0x10A0); // PC verified
    RE_VERIFY_OFFSET(ZActionController, m_iEvaluators, 0x30);
    RE_VERIFY_OFFSET(ZActionController, m_alNextAction, 0x34);
    RE_VERIFY_OFFSET(ZActionController, m_apDistCheck, 0x44);
    RE_VERIFY_OFFSET(ZActionController, m_DistCheck, 0x54);
    RE_VERIFY_OFFSET(ZActionController, m_msgAddEvaluator, 0x109c);
    RE_VERIFY_OFFSET(ZActionController, m_msgRemoveEvaluator, 0x109e);
}
