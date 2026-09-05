#include <Glacier/GameBase/ZCheckVisible.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Debug/ZDebugInt.h>
#include <Glacier/EventBase/ZEventBuffer.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Items/ZItem.h>
#include <Glacier/IK/ZCTRLIKLNKOBJ.h>
#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/GameBase/ZPlayer.h>
#include <Glacier/Physics/COLI.h>
#include <Glacier/Render/Debug/ZDrawDebugRender.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Data/ZGameData.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>


namespace Glacier
{
    static ZDebugInt g_lInvisible { "Invisible", "Force the player invisible", 0, 0, 1, 1, "Vision/" };
    static ZDebugInt g_iBlind { "Blind", "Force all seers blind", 0, 0, 1, 1, "Vision/" };
    static ZDebugInt g_lShowVisible { "ShowVisible", "Draw visibility tests", 0, 0, 2, 1, "Vision/" };
    static ZDebugInt g_iShowVisionPosCache { "ShowVisionPosCache", "Draw cached seeable positions", 0, 0, 1, 1, "Vision/" };
    ZDebugInt g_iDumpSeeables { "DumpSeeables", "Dump all seeables", 0, 0, 1, 1, "Vision/" };
    static ZDebugInt g_iDumpSeers { "DumpSeers", "Dump all seers", 0, 0, 1, 1, "Vision/" };
    ZDebugInt g_iShowVisionCells { "ShowVisionCells", "Draw visibility cells", 0, 0, 2, 1, "Vision/" };
    static ZDebugInt g_iShowSeerCones { "ShowSeerCones", "Draw seer cones", 0, 0, 2, 1, "Vision/" };
    ZDebugInt g_iShowCellsForSeer { "ShowCellsForSeer", "Draw cells for one seer", -1, -1, 512, 1, "Vision/" };
    ZCellBitmap g_DebugShowCells;

    ZCheckVisible* ZCheckVisible::m_pCheckVisible = nullptr;
    uint16_t ZCheckVisible::m_CallbackWantedMask = 0;

    ZCheckVisible::ZCheckVisible()
        : CBaseEvent<ZLIST>()
    {
        m_fItemMaxDistance = 2000.0f;
        m_fWallDecalMaxDistance = 1500.0f;
        m_fGroundDecalMaxDistance = 600.0f;
        m_iNumSeeables = 0;
        m_iHighestSeeableDecalID = 0xFFFF;
        m_iNumSeers = 0;
        m_bResumeSeer = 0;
        m_cCurrentCell = ZCellCoordinate();
        m_iCurrentIndex = 0;
        m_iCurrentSeer = 0;
        m_fVisionDistMultiplier = 1.0f;
        m_pCheckVisible = this;
        m_CallbackWantedMask = 0;
        memset(m_Seeables, 0, sizeof(m_Seeables));
        memset(m_Seers, 0, sizeof(m_Seers));
    }

    ZCheckVisible::~ZCheckVisible()
    {
        m_pCheckVisible = nullptr;
    }

    void ZCheckVisible::Release(bool bFree)
    {
        this->~ZCheckVisible();
        if (bFree)
        {
            ZEventBuffer::Instance().FreeEventRam(this);
        }
    }

    ZCheckVisible& ZCheckVisible::Instance()
    {
        ZASSERT(ZCheckVisible::m_pCheckVisible);
        return *ZCheckVisible::m_pCheckVisible;
    }

    void ZCheckVisible::NukeIt()
    {
        ZCheckVisible::m_pCheckVisible = nullptr;
    }

    void ZCheckVisible::PostSave(ISerializerStream& stream)
    {
        ZSerializable::PostSave(stream);
        for (uint32_t i = 0; i < m_iNumSeers; ++i)
        {
            m_Seers[i].LoadSave(stream, false);
        }
        for (uint32_t j = 0; j < m_iNumSeeables; ++j)
        {
            m_Seeables[j].LoadSave(stream, false);
        }
    }

    bool ZCheckVisible::PostLoad(ISerializerStream& stream)
    {
        const bool bResult = ZSerializable::PostLoad(stream);
        if (stream.TestStreamFilter(1u << ISerializerStream::CONTENT_SavedGame))
        {
            for (uint32_t i = 0; i < m_iNumSeers; ++i)
            {
                m_Seers[i].LoadSave(stream, true);
            }
            for (uint32_t j = 0; j < m_iNumSeeables; ++j)
            {
                m_Seeables[j].LoadSave(stream, true);
                if (m_Seeables[j].pSeeable)
                {
                    m_Seeables[j].locator = m_Space.AddSeeable(m_Seeables[j].pos, static_cast<uint8_t>(j));
                }
            }
        }
        return bResult;
    }

    const RTP::ZPropertyInfo& ZCheckVisible::GetProperties() const
    {
        return ZCheckVisible::Info;
    }

    void ZCheckVisible::Init()
    {
        ZASSERT(g_pGameData);
        g_pGameData->AddEventToCheckPoint(this);
        m_Space.Initialize();
    }

    void ZCheckVisible::Init2()
    {
        // Do nothing
    }

    void ZCheckVisible::CheckPointLoad(ZCheckPointBuffer&)
    {
        for (uint32_t i = 0; i < m_iNumSeers; ++i)
        {
            if (m_Seers[i].pSeer)
            {
                for (uint32_t j = 0; j < Vision::MAXSEEABLES; ++j)
                {
                    m_Seers[i].Objects[j].SetVisible(false);
                }
            }
        }
    }

