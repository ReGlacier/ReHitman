#include <Glacier/Render/Entry/ZRenderEntryGeom.h>
#include <Glacier/Render/Entry/SRenderEntryNotifyInfo.h>
#include <Glacier/Render/Entry/ZRenderEntryLists.h>
#include <Glacier/Render/Object/ZRenderObjectInstance.h>
#include <Glacier/Render/Object/ZRenderObject.h>
#include <Glacier/Render/Prim/SPrimObject.h>
#include <Glacier/Render/Prim/SPrimObjectHeader.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <Glacier/Render/Prim/EPrimType.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/Decal/ZDecalMarkController.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZSHADOWMESHOBJ.h>
#include <cstring>


namespace Glacier
{
    static constexpr uint32_t ZDECALMARK_MAX_NUM_DECALS = 0x200;

    ZRenderEntryGeom::ZRenderEntryGeom(const ZRenderEntryGeomCreateInfo& sInfo)
        : ZRenderEntry()
    {
        m_unk14 = 0;
        m_lLODLevelsActive = 0;
        m_lLODLevelsWanted = 0;
        m_lVariantId = 0;
        m_lEntryListsMask = 0;
        m_unk2C = 0;
        m_lDrawDestinationOverride = 0;
        m_lGeomListsControl = 0;
        m_lFade = 0xFF;
        m_lControl = 0;
        m_lNumRenderEntryInstances = 0;
        m_pRenderEntryInstances = nullptr;
        m_pEnvironment = nullptr;
        m_pDrawArray = nullptr;
        memset(&m_ObjectToWorldMatrix, 0, sizeof(m_ObjectToWorldMatrix));

        m_lUnknown80 = 0xFFFFFFFFu;
        m_fVertexFrameNumber = 0.0f;
        m_lRenderFlags = 0;
        m_pBaseGeom = nullptr;
        m_lPrimId = 0;
        m_lUnknown94 = 0;
        m_lUnknown98 = 0;

        if (sInfo.m_pBaseGeom)
        {
            m_pBaseGeom = sInfo.m_pBaseGeom;

            if ((m_pBaseGeom->m_lControl & ZCRENDERATTACHED) != 0)
            {
                m_lControl |= RE_ATTACH_UPDATE;
            }

            m_lPrimId = m_pBaseGeom->m_lPrim;

            if (m_pBaseGeom->GetGeom()->WantDrawBufferControl())
            {
                m_lGeomListsControl |= (1u << ZRenderEntryLists::LISTTYPES::LT_DRAWUPDATE);
            }

            const ZGEOM* pGeom = m_pBaseGeom->GetGeom();
            const bool bShadowMeshObj = pGeom
                ? pGeom->IsDerivedFrom<ZSHADOWMESHOBJ>()
                : m_pBaseGeom->IsDerivedFromStdObj(ZSHADOWMESHOBJ::m_Id);

            if (bShadowMeshObj)
            {
                m_lGeomListsControl |= (1u << ZRenderEntryLists::LISTTYPES::LT_STATICSHADOW);
            }

            m_pBaseGeom->m_lControl |= ZCINVIEW;

            if (m_pBaseGeom->WantCameraMsg())
            {
                m_pBaseGeom->GetGeom()->SendCommand(0x806u, nullptr, nullptr);
            }

            if (m_pBaseGeom->GetGeom()->WantViewPrimHideUnhideRequest())
            {
                m_lControl |= RE_WANT_VIEW_NOTIFY;
            }

            const uint8_t lLODMaskOverride = m_pBaseGeom->GetGeom()->GetLODMaskOverride();
            if (lLODMaskOverride)
            {
                m_lUnknown80 = lLODMaskOverride;
            }

            if (m_pBaseGeom->GetGeom()->GetSortPriority() == 9)
            {
                m_lDrawDestinationOverride = 16;
            }

            if (m_lPrimId)
            {
                const SPrimObjectHeader* pHeader = ZPrimHandle{ m_lPrimId }.Get<SPrimObjectHeader>();
                if (pHeader && pHeader->lType == EPrimType::PTOBJECTHEADER && pHeader->lPlanes)
                {
                    m_lUnknown94 = reinterpret_cast<uint32_t>(ZPrimHandle{ pHeader->lPlanes }.Get<void>());
                }
            }
        }
        else if (sInfo.m_lPrim)
        {
            m_lPrimId = sInfo.m_lPrim;
        }
    }

    ZRenderEntryGeom::~ZRenderEntryGeom()
    {
        if (m_pBaseGeom)
        {
            m_pBaseGeom->m_lControl &= ~0x1000u;
            if (m_pBaseGeom->WantCameraMsg())
            {
                m_pBaseGeom->GetGeom()->SendCommand(0x805u, nullptr, nullptr);
            }
        }
    }

