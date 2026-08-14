#include <Glacier/Render/Material/ZRenderMaterialResourceD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialBufferD3D.h>
#include <Glacier/Render/Material/ZRenderMaterialInstance.h>
#include <Glacier/Render/Material/ZRenderMaterialSubClass.h>
#include <Glacier/Render/Material/ZRenderMaterialLayer.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderList.h>
#include <Glacier/Render/Material/ZRenderMaterialClass.h>
#include <Glacier/Render/Material/SMATHeader.h>
#include <Glacier/Render/ZRenderBinderTexture.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/ZTextureManagerD3D.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZRPropertyReader.h>
#include <Glacier/Render/SRMaterialProperties.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ReGlacier.h>
#include <cstdlib>
#include <cstring>


namespace Glacier
{
    namespace
    {
        // qsort comparator over ZRenderMaterialInstance*: by material sorting value,
        // then by material class index, then by subclass index.
        int __cdecl CmpRenderMaterialInstances2(const void* pLhs, const void* pRhs)
        {
            const auto* pInstance1 = *static_cast<const ZRenderMaterialInstance* const*>(pLhs);
            const auto* pInstance2 = *static_cast<const ZRenderMaterialInstance* const*>(pRhs);

            uint32_t lValue1 = pInstance1->m_pMaterialProperties->lMaterialSortingValue;
            uint32_t lValue2 = pInstance2->m_pMaterialProperties->lMaterialSortingValue;

            if (lValue1 == lValue2)
            {
                const auto* pSubClass1 = pInstance1->m_pMaterialSubClass;
                const auto* pSubClass2 = pInstance2->m_pMaterialSubClass;

                lValue1 = pSubClass1->m_pMaterialClass->m_lIndex;
                lValue2 = pSubClass2->m_pMaterialClass->m_lIndex;

                if (lValue1 == lValue2)
                {
                    return pSubClass1->m_lIndex - pSubClass2->m_lIndex;
                }
            }

            return static_cast<int>(lValue1 - lValue2);
        }
    }

    ZRenderMaterialBufferD3D::~ZRenderMaterialBufferD3D()
    {
        if (m_bResourcesAllocated)
        {
            FreeSubClasses();
            if (ZSharedResourcesD3D::g_pInstance)
            {
                ZSharedResourcesD3D::g_pInstance->m_pResourceFX->FreeResources();
            }

            m_bResourcesAllocated = false;
        }
    }

    ZRenderMaterialBufferD3D::ZRenderMaterialBufferD3D()
        : ZRenderMaterialBuffer()
        , m_lNumMaterialClasses(0)
        , m_lNumMaterialInstances(0)
        , m_bResourcesAllocated(false)
    {
        memset(m_pMaterialInstances, 0, sizeof(m_pMaterialInstances));
        memset(m_pMaterialClasses, 0, sizeof(m_pMaterialClasses));
    }

    void ZRenderMaterialBufferD3D::AllocateResources()
    {
        if (m_pBuffer && !m_bResourcesAllocated)
        {
            CreateMaterials();
            m_bResourcesAllocated = true;
        }
    }

    void ZRenderMaterialBufferD3D::FreeResources()
    {
        if (!m_bResourcesAllocated)
            return;

        FreeSubClasses();
        if (ZSharedResourcesD3D::g_pInstance)
        {
            ZSharedResourcesD3D::g_pInstance->m_pResourceFX->FreeResources();
        }

        m_bResourcesAllocated = false;
    }

    void ZRenderMaterialBufferD3D::FreeSubClasses()
    {
        for (int i = MAX_MATERIAL_INSTANCES_NR - 1; i != -1; --i)
        {
            if (!m_pMaterialInstances[i])
                continue;

            ZUniMemory::Delete(m_pMaterialInstances[i]);
            m_pMaterialInstances[i] = nullptr;
        }

        for (int i = MAX_MATERIAL_CLASSES_NR - 1; i != -1; --i)
        {
            if (!m_pMaterialClasses[i])
                continue;

            ZUniMemory::Delete(m_pMaterialClasses[i]);
            m_pMaterialClasses[i] = nullptr;
        }
    }

