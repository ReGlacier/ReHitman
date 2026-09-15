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
#include <Glacier/Render/Prim/SPrimObject.h>
#include <Glacier/Render/Prim/ZPrimAccess.h>
#include <Glacier/Render/Prim/ZPrimAccessMesh.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Render/Prim/ZPrimAccessMeshWeighted.h>
#include <Glacier/Render/Prim/SPrimMeshWeighted.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Render/Draw/IDraw.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>



namespace Glacier
{
    const ZBone* ZRenderEntryBones::GetBones() const
    {
        return m_lNumAllocatedBones
            ? reinterpret_cast<const ZBone*>(m_pBoneData + 144 + 48 * m_lNumAllocatedBones)
            : nullptr;
    }

    float* ZRenderEntryBones::GetBonesLightData() const
    {
        return m_pLightData ? m_pLightData : reinterpret_cast<float*>(m_pBoneData + 32);
    }

    void ZRenderEntryBones::SetBonesLightData(float* pLightData)
    {
        m_pLightData = pLightData;
    }

    void ZRenderEntryBones::CreateDefaultBones(uint32_t lFirstBoneNum)
    {
        auto* pBaseGeom = m_pBaseGeom;
        auto* pGeom = pBaseGeom ? pBaseGeom->GetGeom() : nullptr;
        if (!pGeom || !pGeom->Is<ZLNKOBJ>() || !m_pBoneData || !m_lNumAllocatedBones)
            return;

        auto* pLinkObject = static_cast<ZLNKOBJ*>(pGeom);
        auto* pBones = reinterpret_cast<ZBone*>(m_pBoneData + 144 + 48 * m_lNumAllocatedBones);
        pLinkObject->GetDefaultBones(pBones, lFirstBoneNum);
    }

    void ZRenderEntryBones::UpdateActiveNumBones()
    {
        if (!m_lLODLevelsWanted || !m_pBaseGeom)
            return;

        auto* pGeom = m_pBaseGeom->GetGeom();
        if (!pGeom || !pGeom->Is<ZLNKOBJ>())
            return;

        auto* pLinkObject = static_cast<ZLNKOBJ*>(pGeom);
        if (!pLinkObject->m_pBoneModify)
            return;

        auto* pPrimControl = g_pRenderDll && g_pRenderDll->m_pPrimControl
            ? g_pRenderDll->m_pPrimControl
            : nullptr;
        if (!pPrimControl)
            return;

        const auto* pHeader = static_cast<const SPrimObjectHeader*>(
            pPrimControl->GetPrimData(m_pBaseGeom->m_lPrim));
        const auto* pProperty = pHeader
            ? static_cast<const SPropertyBones*>(pPrimControl->GetPrimData(pHeader->lPropertyData))
            : nullptr;
        const auto* pBones = pProperty
            ? static_cast<const SPropertyBones*>(pPrimControl->GetPrimData(pProperty->lBoneDefinitions))
            : nullptr;
        if (!pBones)
            return;

        uint32_t lNumActiveBones = 0;
        uint8_t lLODLevels = m_lLODLevelsWanted;
        for (uint32_t i = 0; i < 8 && lLODLevels; ++i, lLODLevels <<= 1)
        {
            if ((lLODLevels & 0x80u) != 0)
                lNumActiveBones = std::max<uint32_t>(lNumActiveBones, pBones->lNumBonesUsedLOD[i]);
        }

        lNumActiveBones = std::min(lNumActiveBones, m_lNumAllocatedBones);
        const uint32_t lPreviousActiveBones = pLinkObject->m_pBoneModify->m_lNumActiveBones;
        if (lNumActiveBones == lPreviousActiveBones)
            return;

        pLinkObject->m_pBoneModify->m_lNumActiveBones = static_cast<uint16_t>(lNumActiveBones);
        if (lNumActiveBones > lPreviousActiveBones)
            CreateDefaultBones(lPreviousActiveBones);
    }

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
            CreateDefaultBones(0);
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
        auto* pBaseGeom = m_pBaseGeom;
        auto* pGeom = pBaseGeom ? pBaseGeom->GetGeom() : nullptr;
        if (!pGeom || !pGeom->Is<ZLNKOBJ>())
            return;

