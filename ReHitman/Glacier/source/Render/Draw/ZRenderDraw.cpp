#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Render/Entry/ZRenderEntry.h>
#include <Glacier/Render/Entry/ZRenderEntryGeom.h>
#include <Glacier/Render/Entry/ZRenderEntryLists.h>
#include <Glacier/Render/ZVolumeList.h>
#include <Glacier/Render/Entry/ZRenderEntrySprite.h>
#include <Glacier/Render/Entry/ZRenderEntrySpriteD3D.h>
#include <Glacier/Render/Entry/ZRenderEntryGeomD3D.h>
#include <Glacier/Render/Entry/ZRenderEntryCamera.h>
#include <Glacier/Render/Entry/ZRenderEntryCameraD3D.h>
#include <Glacier/Render/Entry/ZRenderEntryBones.h>
#include <Glacier/Render/Entry/ZRenderEntryBonesD3D.h>
#include <Glacier/Render/Entry/ZRenderEntryBonesRigid.h>
#include <Glacier/Render/Entry/ZRenderEntryBonesRigidD3D.h>
#include <Glacier/Render/Entry/ZRenderEntryDeformer.h>
#include <Glacier/Render/Entry/ZRenderEntryDeformerD3D.h>
#include <Glacier/Render/Entry/ZRenderEntryLight.h>
#include <Glacier/Render/Entry/ZRenderEntryLightD3D.h>
#include <Glacier/Render/Entry/SRenderEntryNotifyInfo.h>
#include <Glacier/Render/Material/ZRenderMaterialBuffer.h>
#include <Glacier/Render/Material/ZRenderMaterialInstance.h>
#include <Glacier/Render/Prim/SPrims.h>
#include <Glacier/Render/Prim/SPrimSpritesArray.h>
#include <Glacier/Render/Object/ZRenderObject.h>
#include <Glacier/Render/Object/ZRenderObjectInstance.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Geom/GeomControlMasks.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Prim/SBoneDefinition.h>
#include <Glacier/Geom/ZENVIRONMENT.h>
#include <Glacier/Geom/ZEnvSampler.h>
#include <Glacier/Render/Entry/ZRenderEntryEnvSamplerD3D.h>
#include <Glacier/Render/Entry/ZRenderEntryReflectorD3D.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/Animation/Model.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/System/ZSysMem.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


namespace Glacier
{
    namespace
    {
        /**
         * @brief Flag set while ZRenderDraw::Flush / ZRenderDraw::DestroyRenderEntryInstance are iterating
         * the pending-deletion lists. Guards re-entrant cleanup.
         * PC global: unk_8EC154
         */
        static bool s_bRenderDrawFlushActive = false; // PC: unk_8EC154

        /**
         * @brief Head of the render entry factory list.
         * PC global: dword_8EC158
         */
        static void* s_pRenderEntryFactoryList = nullptr; // PC: dword_8EC158

        using RenderEntryFactory = ZRenderEntry* (*)(ZRenderEntryGeomCreateInfo&);

        ZRenderEntry* CreateGeomD3D(ZRenderEntryGeomCreateInfo& sInfo)
        {
            return ZRenderEntryGeomD3D::Create(sInfo);
        }

        ZRenderEntry* CreateBonesD3D(ZRenderEntryGeomCreateInfo& sInfo)
        {
            return ZRenderEntryBonesD3D::Create(sInfo);
        }

        ZRenderEntry* CreateBonesRigidD3D(ZRenderEntryGeomCreateInfo& sInfo)
        {
            return ZRenderEntryBonesRigidD3D::Create(sInfo);
        }

        ZRenderEntry* CreateCameraD3D(ZRenderEntryGeomCreateInfo& sInfo)
        {
            return ZRenderEntryCameraD3D::Create(sInfo);
        }

        ZRenderEntry* CreateLight(ZRenderEntryGeomCreateInfo& sInfo)
        {
            return ZRenderEntryLightD3D::Create(sInfo);
        }

        ZRenderEntry* CreateSpriteD3D(ZRenderEntryGeomCreateInfo& sInfo)
        {
            return ZRenderEntrySpriteD3D::Create(&sInfo);
        }

        ZRenderEntry* CreateEnvSamplerD3D(ZRenderEntryGeomCreateInfo& sInfo)
        {
            return ZRenderEntryEnvSamplerD3D::Create(sInfo);
        }

        ZRenderEntry* CreateReflectorD3D(ZRenderEntryGeomCreateInfo& sInfo)
        {
            return ZRenderEntryReflectorD3D::Create(sInfo);
        }

        ZRenderEntry* CreateDeformerD3D(ZRenderEntryGeomCreateInfo& sInfo)
        {
            return ZRenderEntryDeformerD3D::Create(sInfo);
        }

        ZRenderEntry* CreateRenderEntryFromFactories(
            ZBaseGeom* pBaseGeom, uint32_t lPrim, ZRenderEntry* pObserverEntry)
        {
            ZRenderEntryGeomCreateInfo sInfo{ pObserverEntry, pBaseGeom, lPrim };
            static const RenderEntryFactory apFactories[] = {
                // PC nodes 0x7F81F8..0x7F8240, in linked-list order.
                CreateGeomD3D,
                CreateBonesD3D,
                CreateBonesRigidD3D,
                CreateCameraD3D,
                CreateLight,
                CreateSpriteD3D,
                CreateEnvSamplerD3D,
                CreateReflectorD3D,
                CreateDeformerD3D,
            };

            for (RenderEntryFactory pFactory : apFactories)
            {
                if (ZRenderEntry* pEntry = pFactory(sInfo))
                    return pEntry;
            }

            return nullptr;
        }
    }

    ZRenderDraw::ZRenderEntryMap::ZRenderEntryMap()
    {
        // PC 0x475010: m_Entries default-constructs (chaining the 1536 slot free-list),
        // then the 1024 byte hash bucket table is zeroed.
        memset(&m_HashToFirst, 0, sizeof(m_HashToFirst));
    }

    ZRenderDraw::ZRenderEntryMap::ZEntry* ZRenderDraw::ZRenderEntryMap::Add(uint32_t lIdentifier, ZRenderEntry* pEntry)
    {
        ZEntry* pInserted = m_Entries.Add();
        if (!pInserted)
            return nullptr;

        pInserted->m_lIndentifier = lIdentifier;
        pInserted->m_pRenderEntry = pEntry;

        ZEntry** const ppBucket = &m_HashToFirst[HashOfIdentifier(lIdentifier)];
        pInserted->m_pNext = *ppBucket;
        *ppBucket = pInserted;

        return pInserted;
    }

