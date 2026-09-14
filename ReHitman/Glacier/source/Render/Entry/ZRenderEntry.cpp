#include <Glacier/Render/Material/ZRenderMaterialInstance.h>
#include <Glacier/Render/Material/ZRenderMaterialBuffer.h>
#include <Glacier/Render/Object/ZRenderObjectInstance.h>
#include <Glacier/Render/Object/ZRenderObject.h>
#include <Glacier/Render/Entry/ZRenderEntry.h>
#include <Glacier/Render/Entry/SRenderEntryNotifyInfo.h>
#include <Glacier/Render/Entry/SRenderEntryInstance.h>
#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Debug/ZPushMemColor.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZUniMemory.h>

#include <iterator>


namespace Glacier
{
    ZRenderEntry::ZRenderEntry() = default;

    ZRenderEntry::~ZRenderEntry()
    {
        m_lGeomListsControl = 0;
        ZASSERT(!m_lEntryListsMask);

        if (m_lNumRenderEntryInstances)
        {
            for (int i = 0; i < m_lNumRenderEntryInstances; ++i)
            {
                auto* pEntry = m_pRenderEntryInstances[i];
                m_pRenderEntryInstances[i] = nullptr;
                IDraw::Instance<ZRenderDraw>()->DestroyRenderEntryInstance(pEntry);
            }
        }

        // Cleanup possible alloc (lmao wat?)
        auto* pInst = m_pRenderEntryInstances;
        if (pInst && pInst != m_RenderEntryInstanceTable)
        {
            ZUniMemory::Free(pInst);
        }

        m_pRenderEntryInstances = nullptr;
        m_lNumRenderEntryInstances = 0;
    }

    void ZRenderEntry::Notify(const SRenderEntryNotifyInfo* pEntry)
    {
    }

    void ZRenderEntry::Update()
    {
    }

    void ZRenderEntry::AttachUpdate()
    {
    }

    void ZRenderEntry::GetVisible(ZCmdList* pCmdList, ZRenderEntryGeom* pGeomEntry, ZViewSpace* pViewSpace, ZRenderView* pView, ZRenderEntryLists* pEntryList)
    {
    }

    uint32_t ZRenderEntry::GetInstanceRepeat(const ZRenderObjectInstance* pObjInstance)
    {
        return 1u;
    }

    void ZRenderEntry::SetRenderContext(ZRenderContext* pContext, const ZRenderObjectInstance* pObjInstance)
    {
        pContext->m_ObjectToWorldMatrix = m_ObjectToWorldMatrix;
    }

    void ZRenderEntry::EndFrame()
    {
        m_lControl &= ~(RE_NOTIFIED | RE_NEEDUPDATE | RE_UPDATELIGHT | RE_CREATEDTHISFRAME | RE_HASMOVED);
        m_pEnvironment = nullptr;

        const auto lRemovedLODLevels = m_lLODLevelsActive & ~m_lLODLevelsWanted;
        if (lRemovedLODLevels)
        {
            uint32_t lWriteIndex = 0;
            for (uint32_t lReadIndex = 0; lReadIndex < m_lNumRenderEntryInstances; ++lReadIndex)
            {
                auto* pEntry = m_pRenderEntryInstances[lReadIndex];

                if ((pEntry->lBoneIndexMask & static_cast<uint8_t>(lRemovedLODLevels)) == 0 ||
                    (pEntry->lBoneIndexMask & m_lLODLevelsWanted) != 0 ||
                    (pEntry->lTransparencyMask & 1) != 0)
                {
                    m_pRenderEntryInstances[lWriteIndex++] = pEntry;
                }
                else
                {
                    m_pRenderEntryInstances[lReadIndex] = nullptr;
                    IDraw::Instance<ZRenderDraw>()->DestroyRenderEntryInstance(pEntry);
                }
            }
            m_lNumRenderEntryInstances = static_cast<uint16_t>(lWriteIndex);
        }

        m_lLODLevelsActive = m_lLODLevelsWanted;
        m_lLODLevelsWanted = 0u;
    }