        auto* pLnkObj = static_cast<ZLNKOBJ*>(pGeom);
        auto* pRenderDraw = IDraw::Instance<ZRenderDraw>();
        if (!pRenderDraw)
            return;
        SRenderEntryInstance* aInstances[512];
        uint32_t lNumInstances = 0;

        // Keep the ordinary sub-object instances and discard old bone decals.
        for (uint32_t i = 0; i < m_lNumRenderEntryInstances; ++i)
        {
            auto* pInstance = m_pRenderEntryInstances[i];
            if ((pInstance->lTransparencyMask & 1u) != 0)
            {
                m_pRenderEntryInstances[i] = nullptr;
                pRenderDraw->DestroyRenderEntryInstance(pInstance);
            }
            else
            {
                ZASSERT(lNumInstances < 512);
                aInstances[lNumInstances++] = pInstance;
            }
        }

        auto* pDecal = pRenderDraw->m_DecalMarks.GetBoneDecals(pLnkObj);
        for (; pDecal; pDecal = pDecal->m_pNextSameLookup)
        {
            const auto* pObjectHeader = ZPrimHandle{ m_lPrimId }.Get<SPrimObjectHeader>();
            if (!pObjectHeader || pObjectHeader->lType != 7)
                break;

            const auto* pObjectTable = ZPrimHandle{ pObjectHeader->lObjectTable }.Get<uint32_t>();
            if (!pObjectTable)
                break;

            const float fRadius = pDecal->m_fRadius < 7.5f ? 7.5f : pDecal->m_fRadius;
            ZVector3 vMin = pDecal->m_vPosition;
            ZVector3 vMax = pDecal->m_vPosition;
            vMin.x -= fRadius;
            vMin.y -= fRadius;
            vMin.z -= fRadius;
            vMax.x += fRadius;
            vMax.y += fRadius;
            vMax.z += fRadius;

            ZMat3x3 mDirection;
            createmat(mDirection.Get(), pDecal->m_vDirection.Get(), nullptr);
            ZMat3x3 mRotation;
            mreset(mRotation.Get());
            mrotaxis(mRotation.Get(), pDecal->m_fRandomAngle, 0.0f, 0.0f, 1.0f);
            ZMat3x3 mDecal;
            tmat(mDecal.Get(), mDirection.Get());
            mmmul(mDecal.Get(), mRotation.Get());

            for (uint32_t variation = 0; variation < pObjectHeader->lNumObjects && variation < 4; ++variation)
            {
                ZPrimHandle hSource{ pObjectTable[variation] };
                const auto* pSourceObject = hSource.Get<SPrimObject>();
                if (!pSourceObject || (pSourceObject->lDrawMode & 0x800000u) != 0
                    || (m_lVariantId != 0 && pSourceObject->lVariantId != m_lVariantId)
                    || pSourceObject->lSubType > SPrimObject::SUBTYPE_WEIGHTED)
                {
                    continue;
                }

                auto* pSourceAccess = ZPrimAccess::Create(hSource);
                auto* pSourceMesh = dynamic_cast<ZPrimAccessMeshWeighted*>(pSourceAccess);
                if (!pSourceMesh)
                {
                    if (pSourceAccess)
                        pSourceAccess->Destroy();
                    continue;
                }

                pSourceMesh->Lock(ZPrimAccess::LF_READONLY);
                uint32_t aSelectedIndices[3 * 512] {};
                const uint32_t lTriangles = pSourceMesh->GetTrianglesInBox(
                    aSelectedIndices, 512, vMin.Get(), vMax.Get());

                uint16_t aOutputIndices[3 * 512] {};
                uint32_t lOutputTriangles = 0;
                for (uint32_t triangle = 0; triangle < lTriangles; ++triangle)
                {
                    const uint16_t i0 = static_cast<uint16_t>(aSelectedIndices[3 * triangle]);
                    uint16_t i1 = static_cast<uint16_t>(aSelectedIndices[3 * triangle + 1]);
                    uint16_t i2 = static_cast<uint16_t>(aSelectedIndices[3 * triangle + 2]);
                    float aPositions[9] {};
                    pSourceMesh->GetPositions(i0, 1, &aPositions[0]);
                    pSourceMesh->GetPositions(i1, 1, &aPositions[3]);
                    pSourceMesh->GetPositions(i2, 1, &aPositions[6]);
                    ZVector3 v0{ aPositions[0], aPositions[1], aPositions[2] };
                    ZVector3 v1{ aPositions[3], aPositions[4], aPositions[5] };
                    ZVector3 v2{ aPositions[6], aPositions[7], aPositions[8] };
                    ZVector3 edge1 = v1 - v0;
                    ZVector3 edge2 = v2 - v0;
                    ZVector3 normal;
                    vcross(normal, edge1, edge2);
                    if (normal.x * pDecal->m_vDirection.x +
                        normal.y * pDecal->m_vDirection.y +
                        normal.z * pDecal->m_vDirection.z >= 0.0f)
                    {
                        continue;
                    }
                    const uint32_t outputVertex = lOutputTriangles * 3;
                    aOutputIndices[outputVertex] = i0;
                    aOutputIndices[outputVertex + 1] = i1;
                    aOutputIndices[outputVertex + 2] = i2;
                    ++lOutputTriangles;
                }

                if (lOutputTriangles)
                {
                    const ZPrimHandle hTarget{ g_pRenderDll->m_pPrimControl->GetSubPrim(
                        pDecal->m_lSourcePrim, 0) };
                    const auto* pTargetHeader = hTarget.Get<SPrimHeader>();
                    if (pTargetHeader && pTargetHeader->lType == 3)
                    {
                        auto* pTargetAccess = ZPrimAccess::Create(hTarget);
                        auto* pTargetMesh = pTargetAccess
                            ? dynamic_cast<ZPrimAccessMeshWeighted*>(pTargetAccess)
                            : nullptr;
                        if (pTargetMesh)
                        {
                            auto* pMesh = dynamic_cast<ZPrimAccessMeshWeighted*>(
                                pTargetMesh->CreateEditable(lOutputTriangles, lOutputTriangles * 3));
                            if (pMesh)
                            {
                                auto* pHeader = const_cast<SPrimHeader*>(pTargetHeader);
                                pHeader->lPackType = pSourceObject->lLODMask;
                                pHeader->lDrawDestination = pSourceObject->lDrawDestination;
                                auto* pWeighted = reinterpret_cast<SPrimMeshWeighted*>(pHeader);
                                pWeighted->lNumCopyBones = *reinterpret_cast<const uint32_t*>(
                                    reinterpret_cast<const char*>(pSourceObject) + 0x38);
                                pWeighted->lCopyBones = *reinterpret_cast<const uint32_t*>(
                                    reinterpret_cast<const char*>(pSourceObject) + 0x3C);
                                pMesh->Lock(ZPrimAccess::LF_WRITEONLY);
                                const float fUVScale = 0.5f / (pDecal->m_fRadius < 7.5f ? 7.5f : pDecal->m_fRadius);
                                for (uint32_t triangle = 0; triangle < lOutputTriangles; ++triangle)
                                {
                                    const uint32_t outputVertex = triangle * 3;
                                    for (uint32_t vertex = 0; vertex < 3; ++vertex)
                                    {
                                        const uint16_t sourceVertex = aOutputIndices[outputVertex + vertex];
                                        pMesh->CloneVertex(outputVertex + vertex, pSourceMesh, sourceVertex);
                                        float positionData[3] {};
                                        pMesh->GetPositions(outputVertex + vertex, 1, positionData);
                                        ZVector3 position{ positionData[0], positionData[1], positionData[2] };
                                        position -= pDecal->m_vPosition;
                                        TransformRootVector(position, mDecal);
                                        vscalar(position.Get(), fUVScale);
                                        const float texCoords[2] = { position.x + 0.5f, position.y + 0.5f };
                                        pMesh->SetTexCoords(outputVertex + vertex, 1, texCoords);
                                    }
                                    const uint16_t indices[3] = {
                                        static_cast<uint16_t>(outputVertex),
                                        static_cast<uint16_t>(outputVertex + 1),
                                        static_cast<uint16_t>(outputVertex + 2) };
                                    pMesh->SetTriangles(triangle, 1, indices);
                                }
                                pMesh->Unlock();
                                pDecal->m_pPrimAccess[variation] = pMesh;
                            }
                        }
                        if (pTargetAccess)
                            pTargetAccess->Destroy();
                    }
                }
                pSourceMesh->Unlock();
                pSourceMesh->Destroy();
            }
        }