    void ZCheckVisible::FrameUpdate()
    {
        if (!m_iNumSeers)
        {
            return;
        }
        if ((g_pSysInterface->m_lFrameCount % 10) == 0)
        {
            PruneSeeableDecals();
        }

        TIMETYPE now = g_pSysInterface->m_fRealTime;
        uint32_t seerBudget = 20;
        uint32_t rayBudget = 5;

        while (seerBudget-- && rayBudget)
        {
            if (m_iCurrentSeer >= m_iNumSeers)
            {
                m_iCurrentSeer = 0;
            }

            SSeer& seer = m_Seers[m_iCurrentSeer];
            const int32_t elapsed = seer.CalculateTimeElapsed(now);
            if (!seer.pSeer || seer.bDisabled || elapsed <= 0)
            {
                ++m_iCurrentSeer;
                continue;
            }

            ZVector3 seerPos;
            ZVector3 seerDir;
            GetSeerPosDir(seerPos, seerDir, seer.pSeer, seer.vOffset, (seer.lFlag & 1u) != 0);
            const ZCellCoordinate center = m_Space.FindCell(seerPos);
            if (!center.IsValid())
            {
                seer.ttLastTimeChecked = now;
                ++m_iCurrentSeer;
                continue;
            }

            const ZCellCoordinate radius = GetDistanceInCells(seer.fVisionRange);
            ZCellCoordinate low;
            ZCellCoordinate high;
            ExcludeCellsByDistance(center, radius, low, high);
            const float xAxis[3] = { 1.0f, 0.0f, 0.0f };
            const float zAxis[3] = { 0.0f, 0.0f, 1.0f };
            ExcludeCellsByAngleToAxis(seer, seerDir, const_cast<float*>(xAxis), const_cast<float*>(zAxis), center, low, high);

            ZCellBitmap uncheckedCells(seer.VisibleCells);
            if (g_iShowCellsForSeer >= 0 && static_cast<uint32_t>(g_iShowCellsForSeer) == m_iCurrentSeer)
            {
                g_DebugShowCells.Clear();
            }
            bool exhausted = false;
            for (int x = low.m_X; x <= high.m_X && !exhausted; ++x)
            {
                for (int y = low.m_Y; y <= high.m_Y && !exhausted; ++y)
                {
                    for (int z = low.m_Z; z <= high.m_Z && !exhausted; ++z)
                    {
                        ZCellCoordinate cell(static_cast<int8_t>(x), static_cast<int8_t>(y), static_cast<int8_t>(z));
                        if (m_bResumeSeer
                            && (cell.m_X < m_cCurrentCell.m_X
                                || (cell.m_X == m_cCurrentCell.m_X && cell.m_Y < m_cCurrentCell.m_Y)
                                || (cell.m_X == m_cCurrentCell.m_X && cell.m_Y == m_cCurrentCell.m_Y && cell.m_Z < m_cCurrentCell.m_Z)))
                        {
                            continue;
                        }
                        uncheckedCells.Clear(cell);
                        if (g_iShowCellsForSeer >= 0 && static_cast<uint32_t>(g_iShowCellsForSeer) == m_iCurrentSeer)
                        {
                            g_DebugShowCells.Set(cell);
                        }
                        ZSpaceCell& spaceCell = m_Space.GetCell(cell);
                        uint8_t first = 0;
                        if (m_bResumeSeer && cell == m_cCurrentCell)
                        {
                            first = m_iCurrentIndex;
                        }

                        for (uint8_t index = first; index < spaceCell.GetNumSeeables(); ++index)
                        {
                            const uint8_t id = spaceCell.GetSeeableID(index);
                            ZSeeable& seeable = m_Seeables[id];
                            SSeerObject& state = seer.Objects[id];
                            if (!(seeable.type & seer.lVisibleTypes) || (!seeable.IsDecal() && seeable.pSeeable == seer.pSeer))
                            {
                                continue;
                            }

                            int32_t delay = state.GetDeltaTime() - elapsed;
                            if ((seer.lFlag & 4u) == 0 && delay > 0)
                            {
                                state.SetDeltaTime(std::min(delay, 127));
                                continue;
                            }

                            ZVector3 direction;
                            vsub(direction, seeable.pos, seerPos);
                            const float distance = vnorm(direction);
                            if (distance > seer.fVisionRange * m_fVisionDistMultiplier)
                            {
                                UpdateVisible(m_iCurrentSeer, id, false, false);
                                continue;
                            }

                            if ((seeable.type == Vision::GROUNDDECAL && distance > m_fGroundDecalMaxDistance)
                                || (seeable.type == Vision::WALLDECAL && distance > m_fWallDecalMaxDistance)
                                || (seeable.type == Vision::ITEM && distance > m_fItemMaxDistance))
                            {
                                continue;
                            }

                            const bool sixthSense = seer.fSixthSenseRange > 0.00012207031f && distance <= seer.fSixthSenseRange;
                            if (!sixthSense && vdot(direction, seerDir) < seer.fVisionCone)
                            {
                                UpdateVisible(m_iCurrentSeer, id, false, false);
                                state.SetDeltaTime(0);
                                continue;
                            }

                            COLI coli;
                            coli.lp = seerPos;
                            vsub(coli.ln, seeable.pos, seerPos);
                            const bool blocked = ZROOT->ChkLineColi(&coli, eGlobalTreeType::GT_StdObjs, 16, -1, true, true);
                            if (blocked)
                            {
                                UpdateVisible(m_iCurrentSeer, id, false, false);
                                int32_t nextDelay = static_cast<int32_t>(std::min(distance * 0.0005f, 1.0f) * 60.0f);
                                if (seeable.type == Vision::PLAYER)
                                {
                                    nextDelay >>= 2;
                                }
                                state.SetDeltaTime(nextDelay);
                            }
                            else
                            {
                                seer.VisibleCells.Set(cell);
                                UpdateVisible(m_iCurrentSeer, id, true, false);
                                state.SetDeltaTime(seeable.IsDecal() ? 127 : 60);
                            }

                            if (!--rayBudget)
                            {
                                m_cCurrentCell = cell;
                                m_iCurrentIndex = index + 1;
                                m_bResumeSeer = 1;
                                exhausted = true;
                                break;
                            }
                        }
                    }
                }
            }

            seer.lFlag &= ~4u;
            seer.ttLastTimeChecked = now;
            NotifyObjectsNotInCheckedCells(seer, uncheckedCells);
            if (!exhausted)
            {
                seer.m_cLowCell = low;
                seer.m_cHighCell = high;
                m_bResumeSeer = 0;
                ++m_iCurrentSeer;
            }
        }
    }