    void ZRenderEntry::GetDrawInstances(SRenderEntryInstance** ppEntries, SRenderEntryNotifyInfo* pEntry)
    {
        CalcLODMask(pEntry);
        Notify(pEntry);

        // Do we need calculate that LOD levels?
        if ((m_lLODLevelsWanted & pEntry->lLODMask) == 0)
        {
            // I guess no
            return;
        }

        const uint32_t lHiddenBoneIndices = GetHiddenBoneIndices();
        uint32_t lOutIndex = 0;

        if ((m_lControl & RE_WANT_VIEW_NOTIFY) != 0) // 0x200
        {
            ZBaseGeom* pGeomBase = GetBaseGeom();

            for (uint32_t i = 0; i < m_lNumRenderEntryInstances; ++i)
            {
                auto* pCurrentEntry = m_pRenderEntryInstances[i];

                // Skip if hidden
                if ((lHiddenBoneIndices & pCurrentEntry->lLayerMask) != 0)
                {
                    continue;
                }

                // Hide or nope?
                bool bWantPrimHide = false;

                if (pGeomBase && pGeomBase->GetGeom())
                {
                    if (pEntry->bMirror)
                    {
                        bWantPrimHide = pGeomBase->GetGeom()->WantViewPrimHideMirrors(pCurrentEntry->lBoneIndexMask);
                    }
                    else
                    {
                        bWantPrimHide = pGeomBase->GetGeom()->WantViewPrimHide(pCurrentEntry->lBoneIndexMask, pEntry->bFirstPersonCamera);
                    }
                }

                // No hide - add to ppEntries
                if (!bWantPrimHide)
                {
                    ppEntries[lOutIndex++] = pCurrentEntry;
                }
            }
        }
        else
        {
            for (uint32_t i = 0; i < m_lNumRenderEntryInstances; ++i)
            {
                auto* pCurrent = m_pRenderEntryInstances[i];
                const uint8_t lodMaskByte = static_cast<uint8_t>(pEntry->lLODMask);

                if ((pCurrent->lBoneIndexMask & lodMaskByte) != 0 &&
                    (lHiddenBoneIndices & pCurrent->lLayerMask) == 0)
                {
                    ppEntries[lOutIndex++] = pCurrent;
                }
            }
        }
    }

    ZBaseGeom* ZRenderEntry::GetBaseGeom() const
    {
        return nullptr;
    }

    uint32_t ZRenderEntry::GetPrim() const
    {
        return 0u;
    }

    uint32_t ZRenderEntry::GetHiddenBoneIndices() const
    {
        return 0u;
    }

    uint32_t ZRenderEntry::GetBoneIndexMask(const SPrimObject* pPrimObject)
    {
        return 0u;
    }

    float* ZRenderEntry::GetPlanes() const
    {
        return nullptr;
    }

    void ZRenderEntry::CalcLODMask(SRenderEntryNotifyInfo* pEntry)
    {
        // PC (0x00476890): resets the draw destination override, then either the LOD planes
        // from GetPlanes() say the observer is between them (distance forced to 0, all LODs masked out),
        // or the LOD level is picked from the ZRender distance-indexed lookup table.
        pEntry->lDrawDestinationOverride = 0;

        const float* pPlanes = GetPlanes();
        if (pPlanes)
        {
            ZVector3 vNormal;
            ZVector3 vToObserver;

            // First plane: observer on the outer side of it -> regular distance-based selection
            vmmul(vNormal.Get(), &pPlanes[0], m_ObjectToWorldMatrix.m0.Get());
            vaddscalar(vToObserver.Get(), m_ObjectToWorldMatrix.p0.Get(), vNormal.Get(), -pPlanes[3]);
            vsub(vToObserver.Get(), pEntry->vObserver, vToObserver.Get());

            if (vToObserver.z * vNormal.z + vToObserver.y * vNormal.y + vToObserver.x * vNormal.x < 0.0f)
            {
                // Second plane: observer between both planes -> force full detail
                vmmul(vNormal.Get(), &pPlanes[4], m_ObjectToWorldMatrix.m0.Get());
                vaddscalar(vToObserver.Get(), m_ObjectToWorldMatrix.p0.Get(), vNormal.Get(), -pPlanes[7]);
                vsub(vToObserver.Get(), pEntry->vObserver, vToObserver.Get());

                if (vToObserver.z * vNormal.z + vToObserver.y * vNormal.y + vToObserver.x * vNormal.x < 0.0f)
                {
                    pEntry->fDistance = 0.0f;
                    pEntry->lLODMask = 0;
                    return;
                }
            }
        }

        const float fDistance = vdist(pEntry->vObserver, m_ObjectToWorldMatrix.p0.Get());
        pEntry->fDistance = fDistance;

        int lLODIndex = static_cast<int>(fDistance * pEntry->fLODScale);
        if (lLODIndex < 0)
        {
            lLODIndex = 0;
        }
        else if (lLODIndex > 255)
        {
            lLODIndex = 255;
        }

        pEntry->lLODMask = 1u << g_pSysInterface->WindowFirst->m_aLODLookupTable[lLODIndex];
    }

