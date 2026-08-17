#include <Glacier/Render/Decal/ZDecalMarkControllerBase.h>
#include <Glacier/Render/ZBoneModifyBase.h>
#include <Glacier/Render/Prim/ZPrimAccess.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Geom/GeomControlMasks.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <cstring>


namespace Glacier
{
    // ZDecalMarkControllerBase
    ZDecalMarkControllerBase::ZDecalMarkControllerBase()
    {
        m_pBonesFirst = nullptr;
        m_pBonesLast = nullptr;
        memset(m_pBoneDecalsLookup, 0, sizeof(m_pBoneDecalsLookup));
    }

    ZDecalMarkControllerBase::ZBonesDecal* ZDecalMarkControllerBase::GetAvailBoneDecal()
    {
        ZBonesDecal* pDecal = m_pBonesLast;
        const uint8_t lDecalLookup = pDecal->m_pLnkObj->GetBoneModifier()->DecalLookup();

        // Unlink the decal from its lookup chain
        ZBonesDecal* pPrev = nullptr;
        ZBonesDecal* pEntry = m_pBoneDecalsLookup[lDecalLookup];
        while (pEntry && pEntry != pDecal)
        {
            pPrev = pEntry;
            pEntry = pEntry->m_pNextSameLookup;
        }
        if (pEntry)
        {
            if (pPrev)
            {
                pPrev->m_pNextSameLookup = pEntry->m_pNextSameLookup;
            }
            else
            {
                m_pBoneDecalsLookup[lDecalLookup] = pEntry->m_pNextSameLookup;
            }
        }

        // Drop ZCRENDERATTACHED once no decals of this lnkobj remain in the chain
        bool bHasDecals = false;
        for (const ZBonesDecal* pIt = m_pBoneDecalsLookup[lDecalLookup]; pIt; pIt = pIt->m_pNextSameLookup)
        {
            if (pIt->m_pLnkObj == pDecal->m_pLnkObj)
            {
                bHasDecals = true;
                break;
            }
        }
        if (!bHasDecals)
        {
            pDecal->m_pLnkObj->SetControl(0, ZCRENDERATTACHED);
        }

        // Unlink the decal from the global list
        if (pDecal == m_pBonesLast)
        {
            m_pBonesLast = pDecal->m_pPrev;
        }
        if (pDecal == m_pBonesFirst)
        {
            m_pBonesFirst = pDecal->m_pNext;
        }
        if (pDecal->m_pPrev)
        {
            pDecal->m_pPrev->m_pNext = pDecal->m_pNext;
        }
        if (pDecal->m_pNext)
        {
            pDecal->m_pNext->m_pPrev = pDecal->m_pPrev;
        }

        for (int i = 0; i < 4; ++i)
        {
            if (pDecal->m_pPrimAccess[i])
            {
                pDecal->m_pPrimAccess[i]->Destroy();
            }
        }

        pDecal->m_pLnkObj->BaseGeom()->SetAttachUpdate();

        memset(pDecal, 0xD5, sizeof(ZDecalMarkControllerBase::ZBonesDecal));
        return pDecal;
    }

    ZDecalMarkControllerBase::ZBonesDecal* ZDecalMarkControllerBase::GetBoneDecals(ZLNKOBJ* pLnkObj)
    {
        return m_pBoneDecalsLookup[pLnkObj->GetBoneModifier()->DecalLookup()];
    }

    void ZDecalMarkControllerBase::RemoveBoneDecalBaseGeom(ZLNKOBJ* pLnkObj)
    {
        const uint8_t lDecalLookup = pLnkObj->GetBoneModifier()->DecalLookup();

        ZBonesDecal* pEntry = m_pBoneDecalsLookup[lDecalLookup];
        while (pEntry)
        {
            ZBonesDecal* pDecal = pEntry;
            const bool bForeign = pDecal->m_pLnkObj != pLnkObj;
            pEntry = pDecal->m_pNextSameLookup;

            if (!bForeign)
            {
                if (pDecal == m_pBoneDecalsLookup[lDecalLookup])
                {
                    m_pBoneDecalsLookup[lDecalLookup] = pEntry;
                }

                for (int i = 0; i < 4; ++i)
                {
                    if (pDecal->m_pPrimAccess[i])
                    {
                        pDecal->m_pPrimAccess[i]->Destroy();
                    }
                }

                pDecal->m_pLnkObj->BaseGeom()->SetAttachUpdate();

                m_BoneDecals.Remove(pDecal);
            }
        }

        pLnkObj->SetControl(0, ZCRENDERATTACHED);
    }