    void ZCheckVisible::DrawDebugObjects(ZDrawDebugRender* pRender)
    {
        if (g_iShowVisionPosCache) m_Space.ShowSeeablePositions(pRender);
        if (g_iShowVisionCells) m_Space.ShowCells(pRender);
        if (g_iShowCellsForSeer >= 0) m_Space.ShowCellsForSeer(pRender);
        if (g_iDumpSeeables) m_Space.DumpSeeables();
        if (g_iDumpSeers) DumpSeers();
        if (g_iShowSeerCones) ShowCones(pRender);
        if (g_lShowVisible) ShowVision(pRender);
    }

    void ZCheckVisible::RemoveSeeable(const ZGEOM* pObject)
    {
        const SeeableID id = FindSeeableID(pObject);
        if (id != 0xFF)
        {
            RemoveSeeable(id);
        }
    }

    ZCheckVisible::SeeableID ZCheckVisible::GetFreeSeeableSlot()
    {
        for (uint8_t i = 0; ; ++i)
        {
            if (i >= Vision::MAXSEEABLES)
            {
                return 0xFF;
            }
            if (!m_Seeables[i].pSeeable)
            {
                if (i >= m_iNumSeeables)
                {
                    m_iNumSeeables = i + 1;
                }
                return i;
            }
        }
    }

    ZCheckVisible::SeeableID ZCheckVisible::AddSeeable(ZGEOM* pObject, float* pPos, uint16_t type)
    {
        ZASSERT(pObject);
        const SeeableID id = GetFreeSeeableSlot();
        if (id != 0xFF)
        {
            m_Seeables[id].pSeeable = pObject;
            m_Seeables[id].type = type;
            m_Seeables[id].locator = m_Space.AddSeeable(pPos, id);
            vcpy(m_Seeables[id].pos, pPos);
        }
        return id;
    }

    ZCheckVisible::SeeableID ZCheckVisible::FindSeeableID(const ZGEOM* pObject) const
    {
        ZASSERT(pObject);
        for (uint32_t i = 0; i < m_iNumSeeables; ++i)
        {
            if (!m_Seeables[i].IsDecal() && m_Seeables[i].pSeeable == pObject)
            {
                return static_cast<uint8_t>(i);
            }
        }
        return 0xFF;
    }

    ZSeeable& ZCheckVisible::GetSeeable(uint8_t seeableID)
    {
        return m_Seeables[seeableID];
    }

    void ZCheckVisible::UpdateSeeable(uint8_t seeableID, float* pPos)
    {
        ZSeeable& seeable = m_Seeables[seeableID];
        const Locator locator = seeable.locator;
        seeable.locator = m_Space.UpdateSeeable(seeableID, locator, pPos);
        vcpy(seeable.pos, pPos);
    }

    void ZCheckVisible::RemoveSeeable(uint8_t seeableID)
    {
        SendNotVisibleToSpectators(seeableID);
        const Locator locator = m_Seeables[seeableID].locator;
        m_Space.RemoveSeeable(locator);
        if (seeableID == static_cast<uint8_t>(m_iNumSeeables - 1))
        {
            --m_iNumSeeables;
        }
        m_Seeables[seeableID].pSeeable = nullptr;
    }

    void ZCheckVisible::AddSeeableItem(ZItem* pItem)
    {
        ZVector3 vPos;
        pItem->GetCen(vPos);
        pItem->GetRootPoint(vPos);
        const SeeableID id = AddSeeable(pItem, vPos, Vision::ITEM);
        if (id != 0xFF)
        {
            pItem->SetVisionID(id);
        }
    }

    void ZCheckVisible::UpdateSeeableItem(ZItem* pItem)
    {
        const uint8_t id = pItem->GetVisionID();
        if (id == 0xFF)
        {
            AddSeeableItem(pItem);
            return;
        }
        ZVector3 vPos;
        pItem->GetCen(vPos);
        pItem->GetRootPoint(vPos);
        UpdateSeeable(id, vPos);
    }