    ZRenderEntry* ZRenderDraw::ZRenderEntryMap::GetAndRemove(uint32_t lIdentifier)
    {
        const uint32_t lHash = HashOfIdentifier(lIdentifier);

        // LUT #0
        auto* pEntry = m_HashToFirst[lHash];
        if (!pEntry)
            return nullptr;

        // Fix collisions? Select bucket
        ZRenderDraw::ZRenderEntryMap::ZEntry* pFound = nullptr;
        while (pEntry->m_lIndentifier != lIdentifier)
        {
            pFound = pEntry;
            pEntry = pEntry->m_pNext;
            if (!pEntry)
                return nullptr;
        }

        if (pFound)
        {
            pFound->m_pNext = pEntry->m_pNext;
        }
        else
        {
            m_HashToFirst[lHash] = pEntry->m_pNext;
        }

        // Push the slot back onto the entries free-list.
        // PC 0x473EA0: reads m_lFirstFreeEntry, decrements m_lCount, chains the slot
        // via its low word and stores its index as the new free-list head.
        m_Entries.Remove(pEntry);

        return pEntry->m_pRenderEntry;
    }

    uint32_t ZRenderDraw::ZRenderEntryMap::HashOfIdentifier(uint32_t lIdentifier)
    {
        // PC 0x475060 / 0x473EA0: byte 2 of the identifier selects one of the 256 buckets
        return (lIdentifier >> 4) & 0xFFu; // (uint8_t)(lIdentifier >> 4)
    }

    ZRenderDraw::ZRenderDraw()
        : ZRenderDrawBase()
        , m_DecalMarks()
        , m_RenderEntryIndex(0xF)
        , m_lRenderEntriesCount(0)
        , m_lToBeDeletedCount(0)
        , m_RenderEntryInstances()
    {
        memset(m_apRenderEntryLookup, 0, sizeof(m_apRenderEntryLookup));
        memset(m_apRenderEntries, 0, sizeof(m_apRenderEntries));
        memset(m_apToBeDeleted, 0, sizeof(m_apToBeDeleted));
        // m_RenderObjects default-constructs an empty stlp::map
    }

    void ZRenderDraw::BeginFrame()
    {
        SetMemColor(0xFFFFFFu);
    }

    void ZRenderDraw::Flush()
    {
        // PC: thiscall through vtbl[42] == WaitRenderDone()
        WaitRenderDone();
        m_DecalMarks.Flush();
        ZRenderDrawBase::Flush();

        s_bRenderDrawFlushActive = true;

        for (uint32_t i = 0; i < m_lRenderEntriesCount; ++i)
        {
            ZRenderEntry* pEntry = m_apRenderEntries[i];
            ZASSERT(pEntry);

            ZBaseGeom* pBaseGeom = pEntry->GetBaseGeom();
            if (pBaseGeom)
            {
                pBaseGeom->m_lDrawId = 0;
            }
            ZUniMemory::Delete(pEntry);
        }

        m_lRenderEntriesCount = 0;
        s_bRenderDrawFlushActive = false;

        for (uint32_t i = 0; i < m_lToBeDeletedCount; ++i)
        {
            ZUniMemory::Delete(m_apToBeDeleted[i]);
        }
        m_lToBeDeletedCount = 0;

        for (auto& kv : m_RenderObjects)
        {
            ZUniMemory::Delete(kv.second);
        }
        m_RenderObjects.clear();

        memset(m_apRenderEntryLookup, 0, sizeof(m_apRenderEntryLookup));
        m_RenderEntryIndex.Clean();

        ZASSERT(m_RenderEntryInstances.Count() == 0);
    }

    uint32_t ZRenderDraw::AddMark(
        const float* vPosition,
        const float* vDirection,
        uint32_t lArrayIndex,
        uint32_t lSourcePrim,
        float fRadiusX, float fRadiusY,
        const float* fExtraTextureSize,
        bool bStoreUV,
        float fRotation
    )
    {
        ZASSERT(m_bInitialized);
        return reinterpret_cast<uint32_t>(
            m_DecalMarks.Add(vPosition, vDirection, lSourcePrim, fRadiusX, fRadiusY, fExtraTextureSize, bStoreUV, fRotation)
        );
    }

    void ZRenderDraw::AddBoneMark(ZBaseGeom* pBaseGeom, const float* vPosition, const float* vDirection, float fRadius, uint32_t lBoneId, uint32_t lSourcePrim)
    {
        ZASSERT(m_bInitialized);

        ZGEOM* pGeom = pBaseGeom->m_pExtraGeom;
        const bool bIsLnkObj = pGeom
            ? (pGeom->GetObjectId() & ZLNKOBJ::m_Mask) == ZLNKOBJ::m_Id
            : pBaseGeom->IsDerivedFromStdObj(ZLNKOBJ::m_Id);

        if (bIsLnkObj)
        {
            m_DecalMarks.AddBoneDecal(
                reinterpret_cast<ZLNKOBJ*>(pBaseGeom->m_pExtraGeom),
                vPosition, vDirection, fRadius, lBoneId, lSourcePrim
            );
        }
    }

    void ZRenderDraw::RemoveMark(uint32_t lHandle)
    {
        ZASSERT(m_bInitialized);
        m_DecalMarks.Remove(reinterpret_cast<ZDecalMarkController::ZDecalMark*>(lHandle));
    }

    void ZRenderDraw::ModifyMark(uint32_t lHandle, float fOpacity)
    {
        ZASSERT(m_bInitialized);
        m_DecalMarks.Modify(reinterpret_cast<ZDecalMarkController::ZDecalMark*>(lHandle), fOpacity);
    }

    void ZRenderDraw::SetMarkCallBack(uint32_t lHandle, DecalViewCallback_t pCallBack, void* pCallBackData, uint32_t lCallBackValue)
    {
        m_DecalMarks.SetCallBack(reinterpret_cast<ZDecalMarkController::ZDecalMark*>(lHandle), pCallBack, pCallBackData, lCallBackValue);
    }

    uint32_t ZRenderDraw::GetVisibleBoneObjects(ZBaseGeom** pBoneObjects, int ArraySize)
    {
        int i = 0;
        for (uint32_t idx = 0; idx < m_lRenderEntriesCount; ++idx)
        {
            if (i >= ArraySize)
            {
                break;
            }

            ZRenderEntry* pEntry = m_apRenderEntries[idx];
            ZASSERT(pEntry);

            if (pEntry->m_lControl & ZRenderEntry::RE_ISACTOR)
            {
                pBoneObjects[i++] = pEntry->GetBaseGeom();
            }
        }
        return static_cast<uint32_t>(i);
    }

