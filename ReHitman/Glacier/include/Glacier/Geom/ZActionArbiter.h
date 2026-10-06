#pragma once

#include <Glacier/Runtime/Macro.h>
#include <Glacier/Geom/ZLIST.h>
#include <Glacier/PF4/ZInterface.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <Glacier/ZSTL/REFTAB.h>


namespace Glacier
{
    // Runtime helper for the AI/actor action arbiter (geom factory "ActionArbiter").
    // Tracks the actors registered to the arbiter (m_rtActionInfos), decides whether
    // they may perform a context action (attack / crime scene) and broadcasts
    // warning/info display messages to the ingame briefer.
    // Source: engine/geomsextend/zactionarbiter.cpp
    class ZActionArbiter : public ZLIST
    {
    public:
        // ---- nested data (from the engine debug info) ----------------------

        // Queue entry for the display-message queue (size 0x14).
        struct SMessageInfo
        {
            float m_fTime;       //+0x00 game time the message was queued at
            int32_t m_iType;     //+0x04 message kind tag
            float m_x;           //+0x08 world position of the event
            float m_y;           //+0x0C
            float m_z;           //+0x10
        };
        RE_VERIFY_SIZE(SMessageInfo, 0x14);

        // Per-actor bookkeeping stored inline in m_rtActionInfos.
        // REFTAB element = 4-byte actor ref key followed by this 0x14 payload.
        struct SActionInfo
        {
            int32_t m_iStatus;     //+0x00
            int32_t m_iReactEvent; //+0x04
            float m_x;             //+0x08
            float m_y;             //+0x0C
            float m_z;             //+0x10

            void reset()
            {
                m_iStatus = -1;
                m_iReactEvent = -1;
            }
        };
        RE_VERIFY_SIZE(SActionInfo, 0x14);

        // "Current suspicious thing" payload handed to SActionInfo slots.
        struct CSI
        {
            int32_t m_rObject;     //+0x00
            int32_t m_iReactEvent; //+0x04
            float m_x;             //+0x08
            float m_y;             //+0x0C
            float m_z;             //+0x10
        };
        RE_VERIFY_SIZE(CSI, 0x14);

        // Attack report payload (AttackingTarget argument).
        struct SAtt
        {
            int32_t m_rAttacker;   //+0x00
            int32_t m_rTarget;     //+0x04
            const char* m_szDressName; //+0x08
        };
        RE_VERIFY_SIZE(SAtt, 0xC);

        // "Somebody reported a target" payload (BroadcastTarget argument).
        struct STarget
        {
            int32_t m_rReporter;   //+0x00
            int32_t m_rTarget;     //+0x04
            const char* m_szDressName; //+0x08
        };
        RE_VERIFY_SIZE(STarget, 0xC);

        // BroadcastTargetMsg argument: target report plus the message to relay.
        struct TGT
        {
            int32_t m_rReporter;   //+0x00
            int32_t m_rTarget;     //+0x04
            const char* m_szDressName; //+0x08
            const char* m_szMsg;       //+0x0C
        };
        RE_VERIFY_SIZE(TGT, 0x10);

        // ScriptEvent payload: script function name plus an optional position.
        struct evt
        {
            const char* m_szName;  //+0x00
            float m_x;             //+0x04
            float m_y;             //+0x08
            float m_z;             //+0x0C
        };
        RE_VERIFY_SIZE(evt, 0x10);

        // Deferred display message ("SMS") held by this arbiter.
        struct SMS
        {
            MYSTR m_msg;           //+0x00
            int32_t m_iType;       //+0x80
            float m_x;             //+0x84
            float m_y;             //+0x88
            float m_z;             //+0x8C
        };
        RE_VERIFY_SIZE(SMS, 0x90);

        // Currently broadcast target description.
        struct STgtInfo
        {
            int32_t m_iBroadCastTargetRef; //+0x00
            char* m_sBroadCastTargetDress; //+0x04
        };
        RE_VERIFY_SIZE(STgtInfo, 0x8);

        // ---- RTTI ----------------------------------------------------------
        DECLARE_GEOM_CLASS(ZActionArbiter, 0x80000C9u);

        // ---- vtable (order matches the original ZActionArbiter_vftable) ----
        ~ZActionArbiter() override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // ZGEOM class-level lifecycle
        void ClassInit() override;
        void ClassInit2() override;
        virtual void ClassEnd();
        void ClassFrameUpdate() override;

        // Listener hooks: the deferred warning/info display messages.
        virtual void ReportDisplayWarning(void* pData);
        virtual void Script_ReportDisplayInfo(void* pData);

        // Script entry points
        virtual void Script_EnableWarnDisplay(int bEnable);
        virtual void Script_EnableInfoDisplay(int bEnable);
        virtual int32_t ApproachCrimeScene(void* pCSI);
        virtual void Reset();
        virtual void ResetSingle(int rActor);
        virtual void QueryAttackTarget(int rAttacker);
        virtual void AttackingTarget(void* pAttack);
        virtual void BroadcastTarget(void* pTarget);
        virtual void BroadcastTargetMsg(void* pTargetMsg);
        virtual void BroadcastMessage(uint32_t rActor, const char* szMessage);
        virtual void BroadcastMessagePos(uint32_t rActor, const char* szMessage, float* pPos);
        virtual void BroadcastMessageRef(uint32_t rActor, const char* szMessage, uint32_t rTarget);
        virtual void BroadcastMessageClosestN(uint32_t rActor, const char* szMessage, int lCount);
        virtual void SetSharedRef(uint32_t rRef);
        virtual uint32_t GetSharedRef();