    void ZCheckVisible::RemoveSeeableItem(ZItem* pItem)
    {
        const uint8_t id = pItem->GetVisionID();
        if (id != 0xFF)
        {
            RemoveSeeable(id);
            pItem->SetVisionID(0xFF);
        }
    }

    void ZCheckVisible::AddSeeableActor(ZLNKWHANDS* pActor, uint16_t type)
    {
        if (pActor->GetVisionID() == 0xFF)
        {
            ZVector3 vPos;
            pActor->GetVisionPos(vPos);
            const SeeableID id = AddSeeable(pActor, vPos, type);
            pActor->SetVisionID(id);
        }
    }

    void ZCheckVisible::UpdateSeeableActor(ZLNKWHANDS* pActor)
    {
        ZASSERT(pActor);
        const uint8_t id = pActor->GetVisionID();
        if (id == 0xFF)
        {
            AddSeeableActor(pActor, Vision::BODY);
            return;
        }
        ZVector3 vPos;
        pActor->GetVisionPos(vPos);
        UpdateSeeable(id, vPos);
    }

    void ZCheckVisible::RemoveSeeableActor(ZLNKWHANDS* pActor)
    {
        const uint8_t id = pActor->GetVisionID();
        if (id != 0xFF)
        {
            if (!(m_Seeables[id].locator == Locator::Invalid))
            {
                RemoveSeeable(id);
                pActor->SetVisionID(0xFF);
            }
        }
    }

    void ZCheckVisible::AddSeeablePlayer(ZLNKWHANDS* pPlayer)
    {
        AddSeeableActor(pPlayer, Vision::PLAYER);
    }

    void ZCheckVisible::UpdateSeeablePlayer(ZLNKWHANDS* pPlayer)
    {
        UpdateSeeableActor(pPlayer);
    }

    uint32_t ZCheckVisible::AddSeer(const ZGEOM* pSeer, uint32_t lVisibleTypes)
    {
        int seerIndex = 0;
        if (GetSeer(&seerIndex, pSeer))
        {
            return static_cast<uint32_t>(seerIndex);
        }
        if (m_iNumSeers >= Vision::MAXSEERS)
        {
            printf("Too many seers on the level. Talk to briana.\n");
            return 0;
        }
        SSeer& seer = m_Seers[m_iNumSeers];
        memset(&seer, 0, sizeof(seer));
        seer.pSeer = const_cast<ZGEOM*>(pSeer);
        seer.fVisionCone = std::cos(1.0471976f); // cos(DTOR(60.0f)) = cos(60 degrees)
        seer.lVisibleTypes = lVisibleTypes;
        seer.lFlag = 0;
        seer.fVerticalVisionCone = 0.0f;
        seer.fVisionRange = 3000.0f;
        seer.fSixthSenseRange = 0.0f;
        seer.fLightMultiplier = 1.0f;
        seer.ttLastTimeChecked = g_pSysInterface->m_fRealTime;
        seer.fLastTimeFraction = 0.0f;
        seer.vOffset = ZVector3(0.0f, 0.0f, 0.0f);
        if (pSeer->IsDerivedFrom<ZCTRLIKLNKOBJ>()) seer.lFlag |= 1u;
        if (pSeer->IsDerivedFrom<ZPlayer>()) seer.lFlag |= 4u;
        return m_iNumSeers++;
    }

    void ZCheckVisible::RemoveSeer(const ZGEOM* pSeer)
    {
        int seerIndex = 0;
        if (GetSeer(&seerIndex, pSeer))
        {
            if (seerIndex != static_cast<int>(m_iNumSeers - 1))
            {
                memmove(&m_Seers[seerIndex], &m_Seers[seerIndex + 1], (m_iNumSeers - static_cast<uint32_t>(seerIndex) - 1) * sizeof(SSeer));
            }
            --m_iNumSeers;
        }
    }

    uint32_t ZCheckVisible::GetSeerIndex(ZGEOM* pSeer) const
    {
        int index = 0;
        return GetSeer(&index, pSeer) ? static_cast<uint32_t>(index) : UINT32_MAX;
    }

    bool ZCheckVisible::GetSeer(int* pSeerIndex, const ZGEOM* pSeer) const
    {
        for (uint32_t i = 0; i < m_iNumSeers; ++i)
        {
            if (m_Seers[i].pSeer == pSeer)
            {
                *pSeerIndex = static_cast<int>(i);
                return true;
            }
        }
        return false;
    }

    void ZCheckVisible::GetSeerPosDir(float* pPos, float* pDir, ZGEOM* pSeer, const float* pDefaultPos, bool bUseDefaultPos)
    {
        if (bUseDefaultPos)
        {
            static_cast<ZCTRLIKLNKOBJ*>(pSeer)->GetSeerPosDir(
                *reinterpret_cast<ZVector3*>(pPos),
                *reinterpret_cast<ZVector3*>(pDir));
            return;
        }

        vcpy(pPos, pDefaultPos);
        vset(pDir, 0.0f, 0.0f, -1.0f);
        pSeer->GetRootPoint(*reinterpret_cast<ZVector3*>(pPos));
        pSeer->GetRootVect(*reinterpret_cast<ZVector3*>(pDir));
    }

    void ZCheckVisible::SetVisionRange(ZGEOM* pSeer, float fRange)
    {
        int seerIndex = 0;
        if (GetSeer(&seerIndex, pSeer))
        {
            m_Seers[seerIndex].fVisionRange = clamp(fRange, 10.0f, 10000.0f);
        }
        else
        {
            printf("Warning: OnSetVisRange() %s is not a registered seer\n", pSeer->Name());
        }
    }