    void ZRenderDraw::RemoveBaseGeom(ZBaseGeom* pBaseGeom, bool bRemoveDecals)
    {
        if (bRemoveDecals)
        {
            m_DecalMarks.RemoveBaseGeom(pBaseGeom);
        }

        const uint16_t lDrawId = pBaseGeom->m_lDrawId;
        if (lDrawId)
        {
            ZRenderEntry* pEntry = m_apRenderEntryLookup[lDrawId & 0x7FFF];
            m_apRenderEntryLookup[lDrawId & 0x7FFF] = nullptr;
            m_RenderEntryIndex.Free(lDrawId - 1);

            pBaseGeom->m_lDrawId = 0;
            pBaseGeom->SetControlDirect(0, 0x1000);

            if (pBaseGeom->WantCameraMsg() && pBaseGeom->m_pExtraGeom)
            {
                pBaseGeom->m_pExtraGeom->SendCommand(0x805, nullptr, nullptr); // DeactivateFrameUpdate?
            }

            // Clear the geom back-pointer on the render entry (ZRenderEntryGeom::m_pBaseGeom at +0x8C)
            reinterpret_cast<ZRenderEntryGeom*>(pEntry)->m_pBaseGeom = nullptr;
        }
    }

    void ZRenderDraw::UpdateBaseGeom(ZBaseGeom* pBaseGeom)
    {
        RemoveBaseGeom(pBaseGeom, true);
    }

    void ZRenderDraw::ChangePrim(ZBaseGeom* pBaseGeom, uint32_t lPrim)
    {
        RemoveBaseGeom(pBaseGeom, true);
    }

    bool ZRenderDraw::ValidateCaster(const ZBaseGeom* pBaseGeom)
    {
        if (!pBaseGeom->m_lDrawId)
        {
            return false;
        }

        const uint32_t lControl = pBaseGeom->m_lControl;
        if ((lControl & 0x2000) != 0 || (lControl & 0x10000) == 0)
        {
            return false;
        }

        ZGEOM* pGeom = pBaseGeom->m_pExtraGeom;
        const bool bIsLnkObj = pGeom
            ? (pGeom->GetObjectId() & ZLNKOBJ::m_Mask) == ZLNKOBJ::m_Id
            : pBaseGeom->IsDerivedFromStdObj(ZLNKOBJ::m_Id);
        if (!bIsLnkObj)
            return false;

        const uint16_t lDrawId = pBaseGeom->m_lDrawId;
        const ZRenderEntry* pEntry = m_apRenderEntryLookup[lDrawId];
        if (!pEntry)
        {
            return false;
        }

        return static_cast<const ZRenderEntryBones*>(pEntry)->m_fMinDistanceToObservers <= 2000.0f;
    }

    const ZBone* ZRenderDraw::GetBaseGeomBones(const ZBaseGeom* pBaseGeom)
    {
        const uint16_t lDrawId = pBaseGeom->m_lDrawId;
        if (!lDrawId)
        {
            return nullptr;
        }

        ZRenderEntry* pEntry = m_apRenderEntryLookup[lDrawId];
        if (!pEntry || (pEntry->m_lControl & ZRenderEntry::RE_HASBONES) == 0)
        {
            return nullptr;
        }

        const auto* pBones = static_cast<const ZRenderEntryBones*>(pEntry);
        return pBones->m_lNumAllocatedBones
            ? reinterpret_cast<const ZBone*>(pBones->m_pBoneData + 16 * (3 * pBones->m_lNumAllocatedBones + 9))
            : nullptr;
    }

    bool ZRenderDraw::ValidateReceiver(const ZBaseGeom* pBaseGeom)
    {
        if (!pBaseGeom->m_lDrawId || (pBaseGeom->m_lControl & 0x2000) != 0)
        {
            return false;
        }

        ZGEOM* pGeom = pBaseGeom->m_pExtraGeom;
        if (pGeom)
        {
            return (pGeom->GetObjectId() & ZLNKOBJ::m_Mask) != ZLNKOBJ::m_Id;
        }

        return !pBaseGeom->IsDerivedFromStdObj(ZLNKOBJ::m_Id);
    }

    void ZRenderDraw::WaitRenderDone()
    {
        // Do nothing
    }

    void ZRenderDraw::SetTextureFrameNumber(const ZBaseGeom* pBaseGeom, float fTextureFrameNumber)
    {
        const uint16_t lDrawId = pBaseGeom->m_lDrawId;
        if (lDrawId)
        {
            ZRenderEntry* pEntry = m_apRenderEntryLookup[lDrawId];
            if (pEntry)
            {
                memcpy(&static_cast<ZRenderEntryGeom*>(pEntry)->m_lRenderFlags, &fTextureFrameNumber, sizeof(fTextureFrameNumber));
            }
        }
    }

