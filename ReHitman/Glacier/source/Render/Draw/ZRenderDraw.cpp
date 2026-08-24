#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <Glacier/Render/Entry/ZRenderEntry.h>
#include <Glacier/Render/Entry/ZRenderEntryGeom.h>
#include <Glacier/Render/Entry/ZRenderEntrySprite.h>
#include <Glacier/Render/Entry/SRenderEntryNotifyInfo.h>
#include <Glacier/Render/Material/ZRenderMaterialBuffer.h>
#include <Glacier/Render/Material/ZRenderMaterialInstance.h>
#include <Glacier/Render/Object/ZRenderObject.h>
#include <Glacier/Render/Object/ZRenderObjectInstance.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/IK/ZLNKOBJ.h>
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

        // TODO: Finish this place after ZLNKOBJ shadow-caster validation will be reversed.
        // Reference (PC 0x464870):
        // ZGEOM* pGeom = pBaseGeom->m_pExtraGeom;
        // const bool bIsLnkObj = pGeom
        //     ? (pGeom->GetObjectId() & ZLNKOBJ::m_Mask) == ZLNKOBJ::m_Id
        //     : pBaseGeom->IsDerivedFromStdObj(ZLNKOBJ::m_Id);
        // if (!bIsLnkObj) return false;

        const uint16_t lDrawId = pBaseGeom->m_lDrawId;
        const ZRenderEntry* pEntry = m_apRenderEntryLookup[lDrawId];
        if (!pEntry)
        {
            return false;
        }

        // TODO: Finish this place after ZRenderEntry shadow-caster distance will be reversed.
        // Reference (PC): return *(float*)((char*)pEntry + 0xA4) <= 2000.0f;
        return false;
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

        // TODO: Finish this place after ZRenderEntryBones will be reversed.
        // Reference (PC 0x477C80, ZRenderEntryBones::GetBoneArray):
        // const uint32_t lNumBones = *(uint32_t*)((const char*)pEntry + 0xA8);
        // return lNumBones ? (const ZBone*)(*(uintptr_t*)((const char*)pEntry + 0x9C) + 16 * (3 * lNumBones + 9)) : nullptr;
        return nullptr;
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
                // TODO: Finish this place after ZRenderEntryGeom will be reversed.
                // Reference (PC): *(float*)((char*)pEntry + 0x88) = fTextureFrameNumber;
            }
        }
    }

    SRenderEntryInstance* ZRenderDraw::CreateRenderEntryInstance(const ZPrimHandle& hPrim, ZRenderEntry* pEntry, ZBaseGeom* pBaseGeom, bool bUniqueRenderObject)
    {
        ZRenderObject* pRenderObject = nullptr;

        if (bUniqueRenderObject)
        {
            // TODO: Finish this place after ZRenderMaterialBuffer will be reversed.
            // Reference (PC 0x476520):
            // SPrimInfo* pPrimInfo = hPrim.Get<SPrimInfo>();
            // uint32_t lMaterialId = pPrimInfo->m_kind == 2
            //     ? g_pMaterialBufferInstance->CreateMaterialInstanceSprite(pPrimInfo->m_field8, pPrimInfo->m_field10, pPrimInfo->m_byteC)
            //     : static_cast<uint16_t>(pPrimInfo->m_word0 /* iMaterialId */);
            // ZRenderMaterialInstance* pMaterial = g_pMaterialBufferInstance->GetMaterialInstance(lMaterialId);
            // if (pMaterial)
            // {
            //     pRenderObject = pMaterial->CreateRenderObject(hPrim);
            //     if (pRenderObject)
            //     {
            //         pRenderObject->m_lFlags |= 2;
            //     }
            // }
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
                // TODO: Finish this place after ZRenderMaterialBuffer will be reversed.
                // Reference (PC 0x476520):
                // SPrimInfo* pPrimInfo = hPrim.Get<SPrimInfo>();
                // uint32_t lMaterialId = pPrimInfo->m_kind == 2
                //     ? g_pMaterialBufferInstance->CreateMaterialInstanceSprite(...)
                //     : static_cast<uint16_t>(pPrimInfo->m_word0);
                // ZRenderMaterialInstance* pMaterial = g_pMaterialBufferInstance->GetMaterialInstance(lMaterialId);
                // if (pMaterial)
                // {
                //     pRenderObject = pMaterial->CreateRenderObject(hPrim);
                //     if (pRenderObject)
                //     {
                //         m_RenderObjects[hPrim.m_lHandleValue] = pRenderObject;
                //     }
                // }
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
        // TODO: Finish this place after ZRenderEntryBones and ZRenderDrawD3D::CalcBoneLightSources will be reversed.
        // Reference (PC 0x4744C0):
        // ZBaseGeom* pBaseGeom = pRenderEntryBones->GetBaseGeom();
        // ... validates ZLNKOBJ, then calls the pure virtual
        // CalcBoneLightSources(pBaseGeom, pDirectLights) (vtbl[47]).
    }

    ZRenderEntry* ZRenderDraw::AddRenderEntryArray(uint32_t lPrim, const SDrawArray* pDrawArray)
    {
        // TODO: Finish this place after ZRenderEntry factory and reuse pool will be reversed.
        // Reference (PC 0x4741D0):
        // ZRenderEntry* pEntry = <reuse-pool extract>(lPrim);
        // if (!pEntry)
        // {
        //     pEntry = CreateRenderEntryFromFactories(nullptr, lPrim, nullptr);
        //     if (!pEntry) return nullptr;
        //     if (pEntry->m_lGeomListsControl & 0x10) { delete pEntry; return nullptr; }
        //     ZASSERT(m_lRenderEntriesCount + 1 <= 0x8000);
        //     m_apRenderEntries[m_lRenderEntriesCount++] = pEntry;
        // }
        // ZASSERT((pEntry->m_lControl & ZRenderEntry::RE_NOTIFIED) == 0);
        // pEntry->m_lControl |= ZRenderEntry::RE_NOTIFIED | ZRenderEntry::RE_ADDTOREUSE;
        // pEntry->m_pDrawArray = pDrawArray;
        // SRenderEntryNotifyInfo notifyInfo = {};
        // pEntry->Notify(&notifyInfo);
        // return pEntry;
        return nullptr;
    }

    ZRenderEntrySprite* ZRenderDraw::AddRenderEntrySprite(uint32_t lPrim)
    {
        // TODO: Finish this place after ZRenderEntry factory and reuse pool will be reversed.
        // Reference (PC 0x4740E0):
        // ZRenderEntry* pEntry = <reuse-pool extract>(lPrim);
        // if (!pEntry)
        // {
        //     pEntry = CreateRenderEntryFromFactories(nullptr, lPrim, nullptr);
        //     if (!pEntry) return nullptr;
        //     if (pEntry->m_lGeomListsControl & 0x10) { delete pEntry; return nullptr; }
        //     ZASSERT(m_lRenderEntriesCount + 1 <= 0x8000);
        //     m_apRenderEntries[m_lRenderEntriesCount++] = pEntry;
        // }
        // else
        // {
        //     ZASSERT(((ZRenderEntrySprite*)pEntry)->m_lPrim == lPrim);
        // }
        // ZASSERT((pEntry->m_lControl & ZRenderEntry::RE_NOTIFIED) == 0);
        // ZASSERT(pEntry->GetType() == ZRenderEntry::RT_SPRITE);
        // pEntry->m_lControl |= ZRenderEntry::RE_NOTIFIED | ZRenderEntry::RE_ADDTOREUSE;
        // SRenderEntryNotifyInfo notifyInfo = {};
        // pEntry->Notify(&notifyInfo);
        // return (ZRenderEntrySprite*)pEntry;
        return nullptr;
    }
}