    ZRenderMaterialInstance* ZRenderMaterialBufferD3D::GetMaterialInstance(uint32_t lMaterialId)
    {
        ZASSERT(lMaterialId < MAX_MATERIAL_INSTANCES_NR);
        return m_pMaterialInstances[lMaterialId];
    }

    uint32_t ZRenderMaterialBufferD3D::CreateMaterialInstanceSprite(uint32_t lTextureId, uint32_t lDrawMode, uint32_t lSpriteType)
    {
        // lTextureId and lDrawMode are passed by callers but unused in the PC implementation.
        // Despite the name, the original only looks up an already created sprite material
        // instance with the requested sprite type and returns its index. A match at index 0
        // is indistinguishable from a miss: 0 is returned in both cases.
        for (uint32_t i = 0; i < MAX_MATERIAL_INSTANCES_NR; ++i)
        {
            ZRenderMaterialInstance* pInstance = m_pMaterialInstances[i];
            if (!pInstance)
            {
                continue;
            }

            ZRenderMaterialSubClass* pSubClass = pInstance->m_pMaterialSubClass;
            if (pSubClass->m_lObjectType == 2 && pSubClass->m_lObjectSubType == lSpriteType) // object type 2 = SPRITES
            {
                return i;
            }
        }

        return 0u;
    }

    uint32_t ZRenderMaterialBufferD3D::GetTextureFromMaterial(uint32_t lMaterialId, const char* pszTextureBinderName)
    {
        ZRenderMaterialInstance* pInstance = GetMaterialInstance(lMaterialId);
        if (!pInstance)
        {
            return 0u;
        }

        // The original walks m_pLayers and m_pMaterialBinderList in parallel (offsets 0xC..0x54).
        for (int i = 0; i < ZRenderMaterialSubClass::MAX_LAYERS_NR; ++i)
        {
            ZRenderMaterialLayer* pLayer = pInstance->m_pMaterialSubClass->m_pLayers[i];
            if (!pLayer || !pLayer->NumPasses())
            {
                continue;
            }

            // No null check in the original: a layer with passes always has its binder list.
            ZRenderMaterialBinderList* pBinderList = pInstance->m_pMaterialBinderList[i];
            ZRenderBinderList* pBinders = &pBinderList->m_BindersMaterial;

            for (uint32_t j = 0; j < pBinders->m_lNumBinders; ++j)
            {
                ZRenderBinder* pBinder = pBinders->m_pBinders[j];
                if (pBinder->m_lBinderType == 2 && !strcmp(pBinder->m_pszName, pszTextureBinderName)) // binder type 2 = texture
                {
                    return static_cast<ZRenderBinderTexture*>(pBinder)->GetTextureId();
                }
            }
        }

        return 0u;
    }