    SRenderEntryInstance* ZRenderDraw::CreateRenderEntryInstance(const ZPrimHandle& hPrim, ZRenderEntry* pEntry, ZBaseGeom* pBaseGeom, bool bUniqueRenderObject)
    {
        ZRenderObject* pRenderObject = nullptr;

        if (bUniqueRenderObject)
        {
            const auto* pPrim = hPrim.Get<SPrims>();
            if (pPrim && ZRenderMaterialBuffer::g_pMaterialBufferInstance)
            {
                const auto* pSprite = hPrim.Get<SPrimSpritesArray>();
                const uint32_t lMaterialId = pPrim->lType == 2 && pSprite
                    ? ZRenderMaterialBuffer::g_pMaterialBufferInstance->CreateMaterialInstanceSprite(
                        pSprite->lTextureId, pSprite->lDrawMode, pSprite->lSpriteType)
                    : pPrim->lTextureId;
                auto* pMaterial = ZRenderMaterialBuffer::g_pMaterialBufferInstance->GetMaterialInstance(lMaterialId);
                if (pMaterial)
                {
                    pRenderObject = pMaterial->CreateRenderObject(hPrim);
                    if (pRenderObject)
                        pRenderObject->m_lFlags |= 2;
                }
            }
        }
        else
        {
            auto it = m_RenderObjects.find(hPrim.m_lHandleValue);
            if (it != m_RenderObjects.end())
            {
                pRenderObject = it->second;
            }
            else
            {
                const auto* pPrim = hPrim.Get<SPrims>();
                if (pPrim && ZRenderMaterialBuffer::g_pMaterialBufferInstance)
                {
                    const auto* pSprite = hPrim.Get<SPrimSpritesArray>();
                    const uint32_t lMaterialId = pPrim->lType == 2 && pSprite
                        ? ZRenderMaterialBuffer::g_pMaterialBufferInstance->CreateMaterialInstanceSprite(
                            pSprite->lTextureId, pSprite->lDrawMode, pSprite->lSpriteType)
                        : pPrim->lTextureId;
                    auto* pMaterial = ZRenderMaterialBuffer::g_pMaterialBufferInstance->GetMaterialInstance(lMaterialId);
                    if (pMaterial)
                    {
                        pRenderObject = pMaterial->CreateRenderObject(hPrim);
                        if (pRenderObject)
                            m_RenderObjects[hPrim.m_lHandleValue] = pRenderObject;
                    }
                }
            }
        }

        if (!pRenderObject)
        {
            return nullptr;
        }

        ZRenderObjectInstance* pObjInstance = pRenderObject->CreateInstance(pBaseGeom);
        if (!pObjInstance)
        {
            return nullptr;
        }

        pObjInstance->m_pRenderEntry = pEntry;

        SRenderEntryInstance* pInstance = m_RenderEntryInstances.Add();
        if (!pInstance)
        {
            // Pool exhausted: the object instance is leaked on PC (writes to null base).
            return nullptr;
        }

        pInstance->pRenderObjectInstance = pObjInstance;
        return pInstance;
    }

    void ZRenderDraw::DestroyRenderEntryInstance(SRenderEntryInstance* pRenderEntryInstance)
    {
        if (m_lToBeDeletedCount == 512)
        {
            WaitRenderDone();
            for (uint32_t i = 0; i < m_lToBeDeletedCount; ++i)
            {
                ZUniMemory::Delete(m_apToBeDeleted[i]);
            }
            m_lToBeDeletedCount = 0;
        }

        m_apToBeDeleted[m_lToBeDeletedCount++] = pRenderEntryInstance->pRenderObjectInstance;
        m_RenderEntryInstances.Remove(pRenderEntryInstance);
    }

    void ZRenderDraw::CleanupUnused()
    {
        // Sweep render entries: keep notified ones, destroy the rest
        uint32_t i = 0;
        while (i < m_lRenderEntriesCount)
        {
            ZRenderEntry* pEntry = m_apRenderEntries[i];
            ZASSERT(pEntry);

            if (pEntry->m_lControl & ZRenderEntry::RE_NOTIFIED)
            {
                pEntry->EndFrame();
                ++i;
            }
            else
            {
                ZBaseGeom* pBaseGeom = pEntry->GetBaseGeom();
                if (pBaseGeom)
                {
                    const uint16_t lDrawId = pBaseGeom->m_lDrawId;
                    if (lDrawId)
                    {
                        m_apRenderEntryLookup[lDrawId & 0x7FFF] = nullptr;
                        m_RenderEntryIndex.Free(lDrawId - 1);
                        pBaseGeom->m_lDrawId = 0;
                    }
                }

                ZASSERT(m_lRenderEntriesCount > 0 && m_lRenderEntriesCount <= 0x8000);
                ZASSERT(i < m_lRenderEntriesCount);
                --m_lRenderEntriesCount;
                m_apRenderEntries[i] = m_apRenderEntries[m_lRenderEntriesCount];

                ZUniMemory::Delete(pEntry);
            }
        }

        // Flush pending object-instance deletions
        if (m_lToBeDeletedCount)
        {
            for (uint32_t j = 0; j < m_lToBeDeletedCount; ++j)
            {
                ZUniMemory::Delete(m_apToBeDeleted[j]);
            }
            m_lToBeDeletedCount = 0;
        }

        // Sweep shared render objects without active instances
        for (auto it = m_RenderObjects.begin(); it != m_RenderObjects.end(); )
        {
            ZRenderObject* pObject = it->second;
            if (pObject->m_lNumActiveInstances)
            {
                ++it;
            }
            else
            {
                ZUniMemory::Delete(pObject);
                auto itNext = it;
                ++itNext;
                m_RenderObjects.erase(it);
                it = itNext;
            }
        }
    }

    void ZRenderDraw::CalcBoneLightSources(ZRenderEntryBones* pRenderEntryBones, float* pDirectLights)
    {
        ZBaseGeom* pBaseGeom = pRenderEntryBones->GetBaseGeom();
        ZGEOM* pGeom = pBaseGeom->m_pExtraGeom;
        const bool bIsLnkObj = pGeom
            ? (pGeom->GetObjectId() & ZLNKOBJ::m_Mask) == ZLNKOBJ::m_Id
            : pBaseGeom->IsDerivedFromStdObj(ZLNKOBJ::m_Id);
        ZASSERT(bIsLnkObj);

        const uint16_t lDrawId = pBaseGeom->m_lDrawId;
        if (lDrawId)
        {
            ZRenderEntry* pEntry = m_apRenderEntryLookup[lDrawId];
            if (pEntry)
            {
                CalcBoneLightSources(pBaseGeom, pDirectLights);
            }
        }
    }

    ZRenderEntry* ZRenderDraw::AddRenderEntryArray(uint32_t lPrim, const SDrawArray* pDrawArray)
    {
        ZRenderEntry* pEntry = m_pEntryReuse->GetAndRemove(lPrim);
        if (!pEntry)
        {
            pEntry = CreateRenderEntryFromFactories(nullptr, lPrim, nullptr);
            if (!pEntry)
                return nullptr;

            if ((pEntry->m_lControl & ZRenderEntry::RE_CONSTRUCTION_FAILED) != 0)
            {
                ZUniMemory::Delete(pEntry);
                return nullptr;
            }

            ZASSERT(m_lRenderEntriesCount + 1u <= 0x8000u);
            m_apRenderEntries[m_lRenderEntriesCount++] = pEntry;
        }

        ZASSERT((pEntry->m_lControl & ZRenderEntry::RE_NOTIFIED) == 0);
        pEntry->m_lControl |= ZRenderEntry::RE_NOTIFIED | ZRenderEntry::RE_ADDTOREUSE;
        pEntry->m_pDrawArray = pDrawArray;
        SRenderEntryNotifyInfo notifyInfo{};
        pEntry->Notify(&notifyInfo);
        return pEntry;
    }