        for (auto* pDecal = pRenderDraw->m_DecalMarks.GetBoneDecals(pLnkObj); pDecal; pDecal = pDecal->m_pNextSameLookup)
        {
            for (uint32_t variation = 0; variation < 4; ++variation)
            {
                auto* pAccess = static_cast<ZPrimAccessMesh*>(pDecal->m_pPrimAccess[variation]);
                if (!pAccess)
                    continue;
                ZPrimHandle hPrim = pAccess->m_hPrim;
                auto* pInstance = pRenderDraw->CreateRenderEntryInstance(hPrim, this, m_pBaseGeom, true);
                if (pInstance)
                {
                    const auto* pPrim = hPrim.Get<SPrimObject>();
                    InitRenderEntryInstance(pInstance, pPrim->lLODMask, pPrim->lDrawDestination, 1u, 0u);
                    pInstance->lSortValue &= 0xFFFEEFFBu;
                    ZASSERT(lNumInstances < 512);
                    aInstances[lNumInstances++] = pInstance;
                }
                pAccess->Destroy();
                pDecal->m_pPrimAccess[variation] = nullptr;
            }
        }

        if (m_pRenderEntryInstances && m_pRenderEntryInstances != m_RenderEntryInstanceTable)
            ZUniMemory::Free(m_pRenderEntryInstances);
        m_pRenderEntryInstances = nullptr;
        m_lNumRenderEntryInstances = 0;
        if (lNumInstances)
            AddRenderEntryInstances(aInstances, lNumInstances);
        m_lControl &= ~RE_ATTACH_UPDATE;
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
        if (m_lUnknown98)
        {
            auto* pDraw = IDraw::Instance();
            const auto lViewCount = *reinterpret_cast<const uint16_t*>(
                reinterpret_cast<const char*>(pDraw) + 0x58);
            auto* pRenderEntry = lViewCount
                ? reinterpret_cast<ZRenderEntry**>(reinterpret_cast<char*>(pDraw) + 0x7F70)[lViewCount]
                : nullptr;
            if (pRenderEntry)
            {
                pRenderEntry->CalcLODMask(pEntry);
                pEntry->lDrawDestinationOverride = pRenderEntry->m_lDrawDestinationOverride;
                return;
            }
        }