    uint32_t ZRenderMaterialBufferD3D::GetAllTexturesFromMaterial(uint32_t lMaterial, uint32_t* ppTextures, uint32_t lMaxTexturesNr)
    {
        ZRenderMaterialInstance* pInstance = GetMaterialInstance(lMaterial);
        if (!pInstance)
        {
            return 0u;
        }

        uint32_t lTexturesNr = 0u;

        // The original scans the root property list for every 'BIND' list (there can be
        // several of them): once one BIND list is exhausted, the root scan continues.
        auto* pRootProperty = fuck_cast<ZRPropertyReader::SProperty>(GetData(pInstance->m_pMaterialProperties->lNameOffset));

        for (uint32_t i = 0; i < pRootProperty->lSize; ++i)
        {
            ZASSERT(pRootProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_LIST);
            ZASSERT(i < pRootProperty->lSize);

            const auto* pRootElements = static_cast<const ZRPropertyReader::SProperty*>(GetData(pRootProperty->lData));
            const auto* pBindProperty = &pRootElements[i];

            // Empty BIND lists are skipped too.
            if (pBindProperty->lName != 'BIND' || !pBindProperty->lSize)
            {
                continue;
            }

            const auto* pBindElements = static_cast<const ZRPropertyReader::SProperty*>(GetData(pBindProperty->lData));

            for (uint32_t j = 0; j < pBindProperty->lSize; ++j)
            {
                ZASSERT(pBindProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_LIST);
                ZASSERT(j < pBindProperty->lSize);

                ZRPropertyReader sElement{};
                sElement.m_pBuffer = this;
                sElement.m_pProperty = const_cast<ZRPropertyReader::SProperty*>(&pBindElements[j]);

                ZRPropertyReader sEnabElem{};
                sElement.GetNamedListElement('ENAB', sEnabElem);
                ZASSERT(sEnabElem.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_UINT32);

                // lSize == 1: value is stored inline in lData, otherwise lData is a buffer offset.
                const uint32_t* pEnabData = (sEnabElem.m_pProperty->lSize == 1)
                    ? &sEnabElem.m_pProperty->lData
                    : static_cast<const uint32_t*>(sEnabElem.m_pBuffer->GetData(sEnabElem.m_pProperty->lData));

                // The enabled flag is checked before the 'TEXT' name in the original.
                if (*pEnabData && sElement.m_pProperty->lName == 'TEXT')
                {
                    ZRPropertyReader sTxidElem{};
                    sElement.GetNamedListElement('TXID', sTxidElem);
                    ZASSERT(sTxidElem.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_UINT32);

                    const uint32_t* pTxidData = (sTxidElem.m_pProperty->lSize == 1)
                        ? &sTxidElem.m_pProperty->lData
                        : static_cast<const uint32_t*>(sTxidElem.m_pBuffer->GetData(sTxidElem.m_pProperty->lData));

                    ppTextures[lTexturesNr] = *pTxidData;
                    ++lTexturesNr;

                    if (lTexturesNr == lMaxTexturesNr)
                    {
                        return lTexturesNr;
                    }
                }
            }
        }

        return lTexturesNr;
    }

    uint32_t ZRenderMaterialBufferD3D::GetTextureAnimData(uint32_t lMaterialId)
    {
        uint32_t aTextures[64] {0u};
        uint32_t lResult { 0u };
        const uint32_t lTexturesNr = GetAllTexturesFromMaterial(lMaterialId, aTextures, STATIC_ARR_LEN(aTextures));

        for (int i = 0; i < lTexturesNr; ++i)
        {
            ZTextureD3D* pTexture = g_pRenderDll->m_pTexCon->GetTexture(aTextures[i], 0u);
            if (!pTexture)
            {
                continue;
            }

            const auto* pAnimData = pTexture->m_pAnimData;
            if (pAnimData)
            {
                // ??? Need investigate what is pAnimData
                uint32_t lUnknownEntry = *pAnimData;
                if (lResult < lUnknownEntry)
                {
                    lResult = lUnknownEntry;
                }
            }
        }

        return lResult;
    }

    uint32_t ZRenderMaterialBufferD3D::NumMaterialInstances() const
    {
        return m_lNumMaterialInstances;
    }

