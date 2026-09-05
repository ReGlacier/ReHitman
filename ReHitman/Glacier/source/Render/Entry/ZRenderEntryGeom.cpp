#include <Glacier/Render/Entry/ZRenderEntryGeom.h>
#include <Glacier/Render/Object/ZRenderObjectInstance.h>
#include <Glacier/Render/Object/ZRenderObject.h>
#include <Glacier/Render/Prim/SPrimObject.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <Glacier/Render/Prim/EPrimType.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/Decal/ZDecalMarkController.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGEOM.h>


namespace Glacier
{
    static constexpr uint32_t ZDECALMARK_MAX_NUM_DECALS = 0x200;

    ZRenderEntryGeom::ZRenderEntryGeom(const ZRenderEntryGeomCreateInfo& sInfo)
        : ZRenderEntry()
    {
        // TODO: Finish me
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
        // TODO: Finish me
    }

    void ZRenderEntryGeom::Update()
    {
        // TODO: Finish me
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
        if (pMesh->lType == EPrimType::PTMESH)
        {
            const auto lSubType = pMesh->lProperties & 7;
            if (lSubType)
            {
                ZMatrix& mWorldToView = pContext->m_WorldToViewMatrix;
                ZMat3x3 mMat;

                // TODO: Finish me
                switch (lSubType)
                {
                    case 1:
                    {
                    }
                    break;
                    case 2:
                    {}
                    break;
                    case 4:
                    {}
                    break;
                    default:
                    {
                        tmat(mMat, mWorldToView.m0);
                    }
                    break;
                }

            }
            // TODO: Finish me
        }

        pContext->m_ObjectToViewMatrix = m_ObjectToWorldMatrix;
        pContext->m_lRenderFlags = m_lRenderFlags; // I'm not sure about this place

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