        pEntry->lDrawDestinationOverride = 0;
        if (m_pDrawArray)
        {
            pEntry->fDistance = 0.0f;
            pEntry->lLODMask = 0xFF;
            return;
        }

        const float fDistance = vdist(pEntry->vObserver, m_ObjectToWorldMatrix.p0);
        pEntry->fDistance = fDistance;
        int lLODIndex = static_cast<int>(fDistance * pEntry->fLODScale);
        if (*reinterpret_cast<const uint8_t*>(
                reinterpret_cast<const char*>(m_pBaseGeom->GetGeom()) + 136) != 0)
            lLODIndex = 0;
        else if (lLODIndex < 0)
            lLODIndex = 0;
        else if (lLODIndex > 255)
            lLODIndex = 255;

        const auto* pRender = g_pSysInterface->WindowFirst;
        pEntry->lLODMask = 1u << pRender->m_aLODLookupTable[lLODIndex];

        if (pEntry->lLODMask <= 1 && (m_lControl & RE_ISBACKDROP) != 0
            && (m_lDrawDestinationOverride & 0x10u) == 0)
        {
            auto* pLinkObject = static_cast<ZLNKOBJ*>(m_pBaseGeom->GetGeom());
            const auto* pBoneDefinitions = pLinkObject->GetBoneDefinitions();
            ZVector3 vPoint = pEntry->vObserver - m_ObjectToWorldMatrix.p0;
            vmtmul(vPoint, m_ObjectToWorldMatrix.m0);
            const auto* pBones = reinterpret_cast<const ZBone*>(
                m_pBoneData + 144 + 144 * m_lNumAllocatedBones);
            if (pLinkObject->m_pBoneModify->CheckPointInside(
                    vPoint, *reinterpret_cast<const ZVector3*>(pBones), pBoneDefinitions))
                pEntry->lLODMask = 0;
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