    void ZRenderEntry::GetAttachedBaseGeoms(ZStackArray<1024, ZRenderEntry::ZAttachedBaseGeom>* pArray)
    {
    }

    void* ZRenderEntry::AllocateMemory(uint32_t lSize)
    {
        if (lSize <= std::size(m_RenderEntryInstanceTable))
        {
            return m_RenderEntryInstanceTable;
        }

        {
            PUSH_MEMORY_COLOR(0xF00000u);
            return ZUniMemory::Allocate(sizeof(SRenderEntryInstance*) * lSize, RENDERCPU_MEM);
        }
    }

    void ZRenderEntry::SetObjectToWorldMatrix(const ZMatrix& mMatrix)
    {
        // PC (0x00476800): copies the matrix dword-wise and raises RE_HASMOVED only if something actually changed
        auto* pDst = reinterpret_cast<uint32_t*>(&m_ObjectToWorldMatrix);
        const auto* pSrc = reinterpret_cast<const uint32_t*>(&mMatrix);

        bool bChanged = false;
        for (size_t i = 0; i < sizeof(ZMatrix) / sizeof(uint32_t); ++i)
        {
            if (pDst[i] != pSrc[i])
            {
                pDst[i] = pSrc[i];
                bChanged = true;
            }
        }

        if (bChanged)
        {
            m_lControl |= RE_HASMOVED;
        }
    }

    bool ZRenderEntry::AddRenderEntryInstances(SRenderEntryInstance** pInstances, uint32_t lNumRenderEntryInstances)
    {
        // Compute enough space for old elems + new elems
        const uint32_t lTotalInstances = m_lNumRenderEntryInstances + lNumRenderEntryInstances;

        // Allocate enough space
        auto** pAllocatedEntries = static_cast<SRenderEntryInstance**>(AllocateMemory(lTotalInstances));

        if (pAllocatedEntries)
        {
            // Copy exists elements
            for (uint32_t i = 0; i < m_lNumRenderEntryInstances; ++i)
            {
                pAllocatedEntries[i] = m_pRenderEntryInstances[i];
            }

            // Add new elems
            for (uint32_t j = 0; j < lNumRenderEntryInstances; ++j)
            {
                ZASSERT(pInstances[j] != nullptr);
                pAllocatedEntries[m_lNumRenderEntryInstances + j] = pInstances[j];
            }

            // Release old buffer (check for SBO first)
            if (m_pRenderEntryInstances)
            {
                if (m_pRenderEntryInstances != m_RenderEntryInstanceTable)
                {
                    ZUniMemory::Free(m_pRenderEntryInstances);
                }
            }

            // Update & done
            m_lNumRenderEntryInstances = static_cast<uint16_t>(lTotalInstances);
            m_pRenderEntryInstances = pAllocatedEntries;
            return true;
        }

        // On error: cleanup and fault
        auto* pDraw = IDraw::Instance<ZRenderDraw>();
        for (uint32_t i = 0; i < lNumRenderEntryInstances; ++i)
        {
            pDraw->DestroyRenderEntryInstance(pInstances[i]);
        }

        return false;
    }

