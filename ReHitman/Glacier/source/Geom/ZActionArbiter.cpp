#include <Glacier/Geom/ZActionArbiter.h>

#include <Glacier/Com/CCom.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/GameBase/ZActor.h>
#include <Glacier/GameBase/ZPlayer.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZLIST.h>
#include <Glacier/PF4/ZInterface.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZUniMemory.h>

#include <cstdio>
#include <cstring>


namespace Glacier
{
    namespace
    {
        // The original never allocated m_psMessages (the engine debug info shows a
        // std::vector<SMessageInfo>-style queue that stays empty/null in the shipped
        // build). Positions inside an SMessageInfo address their three consecutive
        // float fields.
        inline float* MsgPos(ZActionArbiter::SMessageInfo& m) { return reinterpret_cast<float*>(&m.m_x); }
        inline const float* MsgPos(const ZActionArbiter::SMessageInfo& m) { return reinterpret_cast<const float*>(&m.m_x); }

        // Owns a NUL-terminated copy allocated from ZUniMemory (the original used
        // ZSysMem::allocate with strlen+1; the matching free is ZUniMemory::Free).
        char* DupString(const char* psz)
        {
            const int lLen = static_cast<int>(strlen(psz)) + 1;
            char* pCopy = static_cast<char*>(ZUniMemory::Allocate(lLen));
            strcpy(pCopy, psz);
            return pCopy;
        }
    }

    ZActionArbiter::ZActionArbiter(const char* psName, ZBaseGeom* pBaseGeom)
        : ZLIST(psName, pBaseGeom)
        , m_rtActionInfos(16, 5) // element = [actor ref][SActionInfo payload]
    {
        m_psMessages = nullptr;
        m_pPathFinder4 = nullptr;
        m_fBroadCastTime = -1.0f;
        m_fBroadCastSMSTime = -1.0f;
        m_rBroadCastReportSnd = 0;
        m_strBroadCastCustomMsg = nullptr;
        m_msgDisplayWarning = g_pEngineData->RegisterZMsg("MSG_DISPWARNING", 0, __FILE__, __LINE__);
        m_msgDisplayInfo = g_pEngineData->RegisterZMsg("MSG_DISPINFO", 0, __FILE__, __LINE__);
        m_sTgtInfo.m_sBroadCastTargetDress = nullptr;
        // m_iMsgIdx, m_iMsgMaxIdx, m_iMaxActiveInGridCell, m_fGridSize,
        // m_fMessageInterval, m_fWarnRange, m_fBroadCastDelay, m_iBroadCastReporterRef,
        // m_sTgtInfo.m_iBroadCastTargetRef and the display-enable flags are left
        // uninitialized, matching the original.
    }

    ZActionArbiter::~ZActionArbiter()
    {
        if (m_sTgtInfo.m_sBroadCastTargetDress)
        {
            ZUniMemory::Free(m_sTgtInfo.m_sBroadCastTargetDress);
            m_sTgtInfo.m_sBroadCastTargetDress = nullptr;
        }
        if (m_strBroadCastCustomMsg)
        {
            ZUniMemory::Free(m_strBroadCastCustomMsg);
            m_strBroadCastCustomMsg = nullptr;
        }
        if (m_psMessages)
        {
            ZUniMemory::Free(m_psMessages);
            m_psMessages = nullptr;
        }
        // The original dtor contains a second (redundant, provably dead) free of
        // m_strBroadCastCustomMsg after it was already nulled; it is not reproduced.
    }

    const RTP::ZPropertyInfo& ZActionArbiter::GetProperties() const
    {
        return ZActionArbiter::Info;
    }

    uint32_t ZActionArbiter::GetObjectId() const
    {
        return ZActionArbiter::m_Id;
    }