    void ZRenderMaterialBufferD3D::CreateMaterials()
    {
        // The MAT buffer starts with a header followed by offset tables and
        // SRMaterialProperties records. All offsets are relative to the buffer start.
        const auto* pHeader = static_cast<const SMATHeader*>(GetData(0));
        const auto* pClassOffsets = static_cast<const uint32_t*>(GetData(pHeader->lClassTableOffset));
        const auto* pInstanceOffsets = static_cast<const uint32_t*>(GetData(pHeader->lInstanceTableOffset));

        // Offset tables are 1-based: slot 0 is unused and the first zero offset terminates the table.
        uint32_t lClassIndex = 1;
        while (true)
        {
            const uint32_t lClassOffset = pClassOffsets[lClassIndex];
            if (!lClassOffset)
            {
                break;
            }

            const auto* pProperties = static_cast<const SRMaterialProperties*>(GetData(lClassOffset));
            ZRenderMaterialClass* pMaterialClass = CreateMaterialClass(pProperties->lMaterialClassType, pProperties);
            m_pMaterialClasses[lClassIndex] = pMaterialClass;
            pMaterialClass->m_lIndex = lClassIndex;

            ++lClassIndex;
            if (lClassIndex >= MAX_MATERIAL_CLASSES_NR)
            {
                break;
            }
        }
        m_lNumMaterialClasses = lClassIndex - 1;

        // The original probes a single instance table slot (the one right after the last
        // created class) to pick the progress divider: it becomes MAX_MATERIAL_INSTANCES_NR
        // when the slot is occupied and stays 0 otherwise. The probed slot does not advance
        // inside the loop - this quirk is preserved on purpose.
        uint32_t lInstancesDivider = 0;
        while (pInstanceOffsets[lClassIndex])
        {
            lInstancesDivider += 4;
            if (lInstancesDivider >= MAX_MATERIAL_INSTANCES_NR)
            {
                break;
            }
        }

        uint32_t lInstanceIndex = 1;
        do
        {
            const uint32_t lInstanceOffset = pInstanceOffsets[lInstanceIndex];
            if (!lInstanceOffset)
            {
                break;
            }

            const auto* pProperties = static_cast<const SRMaterialProperties*>(GetData(lInstanceOffset));
            ZRenderMaterialClass* pMaterialClass = m_pMaterialClasses[pProperties->lMaterialClassIndex];
            if (!pMaterialClass)
            {
                ZASSERT(false);
            }

            m_pMaterialInstances[lInstanceIndex] = pMaterialClass->CreateMaterialInstance(pProperties, lInstanceIndex);

            const float fPercent = static_cast<float>(
                static_cast<double>(lInstanceIndex) / static_cast<double>(lInstancesDivider) * 0.099999994f + 0.25f);
            g_pEngineData->SetAllocSequencePercent(AS_TEXTURE, nullptr, fPercent);

            ++lInstanceIndex;
        }
        while (lInstanceIndex < MAX_MATERIAL_INSTANCES_NR);
        m_lNumMaterialInstances = lInstanceIndex - 1;

        ZRenderMaterialInstance* apSortedInstances[MAX_MATERIAL_INSTANCES_NR];
        uint32_t lNumSortedInstances = 0;

        m_lNumFullyOpaqueMaterials = 0;
        m_lNumOpaqueMaterials = 0;
        m_lNumTransparentMaterials = 0;

        for (int i = 0; i < MAX_MATERIAL_INSTANCES_NR - 1; ++i)
        {
            ZRenderMaterialInstance* pInstance = m_pMaterialInstances[i + 1];
            if (pInstance)
            {
                apSortedInstances[lNumSortedInstances++] = pInstance;
            }
        }

        qsort(apSortedInstances, lNumSortedInstances, sizeof(ZRenderMaterialInstance*), CmpRenderMaterialInstances2);

        for (uint32_t i = 0; i < lNumSortedInstances; ++i)
        {
            apSortedInstances[i]->m_lRemapValue = static_cast<uint16_t>(i);
        }

        // Sorting value ranges: [0, 0xF] - fully opaque, (0xF, 0x5F] - opaque, above - transparent.
        m_lNumFullyOpaqueMaterials = 0;
        if (lNumSortedInstances)
        {
            uint32_t lCount = 0;
            do
            {
                lCount = m_lNumFullyOpaqueMaterials;
                if (apSortedInstances[lCount]->m_pMaterialProperties->lMaterialSortingValue > 0xF)
                {
                    break;
                }

                m_lNumFullyOpaqueMaterials = lCount + 1;
            }
            while (lCount + 1 < lNumSortedInstances);
        }

        if (m_lNumFullyOpaqueMaterials)
        {
            --m_lNumFullyOpaqueMaterials;
        }

        m_lNumOpaqueMaterials = 0;
        if (lNumSortedInstances)
        {
            uint32_t lCount = 0;
            do
            {
                lCount = m_lNumOpaqueMaterials;
                if (apSortedInstances[lCount]->m_pMaterialProperties->lMaterialSortingValue > 0x5F)
                {
                    break;
                }

                m_lNumOpaqueMaterials = lCount + 1;
            }
            while (lCount + 1 < lNumSortedInstances);
        }

        if (m_lNumOpaqueMaterials)
        {
            --m_lNumOpaqueMaterials;
        }
    }

    ZRenderMaterialClass* ZRenderMaterialBufferD3D::CreateMaterialClass(uint32_t lMaterialClass, const SRMaterialProperties* pProperties)
    {
        // TODO: Finish me after ZRenderMaterialClassD3D::GetFactory()  will be reversed
        return nullptr;
    }
}