    void ZRenderEntryGeom::Notify(const SRenderEntryNotifyInfo* pEntry)
    {
        const uint32_t lDrawDestinationOverride = pEntry->lDrawDestinationOverride;
        if (lDrawDestinationOverride)
        {
            m_lDrawDestinationOverride = lDrawDestinationOverride;
        }

        uint8_t lLODMask = pEntry->lLODMask;
        if (m_lUnknown80 != 0xFFFFFFFFu)
        {
            lLODMask = static_cast<uint8_t>(m_lUnknown80);
        }
        if ((m_lControl & RE_WANT_VIEW_NOTIFY) != 0)
        {
            lLODMask = 0xFF;
        }

        if ((lLODMask & static_cast<uint8_t>(m_lLODLevelsActive | m_lLODLevelsWanted)) != lLODMask)
        {
            if (m_lPrimId)
            {
                auto* pDraw = IDraw::Instance<ZRenderDraw>();
                const SPrimObjectHeader* pHeader = ZPrimHandle{ m_lPrimId }.Get<SPrimObjectHeader>();

                if (pHeader && pHeader->lType == EPrimType::PTOBJECTHEADER && pHeader->lNumObjects)
                {
                    const uint32_t* pObjectTable = ZPrimHandle{ pHeader->lObjectTable }.Get<uint32_t>();
                    SRenderEntryInstance* aInstances[ZDECALMARK_MAX_NUM_DECALS];
                    uint32_t lInstanceIndex = 0u;

                    for (uint32_t i = 0; i < pHeader->lNumObjects; ++i)
                    {
                        const ZPrimHandle hSubObject{ pObjectTable[i] };
                        const SPrimObject* pSubObject = hSubObject.Get<SPrimObject>();

                        if ((pSubObject->lDrawMode & 0x800000u) == 0)
                        {
                            const uint8_t lSubLODMask = pSubObject->lLODMask;
                            if ((lSubLODMask & lLODMask) != 0
                                && ((m_lLODLevelsActive | m_lLODLevelsWanted) & lSubLODMask) == 0)
                            {
                                if (!m_lVariantId || pSubObject->lVariantId == m_lVariantId)
                                {
                                    auto* pInstance = pDraw->CreateRenderEntryInstance(hSubObject, this, m_pBaseGeom, false);
                                    if (pInstance)
                                    {
                                        ZASSERT(lInstanceIndex < ZDECALMARK_MAX_NUM_DECALS);

                                        const uint32_t lBoneIndexMask = GetBoneIndexMask(pSubObject);
                                        InitRenderEntryInstance(pInstance, pSubObject->lLODMask, pSubObject->lDrawDestination, 0u, lBoneIndexMask);

                                        m_lControl |= RE_NEEDUPDATE;
                                        aInstances[lInstanceIndex++] = pInstance;

                                        if ((pSubObject->lDrawMode & 0x2000u) != 0)
                                        {
                                            m_lControl |= RE_STATIC_SHADOW_SUB;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (lInstanceIndex)
                    {
                        AddRenderEntryInstances(aInstances, lInstanceIndex);
                    }
                }
            }
        }

        m_lLODLevelsWanted |= lLODMask;
        if ((m_lControl & RE_ATTACH_UPDATE) != 0)
        {
            AttachUpdate();
        }
    }

    void ZRenderEntryGeom::Update()
    {
        if (m_lLODLevelsWanted)
        {
            ZBaseGeom* pBaseGeom = m_pBaseGeom;
            if (pBaseGeom && pBaseGeom->m_uListID != 0)
            {
                if (pBaseGeom->m_lPotentialLightListChange != 0)
                {
                    pBaseGeom->FixLightList();
                }

                if ((pBaseGeom->m_lControl & ZCLIGHTCHANGED) != 0)
                {
                    pBaseGeom->UpdateLightListForGeom();
                }

                if ((pBaseGeom->m_lControl & ZCUPDATELIGHT) != 0)
                {
                    pBaseGeom->m_lControl &= ~ZCUPDATELIGHT;
                    m_lControl |= RE_NEEDUPDATE | RE_UPDATELIGHT;
                }
            }

            if ((m_lControl & RE_HASMOVED) != 0)
            {
                m_lControl |= RE_NEEDUPDATE;

                for (uint32_t i = 0; i < m_lNumRenderEntryInstances; ++i)
                {
                    m_pRenderEntryInstances[i]->pRenderObjectInstance->m_lFlags |= 1u;
                }
            }
        }
    }

    void ZRenderEntryGeom::AttachUpdate()
    {
        ZPrimHandle aAttachedPrims[128];
        SRenderEntryInstance* aInstances[ZDECALMARK_MAX_NUM_DECALS];
        uint32_t lNrAttachedObjects = 0u, lInstanceIndex = 0u, lEntryIndex = 0u;
        auto* pRenderDraw = IDraw::Instance<ZRenderDraw>();

        if (m_pBaseGeom && (m_pBaseGeom->Control() & ZCRENDERATTACHED) != 0)
        {
            lNrAttachedObjects = pRenderDraw->m_DecalMarks.GetAttachedObjects(m_pBaseGeom, aAttachedPrims, 128);
        }

        for (; lEntryIndex < m_lNumRenderEntryInstances; ++lEntryIndex)
        {
            auto* pInstance = m_pRenderEntryInstances[lEntryIndex];
            if ((pInstance->lTransparencyMask & 1) != 0)
            {
                m_pRenderEntryInstances[lEntryIndex] = nullptr;
                pRenderDraw->DestroyRenderEntryInstance(pInstance);
            }
            else
            {
                aInstances[lInstanceIndex++] = pInstance;
            }
        }

        for (uint32_t lAttachedObjectIndex = 0u; lAttachedObjectIndex < lNrAttachedObjects; ++lAttachedObjectIndex)
        {
            auto& hCurrentObject = aAttachedPrims[lAttachedObjectIndex];
            const SPrimObject* pPrimObject = hCurrentObject;

            auto* pEntryInstance= pRenderDraw->CreateRenderEntryInstance(hCurrentObject, this, m_pBaseGeom, true);;
            if (pEntryInstance)
            {
                ZASSERT(lInstanceIndex < ZDECALMARK_MAX_NUM_DECALS);
                InitRenderEntryInstance(pEntryInstance, pPrimObject->lLODMask, pPrimObject->lDrawDestination, 1u, 0u);
                aInstances[lInstanceIndex++] = pEntryInstance;
                m_lControl |= RE_NEEDUPDATE;
            }
        }

        auto** pRenderEntryInstance = m_pRenderEntryInstances;
        if (pRenderEntryInstance && pRenderEntryInstance != m_RenderEntryInstanceTable)
        {
            ZUniMemory::Delete(pRenderEntryInstance);
        }

        m_pRenderEntryInstances = nullptr;
        m_lNumRenderEntryInstances = 0;

        if (lInstanceIndex)
        {
            AddRenderEntryInstances(aInstances, lInstanceIndex);
        }

        m_lControl &= ~RE_ADDTOREUSE;
    }

    void ZRenderEntryGeom::SetRenderContext(ZRenderContext* pContext, const ZRenderObjectInstance* pObjInstance)
    {
        ZPrimHandle hMesh = pObjInstance->m_pRenderObject->m_hPrim;
        const SPrimMesh* pMesh = hMesh;

        ZMatrix mObjectToWorld = m_ObjectToWorldMatrix;

        if (pMesh->lType == EPrimType::PTMESH)
        {
            const uint8_t lSubType = pMesh->lProperties & 7;
            if (lSubType)
            {
                ZMat3x3 mMat;
                const float* pViewMatrix = pContext->m_WorldToViewMatrix.m0.Get();

                switch (lSubType)
                {
                    case 1:
                    {
                        mreset(mMat.Get());
                        mMat.data[6] = pViewMatrix[6];
                        mMat.data[7] = pViewMatrix[3];
                        mMat.data[8] = pViewMatrix[0];
                        vcross(&mMat.data[3], mMat.data, &mMat.data[6]);
                        vnorm(&mMat.data[3]);
                        vcross(mMat.data, &mMat.data[6], &mMat.data[3]);
                    }
                    break;
                    case 2:
                    {
                        mreset(mMat.Get());
                        mMat.data[3] = pViewMatrix[7];
                        mMat.data[4] = pViewMatrix[4];
                        mMat.data[5] = pViewMatrix[1];
                        vcross(mMat.data, &mMat.data[6], &mMat.data[3]);
                        vnorm(mMat.data);
                        vcross(&mMat.data[6], &mMat.data[3], mMat.data);
                    }
                    break;
                    case 4:
                    {
                        mreset(mMat.Get());
                        mMat.data[0] = pViewMatrix[8];
                        mMat.data[1] = pViewMatrix[5];
                        mMat.data[2] = pViewMatrix[2];
                        vcross(&mMat.data[6], &mMat.data[3], mMat.data);
                        vnorm(&mMat.data[6]);
                        vcross(&mMat.data[3], mMat.data, &mMat.data[6]);
                    }
                    break;
                    default:
                    {
                        tmat(mMat.data, pViewMatrix);
                    }
                    break;
                }

                mObjectToWorld.m0 = mMat;
                mObjectToWorld.p0 = m_ObjectToWorldMatrix.p0;
            }
        }

        pContext->m_ObjectToWorldMatrix = mObjectToWorld;
        pContext->m_lRenderFlags = m_lRenderFlags;

        float fObjectFadeScale = 1.0f;
        if (m_lFade == 0xFF)
        {
            fObjectFadeScale = 2.0f;
        }
        else
        {
            fObjectFadeScale = static_cast<float>(m_lFade) * 0.0039370079f;
        }

        float fObjectFade = (fObjectFadeScale * 0.85000002f) + 0.15000001f;
        if (fObjectFade <= 1.0f)
        {
            pContext->m_fObjectFade = fObjectFade;
        }
        else
        {
            pContext->m_fObjectFade = 1.0f;
        }
    }

    ZRenderEntryGeom::RENDERENTRY_BASETYPE ZRenderEntryGeom::GetType() const
    {
        return RENDERENTRY_BASETYPE::RT_GEOM;
    }

    ZBaseGeom* ZRenderEntryGeom::GetBaseGeom() const
    {
        return m_pBaseGeom;
    }

    uint32_t ZRenderEntryGeom::GetPrim() const
    {
        return m_lPrimId;
    }
}