    void ZActionArbiter::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZActionArbiter::m_Id;
        mask = ZActionArbiter::m_Mask;
    }

    ZGEOMCLASSINFO* ZActionArbiter::GetOldClassInfo() const
    {
        return ZActionArbiter::m_OldClassInfo;
    }

    void ZActionArbiter::ClassInit()
    {
        ZGEOM::ClassInit();

        m_sTgtInfo.m_sBroadCastTargetDress = nullptr;
        m_strBroadCastCustomMsg = nullptr;
        m_bEnableWarnDisplay = 1;
        m_bEnableInfoDisplay = 1;
        EnableClassCall(16);
    }

    void ZActionArbiter::ClassInit2()
    {
        m_pPathFinder4 = g_pEngineData->m_pPathfinder4Data;
    }

    void ZActionArbiter::ClassEnd()
    {
        if (m_sTgtInfo.m_sBroadCastTargetDress)
        {
            ZUniMemory::Free(m_sTgtInfo.m_sBroadCastTargetDress);
            m_sTgtInfo.m_sBroadCastTargetDress = nullptr;
        }
        if (m_strBroadCastCustomMsg)
        {
            ZUniMemory::Free(m_strBroadCastCustomMsg);
            m_strBroadCastCustomMsg = nullptr;
        }
    }

    void ZActionArbiter::AddGeom(ZREF rGeom)
    {
        ZLIST::AddGeom(rGeom);

        // REFTAB element layout: [0] = actor ref key, [1..5] = SActionInfo payload.
        uint32_t* pKey = m_rtActionInfos.Add(rGeom);
        reinterpret_cast<SActionInfo*>(pKey + 1)->reset();
    }

    void ZActionArbiter::RemoveGeomById(ZREF rGeom)
    {
        ZLIST::RemoveGeomById(rGeom);
        m_rtActionInfos.Remove(rGeom);
    }

    void ZActionArbiter::Script_EnableWarnDisplay(int bEnable)
    {
        m_bEnableWarnDisplay = bEnable != 0;
    }

    void ZActionArbiter::Script_EnableInfoDisplay(int bEnable)
    {
        m_bEnableInfoDisplay = bEnable != 0;
    }

    void ZActionArbiter::Reset()
    {
        ZASSERT(m_pZList != nullptr);
        ZASSERT(m_rtActionInfos.Count() == m_pZList->Count());

        RefRun run;
        m_rtActionInfos.RunInitNxtRef(&run);
        for (uint32_t* pKey = m_rtActionInfos.RunNxtRefPtr(&run); pKey; pKey = m_rtActionInfos.RunNxtRefPtr(&run))
        {
            reinterpret_cast<SActionInfo*>(pKey + 1)->reset();
        }

        // Only the write cursor is cleared; m_iMsgMaxIdx (live entry count) is kept.
        m_iMsgIdx = 0;
    }

    void ZActionArbiter::ResetSingle(int rActor)
    {
        ZASSERT(m_pZList != nullptr);
        ZASSERT(m_rtActionInfos.Count() == m_pZList->Count());

        uint32_t* pKey = m_rtActionInfos.Find(static_cast<uint32_t>(rActor));
        if (pKey)
        {
            reinterpret_cast<SActionInfo*>(pKey + 1)->reset();
            return;
        }

        MYSTR selfName = CalcTotalName(true);
        ZGEOM* pObject = ZGEOM::RefToPtr(static_cast<uint32_t>(rActor));
        MYSTR objectName = pObject ? pObject->CalcTotalName(true) : MYSTR("");
        printf("ZActionArbiter %s couldn't reset info for object %s (not found in list)\n",
               static_cast<char*>(selfName), static_cast<char*>(objectName));
    }

    int32_t ZActionArbiter::ApproachCrimeScene(void* pCSI)
    {
        ZASSERT(m_pZList != nullptr);
        ZASSERT(m_rtActionInfos.Count() == m_pZList->Count());

        const CSI* pCtx = static_cast<const CSI*>(pCSI);
        int32_t lResult = 0;

        RefRun run;
        m_rtActionInfos.RunInitNxtRef(&run);
        for (uint32_t* pKey = m_rtActionInfos.RunNxtRefPtr(&run); pKey; pKey = m_rtActionInfos.RunNxtRefPtr(&run))
        {
            SActionInfo* pInfo = reinterpret_cast<SActionInfo*>(pKey + 1);

            // The original also computes a vector difference here when the stored
            // react event matches the incoming one, but the result is never used
            // (dead code in the PC release build); it is not reproduced.

            if (static_cast<int32_t>(pKey[0]) == pCtx->m_rObject)
            {
                pInfo->m_iReactEvent = pCtx->m_iReactEvent;
                pInfo->m_x = pCtx->m_x;
                pInfo->m_y = pCtx->m_y;
                pInfo->m_z = pCtx->m_z;
                lResult = 1;
            }
        }
        return lResult;
    }

    bool ZActionArbiter::QueueMessage(SMessageInfo& rMsg)
    {
        const float fNow = static_cast<float>(g_pSysInterface->FrameTime.secs) * TIMETYPE::kInvTPS;

        if (m_iMsgMaxIdx <= 0)
            return true;

        for (int i = 0; i < m_iMsgMaxIdx; ++i)
        {
            // m_psMessages is never allocated by the original, so this dereferences
            // null in the captured build; the behaviour is preserved.
            SMessageInfo& rEntry = m_psMessages[i];

            if (rEntry.m_iType == rMsg.m_iType)
            {
                // The original matches the queue entry against the grid size, not a
                // separate distance field (PC 0x500DF0; PS2 compares 2D only, PC is 3D).
                if (vdist(MsgPos(rEntry), MsgPos(rMsg)) * 0.0099999998f < m_fGridSize &&
                    fNow - rEntry.m_fTime < m_fMessageInterval)
                {
                    return false;
                }
            }
        }
        return true;
    }

    void ZActionArbiter::ReportDisplayWarning(void* pData)
    {
        SMS* pSms = static_cast<SMS*>(pData);

        SMessageInfo q;
        q.m_fTime = static_cast<float>(g_pSysInterface->FrameTime.secs) * TIMETYPE::kInvTPS;
        q.m_iType = pSms->m_iType;
        q.m_x = pSms->m_x;
        q.m_y = pSms->m_y;
        q.m_z = pSms->m_z;

        if (!QueueMessage(q))
            return;
        if (!m_bEnableWarnDisplay)
            return;

        m_psMessages[m_iMsgIdx] = q;
        if (m_iMsgMaxIdx < 10)
            ++m_iMsgMaxIdx;
        m_iMsgIdx = (m_iMsgIdx + 1) % 10;

        ZPlayer* pPlayer = g_pGameData ? g_pGameData->GetPlayer(0) : nullptr;
        // PC: vftable offset +0x3DC (0x5278C0) is ZPlayer::IsDead(), which tests
        // m_CurrentStatus bit 18 (0x00040000).
        if (pPlayer && pPlayer->IsDead())
            return;

        CCom* pCom = g_pEngineData->GetSceneCom();
        int lBriefer = 0;
        pCom->GetVal("rIngameBriefer", &lBriefer);
        SendCommand(static_cast<ZREF>(lBriefer), m_msgDisplayWarning, static_cast<char*>(pSms->m_msg));
    }

    void ZActionArbiter::Script_ReportDisplayInfo(void* pData)
    {
        SMS* pSms = static_cast<SMS*>(pData);

        SMessageInfo q;
        q.m_fTime = static_cast<float>(g_pSysInterface->FrameTime.secs) * TIMETYPE::kInvTPS;
        q.m_iType = pSms->m_iType;
        q.m_x = pSms->m_x;
        q.m_y = pSms->m_y;
        q.m_z = pSms->m_z;

        if (!QueueMessage(q))
            return;
        if (!m_bEnableInfoDisplay)
            return;

        m_psMessages[m_iMsgIdx] = q;
        if (m_iMsgMaxIdx < 10)
            ++m_iMsgMaxIdx;
        m_iMsgIdx = (m_iMsgIdx + 1) % 10;

        ZPlayer* pPlayer = g_pGameData ? g_pGameData->GetPlayer(0) : nullptr;
        if (pPlayer && pPlayer->IsDead())
            return;

        CCom* pCom = g_pEngineData->GetSceneCom();
        int lBriefer = 0;
        pCom->GetVal("rIngameBriefer", &lBriefer);
        SendCommand(static_cast<ZREF>(lBriefer), m_msgDisplayInfo, static_cast<char*>(pSms->m_msg));
    }

    bool ZActionArbiter::ActorAlive(ZGEOM* pGeom)
    {
        if (!pGeom || !pGeom->IsDerivedFrom<ZActor>())
            return false;
        return geom_cast<ZActor>(pGeom)->GetActorState() != 0;
    }

    void ZActionArbiter::SetGridSize(float fSize)
    {
        m_fGridSize = fSize;
    }

    void ZActionArbiter::SetMaxActive(int lMax)
    {
        m_iMaxActiveInGridCell = lMax;
    }

    uint32_t ZActionArbiter::GetClosestMemberFromDistance(uint32_t rRef, float* pDistance)
    {
        ZGEOM* pSelf = ZGEOM::RefToPtr(rRef);
        if (!pSelf)
            return 0;

        ZVector3 vSelf;
        vreset(vSelf);
        pSelf->GetRootPoint(vSelf);

        uint32_t lFound = 0;
        float fBest = 9.9999997e37f;

        if (m_pZList)
        {
            RefRun run;
            m_pZList->RunInitNxtRef(&run);
            for (uint32_t r = m_pZList->RunNxtRef(&run); run; r = m_pZList->RunNxtRef(&run))
            {
                if (r == rRef)
                    continue;

                ZVector3 vOther;
                vreset(vOther);
                ZGEOM::RefToPtr(r)->GetRootPoint(vOther);

                const float fDist = vdist(vSelf, vOther);
                if (fDist < fBest && fDist > *pDistance)
                {
                    fBest = fDist;
                    lFound = r;
                }
            }
        }

        if (lFound)
            *pDistance = fBest;
        return lFound;
    }

    void ZActionArbiter::QueryAttackTarget(int rAttacker)
    {
        ZGEOM* pAttacker = ZGEOM::RefToPtr(static_cast<uint32_t>(rAttacker));
        if (!pAttacker)
            return;

        ZVector3 vAttacker;
        vreset(vAttacker);
        pAttacker->GetRootPoint(vAttacker);

        const ZMSGID msg = g_pEngineData->RegisterZMsg("QueryAttackTarget", 0, __FILE__, __LINE__);

        if (!ZLIST::m_TrackLinkObjectsInstance || !m_pZList)
            return;

        RefRun run;
        m_pZList->RunInitNxtRef(&run);
        for (uint32_t r = m_pZList->RunNxtRef(&run); run; r = m_pZList->RunNxtRef(&run))
        {
            ZGEOM* pMember = ZGEOM::RefToPtr(r);
            if (!pMember || !pMember->IsDerivedFrom<ZActor>() || pMember == pAttacker)
                continue;

            ZVector3 vMember;
            vreset(vMember);
            pMember->GetRootPoint(vMember);

            if (vdist(vAttacker, vMember) * 0.0099999998f < m_fWarnRange)
                pMember->SendCommand(msg, nullptr, nullptr);
        }
    }

    void ZActionArbiter::AttackingTarget(void* pAttack)
    {
        const SAtt* pAtt = static_cast<const SAtt*>(pAttack);

        ZGEOM* pAttacker = ZGEOM::RefToPtr(static_cast<uint32_t>(pAtt->m_rAttacker));
        ZGEOM* pTarget = ZGEOM::RefToPtr(static_cast<uint32_t>(pAtt->m_rTarget));
        if (!pTarget || !pAttacker)
            return;

        ZVector3 vAttacker;
        vreset(vAttacker);
        pAttacker->GetRootPoint(vAttacker);

        const ZMSGID msgForceVisible = g_pEngineData->RegisterZMsg("MSG_ForceVisible", 0, __FILE__, __LINE__);
        // The original registers "ScriptEvent" here too but sends it through the
        // arbiter's own SendCommand; the returned id is unused as a named handle.
        (void)g_pEngineData->RegisterZMsg("ScriptEvent", 0, __FILE__, __LINE__);

        ZLIST* pList = ZLIST::m_TrackLinkObjectsInstance;
        if (!pList || !m_pZList)
            return;

        RefRun run;
        m_pZList->RunInitNxtRef(&run);
        for (uint32_t r = m_pZList->RunNxtRef(&run); run; r = m_pZList->RunNxtRef(&run))
        {
            ZGEOM* pMember = ZGEOM::RefToPtr(r);
            if (!pMember || !ActorAlive(pMember))
                continue;
            if (pMember == pAttacker || pMember == pTarget)
                continue;

            ZVector3 vMember;
            vreset(vMember);
            pMember->GetRootPoint(vMember);

            // The original evaluates the same member<->attacker distance twice
            // (once via vsub+length, once via vdist); one check is equivalent.
            if (vdist(vAttacker, vMember) * 0.0099999998f >= m_fWarnRange)
                continue;

            const uint32_t aVisiblePayload[2] = { r, static_cast<uint32_t>(pAtt->m_rTarget) };
            pList->SendCommandToList(msgForceVisible, const_cast<uint32_t*>(aVisiblePayload));

            // Script payload: { "ReportNewTarget", target ref, dress name }. The
            // duplicated dress string is leaked by the original (consumed by the
            // script receiver); kept as-is.
            void* aScriptPayload[3];
            aScriptPayload[0] = const_cast<char*>("ReportNewTarget");
            aScriptPayload[1] = reinterpret_cast<void*>(static_cast<uintptr_t>(pAtt->m_rTarget));
            aScriptPayload[2] = DupString(pAtt->m_szDressName);
            SendCommand(r, g_pEngineData->RegisterZMsg("ScriptEvent", 0, __FILE__, __LINE__), aScriptPayload);
        }
    }

    void ZActionArbiter::BroadcastTarget(void* pTarget)
    {
        const STarget* pTgt = static_cast<const STarget*>(pTarget);

        if (m_sTgtInfo.m_sBroadCastTargetDress &&
            stricmp(m_sTgtInfo.m_sBroadCastTargetDress, pTgt->m_szDressName) == 0 &&
            m_sTgtInfo.m_iBroadCastTargetRef == pTgt->m_rTarget)
        {
            return;
        }

        const int lNow = g_pSysInterface->FrameTime.secs;
        const int lDelay = static_cast<int>(m_fBroadCastDelay * TIMETYPE::kTicksPerSecond);
        m_fBroadCastTime = static_cast<float>(TIMETYPE(lNow + lDelay).secs) * TIMETYPE::kInvTPS;

        m_sTgtInfo.m_iBroadCastTargetRef = pTgt->m_rTarget;
        if (m_sTgtInfo.m_sBroadCastTargetDress)
            ZUniMemory::Free(m_sTgtInfo.m_sBroadCastTargetDress);
        m_sTgtInfo.m_sBroadCastTargetDress = DupString(pTgt->m_szDressName);
        m_iBroadCastReporterRef = pTgt->m_rReporter;
    }

    void ZActionArbiter::BroadcastTargetMsg(void* pTargetMsg)
    {
        const TGT* pTgt = static_cast<const TGT*>(pTargetMsg);

        if (m_sTgtInfo.m_sBroadCastTargetDress &&
            stricmp(m_sTgtInfo.m_sBroadCastTargetDress, pTgt->m_szDressName) == 0 &&
            m_sTgtInfo.m_iBroadCastTargetRef == pTgt->m_rTarget)
        {
            return;
        }

        if (m_strBroadCastCustomMsg)
        {
            ZUniMemory::Free(m_strBroadCastCustomMsg);
            m_strBroadCastCustomMsg = nullptr;
        }
        m_strBroadCastCustomMsg = DupString(pTgt->m_szMsg);

        // A TGT starts with a valid STarget prefix.
        BroadcastTarget(const_cast<TGT*>(pTgt));
    }

    void ZActionArbiter::BroadcastMessage(uint32_t rActor, const char* szMessage)
    {
        const ZMSGID msg = g_pEngineData->RegisterZMsg(szMessage, 0, __FILE__, __LINE__);

        if (!m_pZList)
            return;

        RefRun run;
        m_pZList->RunInitNxtRef(&run);
        for (uint32_t r = m_pZList->RunNxtRef(&run); run; r = m_pZList->RunNxtRef(&run))
        {
            if (r != rActor)
                SendCommand(r, msg, nullptr);
        }
    }

    void ZActionArbiter::BroadcastMessagePos(uint32_t rActor, const char* szMessage, float* pPos)
    {
        const ZMSGID msg = g_pEngineData->RegisterZMsg("ScriptEvent", 0, __FILE__, __LINE__);

        if (!m_pZList)
            return;

        evt payload;
        payload.m_szName = szMessage;
        payload.m_x = pPos[0];
        payload.m_y = pPos[1];
        payload.m_z = pPos[2];

        RefRun run;
        m_pZList->RunInitNxtRef(&run);
        for (uint32_t r = m_pZList->RunNxtRef(&run); run; r = m_pZList->RunNxtRef(&run))
        {
            if (r != rActor)
                SendCommand(r, msg, &payload);
        }
    }

    void ZActionArbiter::BroadcastMessageRef(uint32_t rActor, const char* szMessage, uint32_t rTarget)
    {
        const ZMSGID msg = g_pEngineData->RegisterZMsg("ScriptEvent", 0, __FILE__, __LINE__);

        if (!m_pZList)
            return;

        uint32_t payload[2];
        payload[0] = reinterpret_cast<uint32_t>(szMessage);
        payload[1] = rTarget;

        RefRun run;
        m_pZList->RunInitNxtRef(&run);
        for (uint32_t r = m_pZList->RunNxtRef(&run); run; r = m_pZList->RunNxtRef(&run))
        {
            if (r != rActor)
                SendCommand(r, msg, payload);
        }
    }

    void ZActionArbiter::BroadcastMessageClosestN(uint32_t rActor, const char* szMessage, int lCount)
    {
        const ZMSGID msg = g_pEngineData->RegisterZMsg("ScriptEvent", 0, __FILE__, __LINE__);

        float fDistance = 0.0f;
        for (int i = 0; i < lCount; ++i)
        {
            const uint32_t r = GetClosestMemberFromDistance(rActor, &fDistance);
            if (!r)
                break;

            // Script payload: pointer to the message-name pointer (single indirection).
            const char* pszName = szMessage;
            SendCommand(r, msg, const_cast<const char**>(&pszName));
        }
    }

    void ZActionArbiter::SetSharedRef(uint32_t rRef)
    {
        m_rSharedRef = rRef;
    }

    uint32_t ZActionArbiter::GetSharedRef()
    {
        return m_rSharedRef;
    }

    void ZActionArbiter::ClassFrameUpdate()
    {
        const int lNow = g_pSysInterface->FrameTime.secs;

        if (m_fBroadCastTime <= 0.0f || TIMETYPE(m_fBroadCastTime).secs >= lNow)
        {
            if (m_fBroadCastSMSTime > 0.0f && TIMETYPE(m_fBroadCastSMSTime).secs < lNow)
            {
                ReportDisplayWarning(&m_sSMS);
                m_fBroadCastSMSTime = -1.0f;
                if (m_strBroadCastCustomMsg)
                {
                    ZUniMemory::Free(m_strBroadCastCustomMsg);
                    m_strBroadCastCustomMsg = nullptr;
                }
            }
            return;
        }

        // The deferred target report is due.
        ZGEOM* pReporter = ZGEOM::RefToPtr(static_cast<uint32_t>(m_iBroadCastReporterRef));
        if (!ActorAlive(pReporter))
            return;

        const ZMSGID msg = g_pEngineData->RegisterZMsg("ScriptEvent", 0, __FILE__, __LINE__);

        void* aPayload[3];
        aPayload[0] = const_cast<char*>("ReportNewTarget");
        aPayload[1] = reinterpret_cast<void*>(static_cast<uintptr_t>(m_sTgtInfo.m_iBroadCastTargetRef));

        if (m_pZList)
        {
            RefRun run;
            m_pZList->RunInitNxtRef(&run);
            for (uint32_t r = m_pZList->RunNxtRef(&run); run; r = m_pZList->RunNxtRef(&run))
            {
                if (m_iBroadCastReporterRef == static_cast<int32_t>(r))
                    continue;

                // The dress copy is re-allocated per member and leaked, like in the
                // original.
                aPayload[2] = DupString(m_sTgtInfo.m_sBroadCastTargetDress);
                SendCommand(r, msg, aPayload);
            }
        }

        m_fBroadCastTime = -1.0f;
        ZUniMemory::Free(m_sTgtInfo.m_sBroadCastTargetDress);
        m_sTgtInfo.m_sBroadCastTargetDress = nullptr;

        pReporter = ZGEOM::RefToPtr(static_cast<uint32_t>(m_iBroadCastReporterRef));
        if (!pReporter)
            return;

        ZMat3x3 mat;
        ZVector3 pos;
        pReporter->GetRootTM(mat, pos);

        if (m_rBroadCastReportSnd)
        {
            if (ZGROUP* pGroup = BaseGeom()->ParentGroup())
                pGroup->AddSound3d(mat.data, pos.Get(), static_cast<int>(m_rBroadCastReportSnd), 0, 0, 0);
        }

        MYSTR text = m_strBroadCastCustomMsg
            ? MYSTR(m_strBroadCastCustomMsg)
            : MYSTR("AllLevels/Warnings/NearByGuardsAlarmed");
        m_sSMS.m_msg = text;
        m_sSMS.m_iType = 999;
        m_sSMS.m_x = pos.x;
        m_sSMS.m_y = pos.y;
        m_sSMS.m_z = pos.z;

        // Schedule the deferred warning 1 second later.
        m_fBroadCastSMSTime = static_cast<float>(TIMETYPE(lNow + 1024).secs) * TIMETYPE::kInvTPS;
    }

#pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(
        ZActionArbiter,
        ZLIST,
        0x0097BC90,
        "ZActionArbiter",
        0x00770A04,
        nullptr, // No first property (inherits ZLIST properties)
        0x00809384,
        0x0097BC3C,
        0x0097BC40
    );
#pragma endregion
}