    void ZCheckVisible::SetFOV(ZGEOM* pSeer, float fFov)
    {
        int seerIndex = 0;
        if (GetSeer(&seerIndex, pSeer))
        {
            m_Seers[seerIndex].fVisionCone = std::cos(clamp(fFov, 0.0f, 360.0f) * 0.0087266462f);
        }
        else
        {
            printf("Warning: OnSetVisFov() %s is not a registered seer\n", pSeer->Name());
        }
    }

    void ZCheckVisible::SetVerticalFOV(ZGEOM* pSeer, float fFov)
    {
        int seerIndex = 0;
        if (GetSeer(&seerIndex, pSeer))
        {
            m_Seers[seerIndex].fVerticalVisionCone = std::cos(clamp(fFov, 0.0f, 360.0f) * 0.0087266462f);
        }
        else
        {
            printf("Warning: OnSetVisVFov() %s is not a registered seer\n", pSeer->Name());
        }
    }

    void ZCheckVisible::SetSeerBlind(ZGEOM* pSeer, bool bBlind)
    {
        int seerIndex = 0;
        if (GetSeer(&seerIndex, pSeer))
        {
            m_Seers[seerIndex].bDisabled = bBlind;
            if (bBlind)
            {
                for (uint8_t i = 0; i < m_iNumSeeables; ++i)
                {
                    if (m_Seeables[i].pSeeable)
                    {
                        UpdateVisible(static_cast<uint32_t>(seerIndex), i, false, false);
                    }
                }
            }
        }
        else
        {
            printf("Warning: SetSeerBlind() %s is not a registered seer\n", pSeer->Name());
        }
    }

    void ZCheckVisible::SetVisionOffset(ZGEOM* pSeer, float* pOffset)
    {
        int index = 0;
        if (GetSeer(&index, pSeer))
        {
            vcpy(m_Seers[index].vOffset, pOffset);
        }
    }

    void ZCheckVisible::SetSixthSenseRange(ZGEOM* pSeer, float fRange)
    {
        int index = 0;
        if (GetSeer(&index, pSeer))
        {
            m_Seers[index].fSixthSenseRange = fRange;
        }
    }

    void ZCheckVisible::SetLightMultiplier(ZGEOM* pSeer, float fMultiplier)
    {
        int index = 0;
        if (GetSeer(&index, pSeer))
        {
            m_Seers[index].fLightMultiplier = fMultiplier;
        }
    }

    void ZCheckVisible::SetVisionDistanceMultiplier(float fMultiplier)
    {
        m_fVisionDistMultiplier = fMultiplier;
    }

    void ZCheckVisible::SetCallbackWantedMask(uint16_t mask)
    {
        m_CallbackWantedMask = mask;
    }

    void ZCheckVisible::ResetTimers()
    {
        for (uint32_t i = 0; i < m_iNumSeers; ++i)
        {
            m_Seers[i].ttLastTimeChecked = g_pSysInterface->m_fRealTime;
            m_Seers[i].fLastTimeFraction = 0.0f;
        }
    }

    uint32_t ZCheckVisible::SetVisibleTypes(ZGEOM* pSeer, uint32_t lVisibleTypes)
    {
        int seerIndex = 0;
        uint32_t oldTypes = 0;
        if (GetSeer(&seerIndex, pSeer))
        {
            oldTypes = m_Seers[seerIndex].lVisibleTypes;
            m_Seers[seerIndex].lVisibleTypes = lVisibleTypes;
        }
        else
        {
            printf("Warning: SetVisibleTypes() %s is not a registered seer\n", pSeer->Name());
        }
        return oldTypes;
    }

    uint32_t ZCheckVisible::GetVisibleTypes(ZGEOM* pSeer)
    {
        int seerIndex = 0;
        if (GetSeer(&seerIndex, pSeer))
        {
            return m_Seers[seerIndex].lVisibleTypes;
        }
        return 0;
    }

    bool ZCheckVisible::IsObjectVisibleToSeer(const ZGEOM* pSeer, ZGEOM* pObject)
    {
        int seerIndex = 0;
        if (!GetSeer(&seerIndex, pSeer))
        {
            return false;
        }
        const uint8_t seeableID = FindSeeableID(pObject);
        return m_Seers[seerIndex].Objects[seeableID].GetVisible();
    }

    uint32_t ZCheckVisible::GetVisibleSeeables(ZLNKWHANDS* pActor, ZGEOM** pSeeables, uint32_t maxCount, uint16_t types)
    {
        if (maxCount == 0)
        {
            return 0;
        }
        int seerIndex = -1;
        GetSeer(&seerIndex, pActor);
        if (seerIndex < 0)
        {
            return 0;
        }
        SSeer& seer = m_Seers[seerIndex];
        uint32_t count = 0;
        for (uint32_t i = 0; i < m_iNumSeeables; ++i)
        {
            ZSeeable& seeable = m_Seeables[i];
            if (!seeable.IsDecal() && (seeable.type & types) != 0)
            {
                if (seer.Objects[i].GetVisible())
                {
                    pSeeables[count++] = seeable.pSeeable;
                    if (count >= maxCount)
                    {
                        break;
                    }
                }
            }
        }
        if (count >= maxCount)
        {
            printf("ZCheckVisible::GetCurrentSpectators() returned a full array - you might not be getting all the spectators. Try increasing the size of the Spectators array if this is not intentional.\n");
        }
        return count;
    }