    ZRenderEntrySprite* ZRenderDraw::AddRenderEntrySprite(uint32_t lPrim)
    {
        ZRenderEntry* pEntry = m_pEntryReuse->GetAndRemove(lPrim);
        if (!pEntry)
        {
            pEntry = CreateRenderEntryFromFactories(nullptr, lPrim, nullptr);
            if (!pEntry)
                return nullptr;

            if ((pEntry->m_lControl & ZRenderEntry::RE_CONSTRUCTION_FAILED) != 0)
            {
                ZUniMemory::Delete(pEntry);
                return nullptr;
            }

            ZASSERT(m_lRenderEntriesCount + 1u <= 0x8000u);
            m_apRenderEntries[m_lRenderEntriesCount++] = pEntry;
        }
        else
        {
            ZASSERT(static_cast<ZRenderEntrySprite*>(pEntry)->m_lPrim == lPrim);
        }

        ZASSERT((pEntry->m_lControl & ZRenderEntry::RE_NOTIFIED) == 0);
        ZASSERT(pEntry->GetType() == ZRenderEntry::RT_SPRITE);
        pEntry->m_lControl |= ZRenderEntry::RE_NOTIFIED | ZRenderEntry::RE_ADDTOREUSE;
        SRenderEntryNotifyInfo notifyInfo{};
        pEntry->Notify(&notifyInfo);
        return static_cast<ZRenderEntrySprite*>(pEntry);
    }

    ZRenderEntry* ZRenderDraw::GetOrCreateRenderEntry(ZBaseGeom* pBaseGeom)
    {
        const uint16_t lDrawId = pBaseGeom->m_lDrawId;
        if (lDrawId != 0)
        {
            ZRenderEntry* pEntry = m_apRenderEntryLookup[lDrawId & 0x7FFFu];
            ZASSERT(pEntry);
            ZASSERT(pEntry->GetBaseGeom() == pBaseGeom);
            return pEntry;
        }

        ZRenderEntry* pEntry = CreateRenderEntryFromFactories(pBaseGeom, 0, nullptr);
        if (pEntry)
        {
            if ((pEntry->m_lControl & ZRenderEntry::RE_CONSTRUCTION_FAILED) == 0)
            {
                pEntry->m_lControl |= ZRenderEntry::RE_CREATEDTHISFRAME;

                const uint16_t lNewDrawId = static_cast<uint16_t>((m_RenderEntryIndex.New() + 1u) & 0x7FFFu);
                ZASSERT(lNewDrawId != 0);
                m_apRenderEntryLookup[lNewDrawId] = pEntry;
                pBaseGeom->m_lDrawId = lNewDrawId;

                ZASSERT(m_lRenderEntriesCount + 1u <= 0x8000u);
                m_apRenderEntries[m_lRenderEntriesCount++] = pEntry;
            }
            else
            {
                ZUniMemory::Delete(pEntry);
                pEntry = nullptr;
            }
        }

        if (pEntry)
        {
            ZASSERT(pEntry->GetBaseGeom() == pBaseGeom);
        }
        return pEntry;
    }

    uint32_t ZRenderDraw::CreateRenderEntries(
        ZRenderEntry** pRenderEntries,
        uint32_t lMaxNumEntries,
        ZVolumeList* pVolumeList,
        ZRenderEntryLists* pGeomList,
        ZRenderEntry* pObserverEntry,
        float* vObserver,
        float fLODScale)
    {
        (void)vObserver;
        (void)fLODScale;

        uint32_t lNumRenderEntries = 0;
        ZStackArray<1024, ZRenderEntry::ZAttachedBaseGeom> attachedGeoms;

        auto addEntry = [&](ZBaseGeom* pBaseGeom, const ZMatrix* pRootPosition, const ZBaseGeom* pEnvironment) -> ZRenderEntry* {
            if (!pBaseGeom)
                return nullptr;

            ZRenderEntry* pEntry = GetOrCreateRenderEntry(pBaseGeom);
            if (!pEntry)
                return nullptr;

            ZASSERT(lNumRenderEntries < lMaxNumEntries);
            pRenderEntries[lNumRenderEntries++] = pEntry;

            pBaseGeom->m_lControl |= 0x10000000u;
            if ((pEntry->m_lControl & ZRenderEntry::RE_NOTIFIED) == 0)
            {
                pEntry->m_lControl |= ZRenderEntry::RE_NOTIFIED;
                if (pRootPosition)
                    pEntry->SetObjectToWorldMatrix(*pRootPosition);
            }

            if (pEnvironment)
                pEntry->m_pEnvironment = pEnvironment;

            if (pGeomList)
                pGeomList->Add(reinterpret_cast<ZRenderEntryGeom*>(pEntry));

            if ((pEntry->m_lControl & ZRenderEntry::RE_HASBONES) != 0)
                pEntry->GetAttachedBaseGeoms(&attachedGeoms);
            return pEntry;
        };

        for (uint32_t i = 0; i < pVolumeList->m_lCount; ++i)
        {
            ZBaseGeomVolume* pVolume = pVolumeList->m_Volumes[i];
            ZBaseGeom* pBaseGeom = pVolume->m_pBaseGeom;
            if (!pBaseGeom)
                continue;

            if ((pBaseGeom->m_lControl & 0x10000000u) != 0)
            {
                if (pVolume->m_pBaseGeomEnvironment)
                {
                    const uint16_t lDrawId = pBaseGeom->m_lDrawId;
                    if (lDrawId && m_apRenderEntryLookup[lDrawId & 0x7FFFu])
                        m_apRenderEntryLookup[lDrawId & 0x7FFFu]->m_pEnvironment = pVolume->m_pBaseGeomEnvironment;
                }
                continue;
            }

            addEntry(pBaseGeom, &pVolume->m_RootPosition, pVolume->m_pBaseGeomEnvironment);
        }

        for (uint32_t i = 0; i < pVolumeList->m_lCount; ++i)
            pVolumeList->m_Volumes[i]->m_pBaseGeom->m_lControl &= ~0x10000000u;

        for (uint32_t i = 0; i < attachedGeoms.Count(); ++i)
        {
            ZRenderEntry::ZAttachedBaseGeom* pAttached = attachedGeoms.Get(i);
            ZBaseGeom* pBaseGeom = pAttached->m_pBaseGeom;
            if (!pBaseGeom || (pBaseGeom->m_lControl & 0x2C00u) != 0)
                continue;

            ZRenderEntry* pEntry = GetOrCreateRenderEntry(pBaseGeom);
            if (!pEntry)
                continue;

            ZASSERT(lNumRenderEntries < lMaxNumEntries);
            pRenderEntries[lNumRenderEntries++] = pEntry;

            if ((pEntry->m_lControl & ZRenderEntry::RE_NOTIFIED) == 0)
            {
                pEntry->m_lControl |= ZRenderEntry::RE_NOTIFIED;
                ZASSERT(pAttached->m_pOwner->GetType() == ZRenderEntry::RT_GEOM);

                if (pAttached->m_pOwner->m_pEnvironment)
                    pEntry->m_pEnvironment = pAttached->m_pOwner->m_pEnvironment;

                if (pEntry->GetType() != ZRenderEntry::RT_DEFORMER)
                {
                    pEntry->SetObjectToWorldMatrix(pAttached->m_pOwner->m_ObjectToWorldMatrix);
                }
            }

            if (pGeomList)
                pGeomList->Add(static_cast<ZRenderEntryGeom*>(pEntry));
        }

        return lNumRenderEntries;
    }