    void ZRenderEntry::InitRenderEntryInstance(SRenderEntryInstance *pRenderEntryInstance, uint8_t lLODMask, uint8_t lDrawDestination, uint32_t lFlags, uint32_t lBoneIndexMask)
    {
        auto pMaterialInstance = pRenderEntryInstance->pRenderObjectInstance->m_pRenderObject->m_pMaterialInstance;
        auto lRemapValue = pMaterialInstance->m_lRemapValue;
        auto lNumOpaqueMaterials = ZRenderMaterialBuffer::g_pMaterialBufferInstance->m_lNumOpaqueMaterials;
        auto bEnoughMaterialsToRemap = ZRenderMaterialBuffer::g_pMaterialBufferInstance->m_lNumFullyOpaqueMaterials >= lRemapValue;
        pRenderEntryInstance->lBoneIndexMask = lLODMask;

        auto lDrawDestinationOverride = m_lDrawDestinationOverride;
        auto bEnoughOpaqueMaterialsToRemap = lNumOpaqueMaterials >= lRemapValue;
        if (!lDrawDestinationOverride)
        {
            lDrawDestinationOverride = lDrawDestination;
        }

        pRenderEntryInstance->lLODMask = lDrawDestinationOverride;
        pRenderEntryInstance->lSortValue = pMaterialInstance->m_lLayerMask;
        uint32_t lNewDrawDestination = 0;
        if (bEnoughMaterialsToRemap)
        {
            lNewDrawDestination = 1;
        }
        else
        {
            lNewDrawDestination = 2 * !bEnoughOpaqueMaterialsToRemap + 2;
        }

        pRenderEntryInstance->lDrawDestination = lNewDrawDestination;
        pRenderEntryInstance->lTransparencyMask = lFlags;
        pRenderEntryInstance->lLayerMask = lBoneIndexMask;

        auto pBaseGeom = GetBaseGeom();
        if (pBaseGeom)
        {
            if ((pBaseGeom->Control() & 0x10000) == 0)
            {
                pRenderEntryInstance->lSortValue &= ~4u;
            }

            if ((pBaseGeom->Control() & 0x20000) == 0)
            {
                pRenderEntryInstance->lSortValue &= ~8u;
            }
        }
    }

    void ZRenderEntry::AddToDrawChain(ZCmdList::ZCmd* pCmd, uint8_t lLODLevels, uint32_t lLayerMask, uint8_t lDrawDestination, uint8_t lTransparencyMask, bool bFirstPersonCamera)
    {
        if ((lLODLevels & m_lLODLevelsWanted) == 0)
            return; // Do not draw this thing (culled by LOD)

        const uint32_t lHiddenBoneIndices = GetHiddenBoneIndices();

        if ((m_lControl & RE_WANT_VIEW_NOTIFY) != 0)
        {
            ZBaseGeom* pBaseGeom = GetBaseGeom();

            for (uint32_t i = 0; i < m_lNumRenderEntryInstances; ++i)
            {
                auto* pEntry = m_pRenderEntryInstances[i];

                if ((lLayerMask & pEntry->lSortValue) == 0 ||
                    (lTransparencyMask & pEntry->lDrawDestination) == 0 ||
                    (lHiddenBoneIndices & pEntry->lLayerMask) != 0)
                {
                    continue;
                }

                uint8_t lLODMask = m_lDrawDestinationOverride;
                if (!lLODMask)
                {
                    lLODMask = pEntry->lLODMask;
                }

                if ((lLODMask & lDrawDestination) == 0)
                {
                    continue;
                }

                if (pBaseGeom)
                {
                    const bool bWantPrimHide = (lDrawDestination & 8) != 0
                        ? pBaseGeom->GetGeom()->WantViewPrimHideMirrors(pEntry->lBoneIndexMask)
                        : pBaseGeom->GetGeom()->WantViewPrimHide(pEntry->lBoneIndexMask, bFirstPersonCamera);

                    if (bWantPrimHide)
                    {
                        continue;
                    }
                }

                pCmd->AddObject(pEntry->pRenderObjectInstance);
            }
        }
        else
        {
            for (uint32_t i = 0; i < m_lNumRenderEntryInstances; ++i)
            {
                auto* pEntry = m_pRenderEntryInstances[i];

                if ((lLODLevels & pEntry->lBoneIndexMask) == 0 ||
                    (lLayerMask & pEntry->lSortValue) == 0 ||
                    (lTransparencyMask & pEntry->lDrawDestination) == 0 ||
                    (lHiddenBoneIndices & pEntry->lLayerMask) != 0)
                {
                    continue;
                }

                uint8_t lLODMask = m_lDrawDestinationOverride;
                if (!lLODMask)
                {
                    lLODMask = pEntry->lLODMask;
                }

                if ((lLODMask & lDrawDestination) == 0)
                {
                    continue;
                }

                pCmd->AddObject(pEntry->pRenderObjectInstance);
            }
        }
    }
}
