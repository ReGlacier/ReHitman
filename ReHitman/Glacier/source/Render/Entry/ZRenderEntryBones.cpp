#include <Glacier/Render/Entry/ZRenderEntryBones.h>
#include <Glacier/Render/Entry/SRenderEntryNotifyInfo.h>
#include <Glacier/Animation/Model.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/IK/ZIKLNKOBJ.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/Prim/SPropertyBones.h>
#include <Glacier/Render/Prim/SPrimObjectHeader.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>



namespace Glacier
{
    ZRenderEntryBones::ZRenderEntryBones(const ZRenderEntryGeomCreateInfo& sInfo)
        : ZRenderEntryGeom(sInfo)
    {
        m_pBoneData = nullptr;
        m_pLightData = nullptr;
        m_fMinDistanceToObservers = 9.9999997e37f;
        m_lNumAllocatedBones = 0;
        m_LodLEvelsAvailable = 0;
        m_lVariantId = 1;

        m_lGeomListsControl |= 2u;
        m_lControl |= RE_HASBONES;

        if (!m_pBaseGeom)
            return;

        auto* pGeom = m_pBaseGeom->GetGeom();
        const bool bLinkObject = pGeom
            ? pGeom->Is<ZLNKOBJ>()
            : m_pBaseGeom->IsDerivedFromStdObj(ZLNKOBJ::m_Id);
        ZASSERT(bLinkObject);
        if (!bLinkObject)
            return;

        auto* pLinkObject = static_cast<ZLNKOBJ*>(pGeom);
        auto* pPrimControl = g_pRenderDll->m_pPrimControl;
        m_lNumAllocatedBones = pPrimControl->GetNrBones(m_pBaseGeom->m_lPrim);

        const auto* pHeader = ZPrimHandle{ m_pBaseGeom->m_lPrim }.Get<SPrimObjectHeader>();
        const auto* pProperty = pHeader
            ? static_cast<const SPropertyBones*>(pPrimControl->GetPrimData(pHeader->lPropertyData))
            : nullptr;
        const auto* pBones = pProperty
            ? static_cast<const SPropertyBones*>(pPrimControl->GetPrimData(pProperty->lBoneDefinitions))
            : nullptr;
        if (!pBones || !m_lNumAllocatedBones)
        {
            m_lNumAllocatedBones = 0;
            m_lVariantId = pLinkObject->m_lVariantId;
            return;
        }

        const bool bStateModel = pBones->stateModel != 0;
        const int lDynamicSize = pLinkObject->m_Model->DynamicSize(
            pLinkObject,
            pBones->lBonePoseHeader,
            pBones->lBoneIdToIndexLookup,
            pBones->lBoneIndexToIdLookup2,
            pBones->lBoneIdToPosLookup,
            pBones->lParentLookup,
            bStateModel,
            static_cast<int>(m_lNumAllocatedBones));
        const uint32_t lSize = static_cast<uint32_t>(
            lDynamicSize + 96 * m_lNumAllocatedBones + 144
            + (bStateModel ? 48 * m_lNumAllocatedBones : 0));
        m_pBoneData = static_cast<char*>(ZUniMemory::Allocate(lSize, RENDERCPU_MEM));
        if (!m_pBoneData)
        {
            m_lControl |= RE_CONSTRUCTION_FAILED;
            m_lNumAllocatedBones = 0;
        }
        else
        {
            std::memset(m_pBoneData, 0, lSize);
            pLinkObject->m_pBoneModify->m_lNumActiveBones =
                static_cast<uint16_t>(m_lNumAllocatedBones);
            pLinkObject->m_Model->Init(
                pLinkObject,
                reinterpret_cast<ZBone*>(m_pBoneData + 144
                    + 48 * 3 * m_lNumAllocatedBones),
                pBones->lBonePoseHeader,
                pBones->lBoneIdToIndexLookup,
                pBones->lBoneIndexToIdLookup2,
                pBones->lBoneIdToPosLookup,
                pBones->lParentLookup,
                bStateModel,
                m_pBoneData,
                static_cast<int>(m_lNumAllocatedBones));
        }

        m_lVariantId = pLinkObject->m_lVariantId;
        if (pLinkObject->Is<ZIKLNKOBJ>())
            m_lControl |= RE_WANT_VIEW_NOTIFY;
    }

    ZRenderEntryBones::~ZRenderEntryBones()
    {
        if (m_pBoneData)
        {
            if (m_pBaseGeom)
            {
                auto* pGeom = m_pBaseGeom->GetGeom();
                if (pGeom && pGeom->Is<ZLNKOBJ>())
                {
                    auto* pLinkObject = static_cast<ZLNKOBJ*>(pGeom);
                    pLinkObject->m_Model->m_Bones = nullptr;
                    pLinkObject->m_Model->m_PoseWeights = nullptr;
                    pLinkObject->m_Model->m_StateBlending = nullptr;
                    pLinkObject->m_Model->m_BlendBones = nullptr;
                    pLinkObject->m_Model->m_State = nullptr;
                }
            }
            ZUniMemory::Free(m_pBoneData);
            m_pBoneData = nullptr;
        }
    }

    void ZRenderEntryBones::Notify(const SRenderEntryNotifyInfo* pEntry)
    {
        const float fDistance = pEntry->fDistance * pEntry->fLODScale * 100.0f;
        if (fDistance < m_fMinDistanceToObservers)
            m_fMinDistanceToObservers = fDistance;
        ZRenderEntryGeom::Notify(pEntry);
    }