    void ZRenderDraw::UpdateBoneModifiers(ZRenderEntryLists* pLists)
    {
        ZStackArray<ELEMENTS_IN_RENDER_ENTRY_LIST_COUNT, ZRenderEntryGeom*> aGeneric, aOwners;
        auto* pList = pLists->GetList(ZRenderEntryLists::LISTTYPES::LT_BONES);

        if (!pList->IsEmpty())
        {
            for (int i = 0; i < pList->Count(); ++i)
            {
                auto* pEntry = *pList->Get(i);
                if ((pEntry->GetBaseGeom()->Control() & ZCOWNERDRAW) != 0)
                {
                    aOwners.Add(&pEntry);
                }
                else
                {
                    aGeneric.Add(pList->Get(i));
                }
            }
        }

        // Normal draw all generic entries as a single batch
        UpdateBoneModifiersList(aGeneric);

        if (!aOwners.IsEmpty())
        {
            // It's weird technique, but in general it's simple:
            // Remember original capacity
            const uint32_t lOriginalCap = aOwners.Count();
            uint32_t lStep = 0;

            do
            {
                // Force override capacity to 1
                aOwners.m_lNrEntries = 1;

                // Run process on single size list
                UpdateBoneModifiersList(aOwners);

                // Move to next element
                ++lStep;
                if (lStep < lOriginalCap)
                    aOwners.m_Array[0] = aOwners.m_Array[lStep];
            }
            while (lStep < lOriginalCap);
        }
    }

