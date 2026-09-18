#include <Glacier/Render/ZWaterManager.h>
#include <Glacier/ZUniMemory.h>
#include <cstdio>


namespace Glacier
{
    ZWaterManager::ZWaterManager()
    {
        InitManager(256);
    }

    ZWaterManager::ZWaterManager(int iTotalPatches)
    {
        InitManager(iTotalPatches);
    }

    void ZWaterManager::InitManager(int iTotalPatches)
    {
        m_iNumPatches = 0;
        m_iNumRipples = 0;

        if (iTotalPatches <= 256)
        {
            m_iTotalPatches = iTotalPatches;
        }
        else
        {
            m_iTotalPatches = 256;
        }

        m_pPatchLinearMem = static_cast<SWaterPatchInst*>(ZUniMemory::Allocate(sizeof(SWaterPatchInst) * m_iTotalPatches));
        m_pPatches = static_cast<SWaterPatchInst**>(ZUniMemory::Allocate(sizeof(SWaterPatchInst*) * m_iTotalPatches));

        for (int i = 0; i < m_iTotalPatches; ++i)
        {
            m_pPatches[i] = &m_pPatchLinearMem[i];
        }
    }

    SWaterPatchInst* ZWaterManager::AllocWaterPatch(void* pKey)
    {
        if (m_iNumPatches < m_iTotalPatches)
        {
            SWaterPatchInst* pPatch = m_pPatches[m_iNumPatches];
            pPatch->pKey = pKey;
            pPatch->iRipples = 0;
            ++m_iNumPatches;
            return pPatch;
        }

        printf("WATERMGR: Total patch capacity exceeded!!! Talk to Mircea\n");
        return nullptr;
    }

    void ZWaterManager::FreeWaterPatch(void* pKey)
    {
        if (m_iNumPatches <= 0)
        {
            return;
        }

        int iIndex = 0;
        for (SWaterPatchInst** i = m_pPatches; pKey != (*i)->pKey; ++i)
        {
            if (++iIndex >= m_iNumPatches)
            {
                return;
            }
        }

        const int iLast = m_iNumPatches - 1;
        m_iNumPatches = iLast;

        if (iLast != 0 && iIndex != iLast)
        {
            SWaterPatchInst* pLast = m_pPatches[iLast];
            SWaterPatchInst* pFound = m_pPatches[iIndex];
            m_pPatches[iIndex] = pLast;
            m_pPatches[m_iNumPatches] = pFound;
        }
    }

    int ZWaterManager::UpdateRippleInfo(SWaterPatchInst* p)
    {
        const float fXPos = p->fXPos;
        const float fZPos = p->fZPos;
        const float fXMax = p->fSegSizeX * p->iXsegs + fXPos;
        const float fZMax = p->fSegSizeZ * p->iZsegs + fZPos;

        p->iRipples = 0;

        for (int i = 0; i < m_iNumRipples; ++i)
        {
            if (p->iRipples >= 8)
            {
                break;
            }

            SWaterRippleInst* pRipple = &m_pRipples[i];
            if (pRipple->fRadius + pRipple->fXCenter >= fXPos
                && pRipple->fXCenter - pRipple->fRadius <= fXMax
                && pRipple->fRadius + pRipple->fZCenter >= fZPos
                && pRipple->fZCenter - pRipple->fRadius <= fZMax)
            {
                p->ripples[p->iRipples++] = pRipple;
            }
        }

        return p->iRipples;
    }

    void ZWaterManager::FrameUpdate(float fTime)
    {
        for (int i = 0; i < m_iNumRipples; ++i)
        {
            SWaterRippleInst* pRipple = &m_pRipples[i];

            pRipple->fAmplitude -= pRipple->fDamping;
            if (pRipple->fAmplitude <= 0.0f)
            {
                const int iLast = m_iNumRipples - 1;
                m_iNumRipples = iLast;

                if (iLast)
                {
                    m_pRipples[i] = m_pRipples[iLast];
                    --i;
                }
            }
            else
            {
                pRipple->fRadius += pRipple->fRadiusSpeed;
                pRipple->fOneOverRadius = 1.0f / pRipple->fRadius;
                pRipple->fPhase = fTime * pRipple->fSpeed * 6.2831855f;
            }
        }

        for (int i = 0; i < m_iNumPatches; ++i)
        {
            UpdateRippleInfo(m_pPatches[i]);
        }
    }

    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZWaterManager*, g_pWaterManager, 0x008EC128, nullptr);
}