    uint32_t ZCheckVisible::GetCurrentSpectators(ZGEOM* pObject, ZGEOM** pSpectators, uint32_t maxCount)
    {
        uint32_t count = 0;
        const uint8_t seeableID = FindSeeableID(pObject);
        if (maxCount != 0)
        {
            for (uint32_t i = 0; i < m_iNumSeers; ++i)
            {
                if (m_Seers[i].Objects[seeableID].GetVisible())
                {
                    pSpectators[count++] = m_Seers[i].pSeer;
                    if (count >= maxCount)
                    {
                        break;
                    }
                }
            }
            if (count >= maxCount)
            {
                printf("ZCheckVisible::GetCurrentSpectators() returned a full array - you might not be getting all the spectators. Try increasing the size of the Spectators array if this is not intentional.\n");
            }
        }
        return count;
    }

    void ZCheckVisible::NotifySeersOfCellChange(uint8_t seeableID, const ZCellCoordinate& coord)
    {
        for (uint32_t i = 0; i < m_iNumSeers; ++i)
        {
            if (m_Seers[i].Objects[seeableID].GetVisible())
            {
                printf("Seer is notified of Seeable %d's warp jump\n", seeableID);
                if (!coord.IsWithin(m_Seers[i].m_cLowCell, m_Seers[i].m_cHighCell))
                {
                    UpdateVisible(i, seeableID, false, false);
                }
            }
        }
    }

    void ZCheckVisible::SendNotVisibleToSpectators(uint8_t seeableID)
    {
        for (uint32_t i = 0; i < m_iNumSeers; ++i)
        {
            if (m_Seers[i].Objects[seeableID].GetVisible())
            {
                UpdateVisible(i, seeableID, false, false);
            }
        }
    }

    void ZCheckVisible::UpdateVisible(uint32_t lSeerNr, uint8_t seeableID, bool bIsVisible, bool bForce)
    {
        ZASSERT(lSeerNr < Vision::MAXSEERS);
        ZASSERT(seeableID < Vision::MAXSEEABLES);

        SSeer& seer = m_Seers[lSeerNr];
        SSeerObject& state = seer.Objects[seeableID];
        ZSeeable& seeable = m_Seeables[seeableID];
        if ((g_lInvisible && seeable.type == Vision::PLAYER) || g_iBlind)
        {
            bIsVisible = false;
        }

        if (seeable.IsDecal())
        {
            state.SetVisible(bIsVisible);
            return;
        }

        ZASSERT(seeable.pSeeable);
        ZREF data[3] = { seeable.pSeeable->GetRef(), 0, 0 };
        if (bIsVisible)
        {
            if ((!state.GetVisible() && bIsVisible) || bForce)
            {
                seer.pSeer->SendCommand(2056, data, GetGeom());
            }
        }
        else if (state.GetVisible())
        {
            seer.pSeer->SendCommand(2057, data, GetGeom());
        }
        state.SetVisible(bIsVisible);
    }

    void ZCheckVisible::NotifyObjectsNotInCheckedCells(SSeer& seer, ZCellBitmap& visibleCells)
    {
        const ZCellCoordinate low = seer.m_cLowCell.Shift(-1, -1, -1);
        const ZCellCoordinate high = seer.m_cHighCell.Shift(1, 1, 1);
        for (int x = low.m_X; x <= high.m_X; ++x)
        {
            for (int y = low.m_Y; y <= high.m_Y; ++y)
            {
                for (int z = low.m_Z; z <= high.m_Z; ++z)
                {
                    ZCellCoordinate cell(static_cast<int8_t>(x), static_cast<int8_t>(y), static_cast<int8_t>(z));
                    const bool boundary = x == low.m_X || x == high.m_X || y == low.m_Y || y == high.m_Y || z == low.m_Z || z == high.m_Z;
                    if (!visibleCells.IsSet(cell) && !(boundary && !cell.IsWithin(seer.m_cLowCell, seer.m_cHighCell)))
                    {
                        continue;
                    }

                    ZSpaceCell& spaceCell = m_Space.GetCell(cell);
                    for (uint8_t i = 0; i < spaceCell.GetNumSeeables(); ++i)
                    {
                        UpdateVisible(m_iCurrentSeer, spaceCell.GetSeeableID(i), false, false);
                    }
                }
            }
        }
    }

    ZCellCoordinate ZCheckVisible::GetDistanceInCells(float fDist) const
    {
        ZCellCoordinate coord;
        coord.m_X = static_cast<int8_t>(fDist / m_Space.m_vCellSize.x + 0.5f);
        coord.m_Y = static_cast<int8_t>(fDist / m_Space.m_vCellSize.y + 0.5f);
        coord.m_Z = static_cast<int8_t>(fDist / m_Space.m_vCellSize.z + 0.5f);
        if (coord.m_X < 1)
            coord.m_X = 1;
        if (coord.m_Y < 1)
            coord.m_Y = 1;
        if (coord.m_Z < 1)
            coord.m_Z = 1;
        return coord;
    }

    bool ZCheckVisible::IsCellInVisionCone(ZCellCoordinate& coord, float* pPos, float* pDir, float fCosCone, float fCosVerticalCone)
    {
        return true;
    }