    bool ZRenderDraw::UpdateAttachedBaseGeomsPositions(ZRenderEntryBones* pOwnerEntry, bool bFirstPerson)
    {
        if (!pOwnerEntry || !pOwnerEntry->m_pBaseGeom)
            return false;

        auto* pOwnerGeom = pOwnerEntry->m_pBaseGeom->GetGeom();
        auto* pLinkObject = pOwnerGeom ? geom_cast<ZLNKOBJ>(pOwnerGeom) : nullptr;
        if (!pLinkObject || !pLinkObject->m_pBoneModify)
            return false;

        auto* pPrimControl = g_pRenderDll ? g_pRenderDll->m_pPrimControl : nullptr;
        const uint32_t lPrim = pOwnerEntry->m_lPrimId;
        if (!pPrimControl || !lPrim)
            return false;

        const ZBone* pBones = pOwnerEntry->GetBones();
        const float* pConvBones = pPrimControl->GetConvBones(lPrim);
        const uint8_t* pBoneIdToIndex = pPrimControl->GetBoneIdToIndexLookup(lPrim);
        const SBoneDefinition* pBoneDefinitions = pPrimControl->GetBoneDefinitions(lPrim);
        if (!pBones || !pConvBones || !pBoneIdToIndex || !pBoneDefinitions)
            return false;

        const uint32_t lActiveBones = pLinkObject->m_pBoneModify->m_lNumActiveBones;
        const auto getBoneIndex = [&](uint32_t lBoneId)
        {
            uint32_t lBoneIndex = pBoneIdToIndex[lBoneId];
            if (lBoneIndex == 0xFFu)
                return 0u;

            while (lBoneIndex >= lActiveBones)
            {
                lBoneIndex = pBoneDefinitions[lBoneIndex].lPrevBoneNr;
                if (lBoneIndex == 0xFFu)
                    return 0u;
            }
            return lBoneIndex;
        };

        const auto updateEntry = [&](ZBaseGeom* pBaseGeom, const ZMat3x3& mat, const ZVector3& pos,
                                     uint32_t lBoneId, bool bCorrectOwnerDraw)
        {
            if (!pBaseGeom || !pBaseGeom->m_lPrim || !pBaseGeom->m_lDrawId
                || (pBaseGeom->m_lControl & 0x2C00u) != 0)
                return;

            auto* pEntry = m_apRenderEntryLookup[pBaseGeom->m_lDrawId & 0x7FFFu];
            if (!pEntry)
                return;

            ZMat3x3 correctedMat = mat;
            ZVector3 correctedPos = pos;
            if (bCorrectOwnerDraw && pBaseGeom->GetGeom())
                pBaseGeom->GetGeom()->CorrectOwnerDrawMatrix(correctedMat, correctedPos,
                                                              pOwnerEntry->m_pBaseGeom, lBoneId);

            ZMatrix objectToWorld = pEntry->m_ObjectToWorldMatrix;
            objectToWorld.m0 = correctedMat;
            objectToWorld.p0 = correctedPos;
            pEntry->SetObjectToWorldMatrix(objectToWorld);
            const float fOwnerFade = pOwnerEntry->m_lFade == 0xFF
                ? 2.0f
                : static_cast<float>(pOwnerEntry->m_lFade) * 0.0039370079f;
            pEntry->m_lFade = static_cast<uint8_t>(fOwnerFade * 254.0f);
            pEntry->Update();
            pEntry->CopyAttachedRenderStateFrom(*pOwnerEntry);
            if ((pEntry->m_lControl & ZRenderEntry::RE_ATTACH_UPDATE) != 0)
                pEntry->AttachUpdate();

            if (auto* pAttachedBones = dynamic_cast<ZRenderEntryBones*>(pEntry))
            {
                if (pOwnerGeom->Is<ZLNKOBJ>())
                    pAttachedBones->SetBonesLightData(pOwnerEntry->GetBonesLightData());
            }
        };

        const auto updateAttached = [&](ZBaseGeom* pBaseGeom,
                                        const ZBoneModifyBase::ZAttachGeom* pAttached)
        {
            if (!pBaseGeom || !pAttached)
                return;

            const uint32_t lBoneIndex = getBoneIndex(pAttached->m_lBoneId);
            ZMat3x3 boneMat;
            ZVector3 bonePos;
            pLinkObject->m_pBoneModify->GetIKBone(pBones, pConvBones, lBoneIndex, boneMat, bonePos);

            ZMat3x3 ownerMat;
            ZVector3 ownerPos;
            pOwnerGeom->GetRootTM(ownerMat, ownerPos);
            vmmul(bonePos, ownerMat);
            bonePos += ownerPos;
            mmmul(boneMat, ownerMat);

            ZVector3 offset;
            vmmul(offset, pAttached->m_vOffset, boneMat);
            bonePos += offset;
            ZMat3x3 attachedMat;
            mmmul(attachedMat, pAttached->m_mOffset, boneMat);

            if (pBaseGeom->GetGeom())
                pBaseGeom->GetGeom()->CorrectOwnerDrawMatrix(attachedMat, bonePos,
                                                              pOwnerEntry->m_pBaseGeom,
                                                              pAttached->m_lBoneId);

            if (auto* pGroup = pBaseGeom->GetGeom() ? geom_cast<ZGROUP>(pBaseGeom->GetGeom()) : nullptr)
            {
                for (ZBaseGeom* pChild = pGroup->m_pGroupFirst; pChild; pChild = pChild->Next())
                {
                    auto* pChildGeom = pChild->GetGeom();
                    if (pChildGeom && pChildGeom->Is<ZGROUP>())
                        continue;

                    ZMat3x3 childMat;
                    ZVector3 childPos;
                    pChild->GetLocalMatPos(childMat, childPos);
                    vmmul(childPos, attachedMat);
                    childPos += bonePos;
                    mmmul(childMat, attachedMat);
                    updateEntry(pChild, childMat, childPos, pAttached->m_lBoneId, true);
                }
                return;
            }
            updateEntry(pBaseGeom, attachedMat, bonePos, pAttached->m_lBoneId, false);
        };

        uint32_t i = 0;
        while (i < pLinkObject->m_pBoneModify->m_AttachedGeoms.Count())
        {
            auto* pAttached = pLinkObject->m_pBoneModify->m_AttachedGeoms.Get(i);
            auto* pBaseGeom = ZGeomBuffer::Instance().GeomRefToBasePtr(pAttached->m_rBaseGeom);
            if (!pBaseGeom)
            {
                *pAttached = *pLinkObject->m_pBoneModify->m_AttachedGeoms.Get(
                    pLinkObject->m_pBoneModify->m_AttachedGeoms.Count() - 1);
                pLinkObject->m_pBoneModify->m_AttachedGeoms.Remove(
                    pLinkObject->m_pBoneModify->m_AttachedGeoms.Count() - 1);
                continue;
            }
            updateAttached(pBaseGeom, pAttached);
            ++i;
        }

        const auto updateGroupChildren = [&](ZGROUP* pGroup, auto&& updateGroupChildrenRef) -> void
        {
            for (ZBaseGeom* pChild = pGroup->m_pGroupFirst; pChild; pChild = pChild->Next())
            {
                auto* pChildGeom = pChild->GetGeom();
                if (!pChildGeom || pChildGeom->Is<ZGROUP>())
                {
                    if (pChildGeom && pChildGeom->Is<ZGROUP>())
                        updateGroupChildrenRef(static_cast<ZGROUP*>(pChildGeom), updateGroupChildrenRef);
                    continue;
                }

                ZMat3x3 childMat;
                ZVector3 childPos;
                pChild->GetRootTM(childMat, childPos);
                updateEntry(pChild, childMat, childPos, 0, false);
            }
        };

        for (uint32_t j = 0; j < pLinkObject->m_pBoneModify->m_ConnectedPhysics.Count(); ++j)
        {
            auto* pConnected = ZGEOM::RefToPtr(*pLinkObject->m_pBoneModify->m_ConnectedPhysics.Get(j));
            if (!pConnected)
                continue;

            auto* pConnectedBase = pConnected->BaseGeom();
            if (!pConnectedBase)
                continue;
            if (auto* pGroup = geom_cast<ZGROUP>(pConnected))
                updateGroupChildren(pGroup, updateGroupChildren);
            else
            {
                ZMat3x3 mat;
                ZVector3 pos;
                pConnected->GetRootTM(mat, pos);
                updateEntry(pConnectedBase, mat, pos, 0, false);
            }
        }

        if (!bFirstPerson)
            pLinkObject->m_pBoneModify->UpdateConnectedPhysics(pBones);

        // PC 0x4749FA, 0x474B0D and 0x474D9D transfer this typed
        // ZRenderEntry state immediately after each attached entry update.
        // Expected call at PC 0x474D9D: sub_473BE0(target, pOwnerEntry).
        (void)bFirstPerson;
        return true;
    }

    void ZRenderDraw::UpdateLightList(ZRenderEntryLists* pLists)
    {
        if (!pLists)
            return;

        auto* pLightList = pLists->GetList(ZRenderEntryLists::LT_LIGHT);
        for (uint32_t i = 0; i < pLightList->Count(); ++i)
        {
            auto* pEntry = *pLightList->Get(i);
            if (!pEntry)
                continue;

            auto* pBaseGeom = pEntry->GetBaseGeom();
            if (!pBaseGeom)
                continue;

            auto* pGeom = pBaseGeom->GetGeom();
            const bool bEnvironment = pGeom
                ? (pGeom->GetObjectId() & ZENVIRONMENT::m_Mask) == ZENVIRONMENT::m_Id
                : pBaseGeom->IsDerivedFromStdObj(ZENVIRONMENT::m_Id);

            if (!bEnvironment)
            {
                if ((pBaseGeom->m_lControl & ZCLIGHTCHANGED) != 0)
                {
                    pBaseGeom->LightNotifyPotentialDetachment(false);
                    pBaseGeom->UpdateLightListForLight();
                }
                else if (pGeom && (pGeom->m_lGeomControl & 0x8u) != 0)
                {
                    if (g_pSysInterface && g_pSysInterface->m_pEngineData
                        && g_pSysInterface->m_pEngineData->GetListUser())
                    {
                        g_pSysInterface->m_pEngineData->GetListUser()->NotifyAllMembers(pBaseGeom);
                    }
                }
            }

            if (pGeom)
                pGeom->m_lGeomControl = static_cast<uint16_t>(pGeom->m_lGeomControl & ~0x8u);
        }
    }