        // ZLIST
        void AddGeom(ZREF rGeom) override;
        void RemoveGeomById(ZREF rGeom) override;

        // ---- api -----------------------------------------------------------
        bool QueueMessage(SMessageInfo& rMsg);

        // ---- lifecycle -----------------------------------------------------
        ZActionArbiter(const char* psName, ZBaseGeom* pBaseGeom);

        // ---- helpers -------------------------------------------------------
        bool ActorAlive(ZGEOM* pGeom);
        void SetGridSize(float fSize);
        void SetMaxActive(int lMax);
        uint32_t GetClosestMemberFromDistance(uint32_t rRef, float* pDistance);

        // ---- members (PC layout; the PS2 build keeps this block at +4) -----
        SMessageInfo* m_psMessages;            //+0x14 queued display messages (owned)
        int32_t m_iMsgIdx;                     //+0x18 write cursor into m_psMessages
        int32_t m_iMsgMaxIdx;                  //+0x1C live queue entries
        REFTAB m_rtActionInfos;                //+0x20 actor refs keyed SActionInfo table
        PF4::ZInterface* m_pPathFinder4;       //+0x3C PF4 runtime interface (ClassInit2)
        ZMSGID m_msgDisplayWarning;            //+0x40 "MSG_DISPWARNING"
        ZMSGID m_msgDisplayInfo;               //+0x42 "MSG_DISPINFO"
        int32_t m_iMaxActiveInGridCell;        //+0x44
        float m_fGridSize;                     //+0x48
        float m_fMessageInterval;              //+0x4C
        float m_fWarnRange;                    //+0x50 range for attack/crime evaluation
        uint32_t m_rBroadCastReportSnd;        //+0x54 sound played when a target is reported
        float m_fBroadCastTime;                //+0x58 deadline for the deferred report (-1 idle)
        float m_fBroadCastDelay;               //+0x5C
        int32_t m_iBroadCastReporterRef;       //+0x60 actor that reported the current target
        float m_fBroadCastSMSTime;             //+0x64 deadline for the deferred SMS (-1 idle)
        char* m_strBroadCastCustomMsg;         //+0x68 custom deferred message (owned)
        SMS m_sSMS;                            //+0x6C deferred display message
        STgtInfo m_sTgtInfo;                   //+0xFC current broadcast target
        uint8_t m_bEnableWarnDisplay;          //+0x104
        uint8_t m_bEnableInfoDisplay;          //+0x105
        uint8_t m_Pad0x106[0x02];              //+0x106
        uint32_t m_rSharedRef;                 //+0x108
        uint8_t m_Pad0x10C[0x04];              //+0x10C
    };
    RE_VERIFY_SIZE(ZActionArbiter, 0x110);

    RE_VERIFY_OFFSET(ZActionArbiter, m_psMessages, 0x14);
    RE_VERIFY_OFFSET(ZActionArbiter, m_iMsgIdx, 0x18);
    RE_VERIFY_OFFSET(ZActionArbiter, m_iMsgMaxIdx, 0x1C);
    RE_VERIFY_OFFSET(ZActionArbiter, m_rtActionInfos, 0x20);
    RE_VERIFY_OFFSET(ZActionArbiter, m_pPathFinder4, 0x3C);
    RE_VERIFY_OFFSET(ZActionArbiter, m_msgDisplayWarning, 0x40);
    RE_VERIFY_OFFSET(ZActionArbiter, m_msgDisplayInfo, 0x42);
    RE_VERIFY_OFFSET(ZActionArbiter, m_iMaxActiveInGridCell, 0x44);
    RE_VERIFY_OFFSET(ZActionArbiter, m_fGridSize, 0x48);
    RE_VERIFY_OFFSET(ZActionArbiter, m_fMessageInterval, 0x4C);
    RE_VERIFY_OFFSET(ZActionArbiter, m_fWarnRange, 0x50);
    RE_VERIFY_OFFSET(ZActionArbiter, m_rBroadCastReportSnd, 0x54);
    RE_VERIFY_OFFSET(ZActionArbiter, m_fBroadCastTime, 0x58);
    RE_VERIFY_OFFSET(ZActionArbiter, m_fBroadCastDelay, 0x5C);
    RE_VERIFY_OFFSET(ZActionArbiter, m_iBroadCastReporterRef, 0x60);
    RE_VERIFY_OFFSET(ZActionArbiter, m_fBroadCastSMSTime, 0x64);
    RE_VERIFY_OFFSET(ZActionArbiter, m_strBroadCastCustomMsg, 0x68);
    RE_VERIFY_OFFSET(ZActionArbiter, m_sSMS, 0x6C);
    RE_VERIFY_OFFSET(ZActionArbiter, m_sTgtInfo, 0xFC);
    RE_VERIFY_OFFSET(ZActionArbiter, m_bEnableWarnDisplay, 0x104);
    RE_VERIFY_OFFSET(ZActionArbiter, m_bEnableInfoDisplay, 0x105);
    RE_VERIFY_OFFSET(ZActionArbiter, m_rSharedRef, 0x108);
}