    void ZCheckVisible::ExcludeCellsByDistance(const ZCellCoordinate& lowCell, const ZCellCoordinate& highCell, ZCellCoordinate& newLowCell, ZCellCoordinate& newHighCell)
    {
        newHighCell.m_X = static_cast<int8_t>(std::min<int>(lowCell.m_X + highCell.m_X, ZCellCoordinate::XSEGMENTS - 1));
        newHighCell.m_Y = static_cast<int8_t>(std::min<int>(lowCell.m_Y + highCell.m_Y, ZCellCoordinate::YSEGMENTS - 1));
        newHighCell.m_Z = static_cast<int8_t>(std::min<int>(lowCell.m_Z + highCell.m_Z, ZCellCoordinate::ZSEGMENTS - 1));
        newLowCell.m_X = static_cast<int8_t>(std::max<int>(lowCell.m_X - highCell.m_X, 0));
        newLowCell.m_Y = static_cast<int8_t>(std::max<int>(lowCell.m_Y - highCell.m_Y, 0));
        newLowCell.m_Z = static_cast<int8_t>(std::max<int>(lowCell.m_Z - highCell.m_Z, 0));
    }

    void ZCheckVisible::ExcludeCellsByAngleToAxis(SSeer& seer, float* pPos, float* pAxis, float* pRight, const ZCellCoordinate& cCone, ZCellCoordinate& lowCell, ZCellCoordinate& highCell)
    {
        if (vdot(pPos, pAxis) < seer.fVisionCone)
        {
            if (pPos[2] <= 0.0f) highCell.m_Z = cCone.m_Z;
            else lowCell.m_Z = cCone.m_Z;
        }
        if (vdot(pPos, pRight) < seer.fVisionCone)
        {
            if (pPos[0] <= 0.0f) highCell.m_X = cCone.m_X;
            else lowCell.m_X = cCone.m_X;
        }
    }

    void ZCheckVisible::AddSeeableDecalIfNecessary(const COLI* pColi, float fLifeTime, uint8_t bDecalType, float fDist)
    {
        ZASSERT(pColi);
        if (fDist > -1.0f && DecalWithinDistance(pColi, bDecalType, fDist))
        {
            return;
        }

        const SeeableID id = GetFreeSeeableSlot();
        if (id == 0xFF)
        {
            return;
        }
        if (static_cast<int16_t>(m_iHighestSeeableDecalID) < id)
        {
            m_iHighestSeeableDecalID = id;
        }

        ZVector3 edge1;
        ZVector3 edge2;
        ZVector3 normal;
        vsub(edge1, pColi->vP2, pColi->vP1);
        vsub(edge2, pColi->vP3, pColi->vP1);
        vcross(normal, edge2, edge1);
        vnorm(normal);
        ZGEOM::RefToPtr(pColi->ColiRef)->GetRootVect(normal);

        ZSeeable& seeable = m_Seeables[id];
        seeable.pos = pColi->cp;
        seeable.pos.x += normal.x * 10.0f;
        seeable.pos.y += normal.y * 10.0f;
        seeable.pos.z += normal.z * 10.0f;
        seeable.locator = m_Space.AddSeeable(seeable.pos, id);
        seeable.wTime = static_cast<int16_t>(fLifeTime * 60.0f);
        seeable.wDecalType = bDecalType;
        seeable.type = std::fabs(normal.y) >= 0.30000001f ? Vision::GROUNDDECAL : Vision::WALLDECAL;
    }