    void ZRenderDraw::UpdateBoneModifiersList(ZStackArray<ELEMENTS_IN_RENDER_ENTRY_LIST_COUNT, ZRenderEntryGeom*>& sList)
    {
        UpdateBoneModifiersListIK(sList);

        for (uint32_t i = 0; i < sList.Count(); ++i)
        {
            auto* pEntry = sList.Get(i) ? *sList.Get(i) : nullptr;
            auto* pBones = dynamic_cast<ZRenderEntryBones*>(pEntry);
            if (!pBones || !pBones->m_pBaseGeom)
                continue;

            auto* pGeom = pBones->m_pBaseGeom->GetGeom();
            if (!pGeom || !pGeom->Is<ZLNKOBJ>())
                continue;

            auto* pLinkObject = static_cast<ZLNKOBJ*>(pGeom);
            if (pLinkObject->m_pBoneModify && pLinkObject->m_Model && pLinkObject->m_Model->m_Bones)
            {
                pLinkObject->m_pBoneModify->UpdateGlobalIK(
                    pLinkObject->m_Model->m_Bones, pBones->m_lPrimId, pLinkObject);
            }
        }
    }

    void ZRenderDraw::UpdateBoneModifiersListIK(ZStackArray<ELEMENTS_IN_RENDER_ENTRY_LIST_COUNT, ZRenderEntryGeom*>& sList)
    {
        for (uint32_t i = 0; i < sList.Count(); ++i)
        {
            ZStackArray<ELEMENTS_IN_RENDER_ENTRY_LIST_COUNT, ZRenderEntryGeom*>::iterator it {
                reinterpret_cast<ZRenderEntryGeom***>(&sList.m_Array[i]) };
            UpdateBoneModifiersListIK(&it);
        }
    }

    void ZRenderDraw::UpdateBoneModifiersListIK(ZStackArray<ELEMENTS_IN_RENDER_ENTRY_LIST_COUNT, ZRenderEntryGeom*>::iterator* pIt)
    {
        if (!pIt || !pIt->current || !*pIt->current || !**pIt->current)
            return;

        UpdateBoneModifiersListIK_IMPL(pIt);
    }

    void ZRenderDraw::UpdateBoneModifiersListIK_IMPL(
        ZStackArray<ELEMENTS_IN_RENDER_ENTRY_LIST_COUNT, ZRenderEntryGeom*>::iterator* pIt)
    {
        if (!pIt || !pIt->current || !*pIt->current || !**pIt->current)
            return;

        auto* pEntry = **pIt->current;
        auto* pBonesEntry = dynamic_cast<ZRenderEntryBones*>(pEntry);
        if (!pBonesEntry || !pBonesEntry->m_pBaseGeom)
            return;

        auto* pGeom = pBonesEntry->m_pBaseGeom->GetGeom();
        auto* pLinkObject = pGeom ? geom_cast<ZLNKOBJ>(pGeom) : nullptr;
        if (!pLinkObject || !pLinkObject->m_pBoneModify || !pLinkObject->m_Model)
            return;

        auto* pModel = pLinkObject->m_Model;
        auto* pModifier = pLinkObject->m_pBoneModify;
        auto* pBones = pModel->m_Bones;
        if (!pBones)
            return;

        if (pModel->m_State)
            pBonesEntry->UpdateActiveNumBones();

        const uint32_t lPrim = pBonesEntry->m_lPrimId;
        if (pModel->m_BoneCount != static_cast<int>(pModifier->m_lNumActiveBones))
            pModel->m_BoneCount = pModifier->m_lNumActiveBones;

        const bool bDoAnimations = pModifier->DoAnimations();
        const bool bNeedsAnimation = pModel->m_Animated || pModel->m_State
            || (pModel->m_ActiveAnims[0].mode & 7) != 0
            || pModifier->m_fGlobalScale != 1.0f;

        if (bDoAnimations && bNeedsAnimation)
        {
            ZMat3x3 rootMat;
            ZVector3 rootPos;
            pLinkObject->GetRootTM(rootMat, rootPos);
            pBones[0]._Mat = rootMat;
            pBones[0]._Pos = rootPos;

            pModel->PrepareAnim();

            // Blend bones are the animation-space result produced by PrepareAnim.
            // Preserve the root transform above; the engine does the same before
            // evaluating human state and quaternion blending.
            if (pModel->m_BlendBones)
            {
                for (int i = 1; i < pModel->m_BoneCount; ++i)
                {
                    pBones[i]._Quat = pModel->m_BlendBones[i].m_Quat;
                    pBones[i]._Pos = pModel->m_BlendBones[i].m_Pos;
                }
            }

            if (pModel->m_State)
            {
                if (pModel->m_Poses.m_PoseIdx)
                    pModel->BlendOutPoseWeights();

                const ZHumanState state = *pModel->m_State;
                pModel->AnimateState(g_pSysInterface->m_pEngineData->m_AnimationManager,
                                     g_pSysInterface->DeltaFrameTime);
                if (pModel->m_Animated)
                {
                    if (pModel->m_BoneCount > 70)
                    {
                        Animation::ZAngelBone angelPose;
                        pModel->LookAt(&angelPose,
                                       g_pSysInterface->m_pEngineData->m_AnimationManager,
                                       g_pSysInterface->DeltaFrameTime);
                        pModel->Bank(g_pSysInterface->DeltaFrameTime);
                        pModel->StateFit(&angelPose);
                    }
                }
                *pModel->m_State = state;
            }

            pModel->AnimateQuats(g_pSysInterface->m_pEngineData->m_AnimationManager);
            pModel->BlendQuats();

            if (pModel->m_PoseWeights && pModel->m_Poses.m_PoseIdx)
                pModel->PoseRotationAndTranslation();

            pBones[0]._Mat.Reset();
            pBones[0]._Pos.Reset();
            pModel->ModelSpaceBones();
            pModifier->UpdateGlobalIK(pBones, lPrim, pLinkObject);
        }
        else
        {
            // Even when animation is stopped, constraints and active physics must
            // still be reflected in the render pose.
            pModel->ModelSpaceBones();
            pModifier->UpdateGlobalIK(pBones, lPrim, pLinkObject);
        }

        UpdateAttachedBaseGeomsPositions(pBonesEntry, false);

    }
}