    void ZDecalMarkControllerBase::RemoveBoneDecal(ZLNKOBJ* pLnkObj, ZDecalMarkControllerBase::ZBonesDecal* pBonesDecal)
    {
        if (!pBonesDecal)
        {
            return;
        }

        ZBonesDecal* pNextSameLookup = pBonesDecal->m_pNextSameLookup;
        if (pBonesDecal->m_pLnkObj != pLnkObj)
        {
            return;
        }

        const uint8_t lDecalLookup = pLnkObj->GetBoneModifier()->DecalLookup();
        if (pBonesDecal == m_pBoneDecalsLookup[lDecalLookup])
        {
            m_pBoneDecalsLookup[lDecalLookup] = pNextSameLookup;
        }
        else
        {
            ZBonesDecal** ppLink = &m_pBoneDecalsLookup[lDecalLookup]->m_pNextSameLookup;
            while (*ppLink != pBonesDecal)
            {
                ppLink = &(*ppLink)->m_pNextSameLookup;
            }
            *ppLink = pBonesDecal->m_pNextSameLookup;
        }

        for (int i = 0; i < 4; ++i)
        {
            if (pBonesDecal->m_pPrimAccess[i])
            {
                pBonesDecal->m_pPrimAccess[i]->Destroy();
            }
        }

        pBonesDecal->m_pLnkObj->BaseGeom()->SetAttachUpdate();

        m_BoneDecals.Remove(pBonesDecal);

        if (!pNextSameLookup)
        {
            pLnkObj->SetControl(0, ZCRENDERATTACHED);
        }
    }

    void ZDecalMarkControllerBase::AddBoneDecal(ZLNKOBJ* pLnkObj, const float* pvPosition, const float* pvDirection, float fRadius, uint32_t lBoneId, uint32_t lSourcePrim)
    {
        ZBoneModifyBase* pBoneModify = pLnkObj->GetBoneModifier();
        if (!pBoneModify)
        {
            return;
        }

        // TODO: Finish this place after ZPrimControlBase will be reversed
        // const uint8_t lDecalLookup = pBoneModify->DecalLookup();
        //
        // ZMatrix rootTM;
        // pLnkObj->GetRootTM(rootTM.m0, rootTM.p0);
        //
        // const ZBone* pBones = pLnkObj->GetBones();
        // const uint8_t* pBoneIdLookup = g_pRenderDll->m_pPrimControl->GetBoneIdToIndexLookup(pLnkObj->Prim());
        // const ZBone* pBone = &pBones[pBoneIdLookup[lBoneId]];
        //
        // // Bring the mark into bone-local space and back through the global prim bones
        // ZVector3 vPosition;
        // MatrixTransformInverse(vPosition.Get(), pvPosition, rootTM);
        // vsub(vPosition.Get(), pBone->_Pos.Get());
        // vmtmul(vPosition.Get(), pBone->_Mat.Get());
        //
        // ZVector3 vDirection;
        // vmtmul(vDirection.Get(), pvDirection, rootTM.m0.Get());
        // vmtmul(vDirection.Get(), pBone->_Mat.Get());
        //
        // const ZBone* pGlobalBone = &pLnkObj->GetGlobalPrimBones()[pBoneIdLookup[lBoneId]];
        // vmmul(vPosition.Get(), pGlobalBone->_Mat.Get());
        // vadd(vPosition.Get(), pGlobalBone->_Pos.Get());
        // vmmul(vDirection.Get(), pGlobalBone->_Mat.Get());
        //
        // ZBonesDecal* pDecal;
        // if (m_BoneDecals.Count() == m_BoneDecals.TotalNrEntries())
        // {
        //     pDecal = GetAvailBoneDecal();
        // }
        // else
        // {
        //     pDecal = m_BoneDecals.Add();
        // }
        //
        // pDecal->m_vPosition = vPosition;
        // vnorm(pDecal->m_vDirection.Get(), vDirection.Get());
        // pDecal->m_fRadius = fRadius;
        // pDecal->m_fRandomAngle = g_pSysInterface->FRand(__FILE__, __LINE__) * 6.2831855f;
        // pDecal->m_lBoneId = lBoneId;
        // pDecal->m_lSourcePrim = lSourcePrim;
        // pDecal->m_pLnkObj = pLnkObj;
        // pDecal->m_pPrimAccess[0] = nullptr;
        // pDecal->m_pPrimAccess[1] = nullptr;
        // pDecal->m_pPrimAccess[2] = nullptr;
        // pDecal->m_pPrimAccess[3] = nullptr;
        //
        // pDecal->m_pNextSameLookup = m_pBoneDecalsLookup[lDecalLookup];
        // pDecal->m_pNext = m_pBonesFirst;
        // if (m_pBonesFirst)
        // {
        //     m_pBonesFirst->m_pPrev = pDecal;
        // }
        // m_pBonesFirst = pDecal;
        // pDecal->m_pPrev = nullptr;
        // if (!m_pBonesLast)
        // {
        //     m_pBonesLast = pDecal;
        // }
        // m_pBoneDecalsLookup[lDecalLookup] = pDecal;
        //
        pLnkObj->SetControl(ZCRENDERATTACHED, 0);
        pLnkObj->BaseGeom()->SetAttachUpdate();
    }
}