    bool ZCheckVisible::DecalWithinDistance(const COLI* pColi, uint8_t bDecalType, float fDist) const
    {
        const ZCellCoordinate center = m_Space.FindCell(pColi->cp);
        const ZCellCoordinate radius = GetDistanceInCells(fDist);
        ZCellCoordinate low;
        ZCellCoordinate high;
        const_cast<ZCheckVisible*>(this)->ExcludeCellsByDistance(center, radius, low, high);
        for (int x = low.m_X; x <= high.m_X; ++x)
        {
            for (int y = low.m_Y; y <= high.m_Y; ++y)
            {
                for (int z = low.m_Z; z <= high.m_Z; ++z)
                {
                    ZCellCoordinate cell(static_cast<int8_t>(x), static_cast<int8_t>(y), static_cast<int8_t>(z));
                    if (const_cast<ZCheckVisible*>(this)->CellHasDecalWithinDistance(cell, bDecalType, pColi->cp, fDist))
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool ZCheckVisible::CellHasDecalWithinDistance(ZCellCoordinate& coord, uint8_t bDecalType, const float* pPos, float fDist)
    {
        ZSpaceCell& cell = m_Space.GetCell(coord);
        for (uint8_t i = 0; i < cell.GetNumSeeables(); ++i)
        {
            ZSeeable& seeable = m_Seeables[cell.GetSeeableID(i)];
            if (seeable.IsDecal() && seeable.wDecalType == bDecalType && vdist(pPos, seeable.pos) < fDist)
            {
                return true;
            }
        }
        return false;
    }

    void ZCheckVisible::RemoveAllDecals()
    {
        const int16_t highest = static_cast<int16_t>(m_iHighestSeeableDecalID);
        for (int16_t i = 0; i <= highest; ++i)
        {
            if (m_Seeables[i].Exists() && m_Seeables[i].IsDecal())
            {
                RemoveSeeable(static_cast<uint8_t>(i));
            }
        }
        m_iHighestSeeableDecalID = 0;
    }

    void ZCheckVisible::PruneSeeableDecals()
    {
        const float elapsed = g_pSysInterface->DeltaFrameTime * 60.0f;
        uint16_t frames = static_cast<uint16_t>(elapsed);
        m_fDeltaTimeFraction += elapsed - frames;
        if (m_fDeltaTimeFraction > 1.0f)
        {
            ++frames;
            m_fDeltaTimeFraction -= 1.0f;
        }

        int16_t newHighest = 0;
        const int16_t oldHighest = static_cast<int16_t>(m_iHighestSeeableDecalID);
        for (int16_t i = 0; i <= oldHighest; ++i)
        {
            ZSeeable& seeable = m_Seeables[i];
            if (seeable.Exists() && seeable.IsDecal())
            {
                seeable.wTime -= frames;
                if (seeable.wTime >= 0) newHighest = i;
                else RemoveSeeable(static_cast<uint8_t>(i));
            }
        }
        m_iHighestSeeableDecalID = static_cast<uint16_t>(newHighest);
    }

    void ZCheckVisible::DumpSeers()
    {
        g_iDumpSeers.m_iValue = 0;
        for (uint32_t i = 0; i < m_iNumSeers; ++i)
        {
            m_Seers[i].Dump();
        }
    }

    void ZCheckVisible::ShowCones(ZDrawDebugRender* pRender)
    {
        for (uint32_t i = 0; i < m_iNumSeers; ++i)
        {
            ShowConeForSeer(pRender, m_Seers[i], i);
        }
    }

    void ZCheckVisible::ShowVision(ZDrawDebugRender* pRender)
    {
        for (uint32_t seerIndex = 0; seerIndex < m_iNumSeers; ++seerIndex)
        {
            SSeer& seer = m_Seers[seerIndex];
            ZVector3 seerPos;
            ZVector3 seerDir;
            GetSeerPosDir(seerPos, seerDir, seer.pSeer, seer.vOffset, (seer.lFlag & 1u) != 0);
            for (uint32_t id = 0; id < m_iNumSeeables; ++id)
            {
                ZSeeable& seeable = m_Seeables[id];
                if (!seeable.Exists() || !(seeable.type & seer.lVisibleTypes)) continue;
                if (g_lShowVisible >= 2 || vdist(seerPos, seeable.pos) <= seer.fVisionRange)
                {
                    printf("seer %u -> seeable %u: %s\n", seerIndex, id, seer.Objects[id].GetVisible() ? "visible" : "hidden");
                }
            }
        }
    }

    void ZCheckVisible::ShowConeForSeer(ZDrawDebugRender* pRender, SSeer& seer, uint32_t lSeerNr)
    {
        ZVector3 pos;
        ZVector3 dir;
        GetSeerPosDir(pos, dir, seer.pSeer, seer.vOffset, (seer.lFlag & 1u) != 0);
        const ZCellCoordinate cell = m_Space.FindCell(pos);
        char text[128];
        std::snprintf(text, sizeof(text), "id: %u (%d, %d, %d) range: %.2f cone: %.2f vcone %.2f",
            lSeerNr, cell.m_X, cell.m_Y, cell.m_Z, seer.fVisionRange,
            2.0f * std::acos(seer.fVisionCone) * 57.2957795f,
            2.0f * std::acos(seer.fVerticalVisionCone) * 57.2957795f);
        ZVector3 textPos = pos;
        textPos.y += 20.0f;
        printf("%s\n", text);
    }

#   pragma region "RTTI"
    namespace cProperties
    {
        static RTP::ZDataProperty<float> NamespaceItem_3659 {
            .m_Node {
                .m_Next = nullptr,
                .m_Name = "m_fDeltaTimeFraction",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__11,
            .m_Offset = CLASS_PROPERTY(ZCheckVisible, m_fDeltaTimeFraction)
        };

        static RTP::ZDataProperty<uint16_t> NamespaceItem_3658 {
            .m_Node {
                .m_Next = &NamespaceItem_3659.m_Node,
                .m_Name = "m_iHighestSeeableDecalID",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__136,
            .m_Offset = CLASS_PROPERTY(ZCheckVisible, m_iHighestSeeableDecalID)
        };

        static RTP::ZDataProperty<uint32_t> NamespaceItem_3657 {
            .m_Node {
                .m_Next = &NamespaceItem_3658.m_Node,
                .m_Name = "m_iNumSeeables",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__6,
            .m_Offset = CLASS_PROPERTY(ZCheckVisible, m_iNumSeeables)
        };

        static RTP::ZDataProperty<uint32_t> NamespaceItem_3656 {
            .m_Node {
                .m_Next = &NamespaceItem_3657.m_Node,
                .m_Name = "m_iNumSeers",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__6,
            .m_Offset = CLASS_PROPERTY(ZCheckVisible, m_iNumSeers)
        };

        static RTP::ZDataProperty<float> NamespaceItem_3655 {
            .m_Node {
                .m_Next = &NamespaceItem_3656.m_Node,
                .m_Name = "m_fVisionDistMultiplier",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__11,
            .m_Offset = CLASS_PROPERTY(ZCheckVisible, m_fVisionDistMultiplier)
        };
    }

    DEFINE_ROUT_CLASS(ZCheckVisible, ZLIST, CheckVisible, 48, 0, 0x815978, cProperties::NamespaceItem_3655, ZEventBase);
#   pragma endregion
}