    void ZRenderEntryBones::AttachUpdate()
    {
        // TODO: Finish me after the PC ZPrimAccess attachment mesh path is reversed.
        return;
    }

    uint32_t ZRenderEntryBones::GetInstanceRepeat(const ZRenderObjectInstance* pObjInstance)
    {
        (void)pObjInstance;
        return m_pDrawArray ? *reinterpret_cast<const uint32_t*>(m_pDrawArray) : 1u;
    }

    void ZRenderEntryBones::SetRenderContext(ZRenderContext* pContext, const ZRenderObjectInstance* pObjInstance)
    {
        (void)pObjInstance;
        if (m_pDrawArray)
        {
            const auto* pPass = reinterpret_cast<const char*>(m_pDrawArray) +
                *reinterpret_cast<const uint32_t*>(reinterpret_cast<const char*>(m_pDrawArray) + 16) +
                56 * pContext->m_nCurrentPass;
            std::memcpy(&pContext->m_ObjectToWorldMatrix, pPass, sizeof(ZMatrix));
            pContext->m_pDeformBones = *reinterpret_cast<const float* const*>(pPass + 48);
            pContext->m_pBonesLight = *reinterpret_cast<const float* const*>(pPass + 52);
        }
        else
        {
            pContext->m_ObjectToWorldMatrix = m_ObjectToWorldMatrix;
            pContext->m_pDeformBones = reinterpret_cast<const float*>(m_pBoneData + 144);
            pContext->m_pBonesLight = m_pLightData ? m_pLightData : reinterpret_cast<float*>(m_pBoneData + 32);
        }

        const float fFade = (m_lFade == 0xFF ? 2.0f : static_cast<float>(m_lFade) * 0.0039370079f)
            * 0.85000002f + 0.15000001f;
        pContext->m_fObjectFade = fFade > 1.0f ? 1.0f : fFade;
    }

    void ZRenderEntryBones::EndFrame()
    {
        m_fMinDistanceToObservers = 9.9999997e37f;
        ZRenderEntry::EndFrame();
    }

    void ZRenderEntryBones::CalcLODMask(SRenderEntryNotifyInfo* pEntry)
    {
        pEntry->lDrawDestinationOverride = 0;
        if (m_lUnknown98)
        {
            // TODO: Finish me after the PC draw-array destination lookup is reversed.
            return;
        }
        if (m_pDrawArray)
        {
            pEntry->fDistance = 0.0f;
            pEntry->lLODMask = 0xFF;
            return;
        }

        const float fDistance = vdist(pEntry->vObserver, m_ObjectToWorldMatrix.p0);
        pEntry->fDistance = fDistance;
        int lLODIndex = static_cast<int>(fDistance * pEntry->fLODScale);
        if (lLODIndex < 0)
            lLODIndex = 0;
        else if (lLODIndex > 255)
            lLODIndex = 255;

        const auto* pRender = g_pSysInterface->WindowFirst;
        pEntry->lLODMask = 1u << pRender->m_aLODLookupTable[lLODIndex];

        if (pEntry->lLODMask <= 1 && (m_lControl & RE_ISBACKDROP) != 0
            && (m_lDrawDestinationOverride & 0x10u) == 0)
        {
            // TODO: Finish me after ZBoneModifyBase::CheckPointInside is reversed.
        }
    }

    void ZRenderEntryBones::GetAttachedBaseGeoms(ZStackArray<1024, ZRenderEntry::ZAttachedBaseGeom>* pArray)
    {
        ZASSERT(pArray != nullptr);
        ZASSERT(m_pBaseGeom != nullptr);
        auto* pGeom = m_pBaseGeom->GetGeom();
        if (!pGeom || !pGeom->Is<ZLNKOBJ>())
            return;

        auto* pLinkObject = static_cast<ZLNKOBJ*>(pGeom);
        if (!pLinkObject->m_pBoneModify)
            return;

        for (uint32_t i = 0; i < pLinkObject->m_pBoneModify->m_AttachedGeoms.Count(); ++i)
        {
            const auto* pAttached = pLinkObject->m_pBoneModify->m_AttachedGeoms.Get(i);
            auto* pBaseGeom = ZBaseGeom::RefToPtr(pAttached->m_rBaseGeom);
            if (!pBaseGeom)
                continue;

            ZRenderEntry::ZAttachedBaseGeom attached{ pBaseGeom, this };
            pArray->Push(attached);
        }

        const auto& attachedGroups = pLinkObject->m_pBoneModify->m_ConnectedPhysics;
        for (uint32_t i = 0; i < attachedGroups.Count(); ++i)
        {
            auto* pAttachedGeom = ZGEOM::RefToPtr(*attachedGroups.Get(i));
            if (!pAttachedGeom || (pAttachedGeom->GetObjectId() & ZGEOM::m_Mask) != ZGEOM::m_Id)
                continue;

            if (pAttachedGeom->m_baseGeom)
            {
                ZRenderEntry::ZAttachedBaseGeom attached{ pAttachedGeom->m_baseGeom, this };
                pArray->Push(attached);
            }
        }
    }

    ZRenderEntryBones* ZRenderEntryBones::Create(const ZRenderEntryGeomCreateInfo& sInfo)
    {
        return ZUniMemory::New<ZRenderEntryBones>(sInfo);
    }
}
