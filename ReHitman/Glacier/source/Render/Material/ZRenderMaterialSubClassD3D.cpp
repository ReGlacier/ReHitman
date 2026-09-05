#include <Glacier/Render/Material/ZRenderMaterialSubClassD3D.h>
#include <Glacier/Render/Material/ZRenderMaterialInstanceD3D.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderList.h>
#include <Glacier/Render/Material/ZRenderMaterialLayer.h>
#include <Glacier/Render/Object/ZRenderObjectInstance.h>
#include <Glacier/Render/Entry/ZRenderEntry.h>
#include <Glacier/Render/SRMaterialProperties.h>
#include <Glacier/Render/ZRPropertyReader.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Filesystem/IBuffer.h>


namespace Glacier
{
    ZRenderMaterialSubClassD3D::ZRenderMaterialSubClassD3D(const char* pszName, ZRenderMaterialClass* pMaterialClass, uint16_t lObjectType, uint16_t lObjectSubType, uint32_t lMaterialFlags)
        : ZRenderMaterialSubClass(pszName, pMaterialClass, lObjectSubType, lObjectSubType, lMaterialFlags)
        , m_lNumBinderValidators(0u)
    {
    }

    ZRenderMaterialSubClassD3D::~ZRenderMaterialSubClassD3D() = default;

    void ZRenderMaterialSubClassD3D::Draw(const ZRenderMaterialInstance* pMaterial, ZRenderObjectInstance** ppObjects, uint32_t lObjectsNr, ZRenderContext* pCtx)
    {
        auto* pCurrentLayer = m_pLayers[m_lLayer];
        auto* pBinderList = pMaterial->m_pMaterialBinderList[m_lLayer];

        for (int i = 0; i < lObjectsNr; ++i)
        {
            auto* pObject = ppObjects[i];
            auto* pRenderEntry = pObject->m_pRenderEntry;
            
            pCtx->m_pRenderObjectInstance = pObject;

            const auto lRepeatCount = pRenderEntry->GetInstanceRepeat(pObject);
            if (!lRepeatCount) continue;

            for (int lPassIdx = 0; lPassIdx < lRepeatCount; ++lPassIdx)
            {
                pCtx->m_nCurrentPass = lPassIdx;

                pRenderEntry->SetRenderContext(pCtx, pObject);
                pCurrentLayer->BeginObject(pMaterial, pBinderList, pObject, pCtx);
                pObject->Draw(pCtx);
                pCurrentLayer->EndObject();
            }
        }
    }
    
    ZRenderMaterialInstance* ZRenderMaterialSubClassD3D::CreateMaterialInstance(const SRMaterialProperties* pProperties, uint32_t lMaterialId)
    {
        // Two-stage name accessor (weird)
        ZRPropertyReader sNameReader{};
        ZRPropertyReader sNameElem{};
        
        sNameReader.m_pBuffer = reinterpret_cast<IBuffer*>(g_pRenderDll->m_pMaterialBuffer);
        sNameReader.m_pProperty = fuck_cast<ZRPropertyReader::SProperty>(sNameReader.m_pBuffer->GetData(pProperties->lNameOffset));

        sNameReader.GetNamedListElement('NAME', sNameElem);
        ZASSERT(sNameElem.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_CHAR);

        const char* pszMaterialName = static_cast<const char*>(sNameElem.m_pBuffer->GetData(sNameElem.m_pProperty->lData));

        auto* pMaterialInstance = ZUniMemory::New<ZRenderMaterialInstanceD3D>(
            pszMaterialName, 
            static_cast<ZRenderMaterialSubClass*>(this), 
            const_cast<SRMaterialProperties*>(pProperties), 
            lMaterialId
        );
        ZASSERT(pMaterialInstance);

        // Lookup for free slot
        for (int i = 0; i < MAX_LAYERS_NR; ++i)
        {
            ZRenderMaterialLayer* pLayer = m_pLayers[i];
            if (!pLayer)
            {
                continue;
            }

            pMaterialInstance->m_lLayerMask |= (1 << i);

            auto* pBinderList = ZUniMemory::New<ZRenderMaterialBinderList>();
            ZASSERT(pBinderList);

            // Store binder list
            ZASSERT(pMaterialInstance->m_pMaterialBinderList[i] == nullptr);
            pMaterialInstance->m_pMaterialBinderList[i] = pBinderList;

            // Initialize binder list
            pLayer->CreateBinders(i, pMaterialInstance, pBinderList);
        }

        // Done
        return pMaterialInstance;
    }

    bool ZRenderMaterialSubClassD3D::CanCreateMaterialInstance(const SRMaterialProperties* pMatProps)
    {
        ZASSERT(pMatProps);
        if (pMatProps->lObjectType != m_lObjectType || pMatProps->lObjectSubType != m_lObjectSubType)
        {
            return false;
        }

        ZRPropertyReader sRootReader{};
        ZRPropertyReader sBindList{};

        sRootReader.m_pBuffer = reinterpret_cast<IBuffer*>(g_pRenderDll->m_pMaterialBuffer);
        sRootReader.m_pProperty = fuck_cast<ZRPropertyReader::SProperty>(sRootReader.m_pBuffer->GetData(pMatProps->lNameOffset));
        sRootReader.GetNamedListElement('BIND', sBindList);

        const auto* pBindProperty = sBindList.m_pProperty;
        if (!pBindProperty || pBindProperty->lSize == 0)
        {
            return true;
        }

        for (uint32_t i = 0; i < pBindProperty->lSize; ++i)
        {
            ZASSERT(pBindProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_LIST);
            ZASSERT(i < pBindProperty->lSize);

            ZRPropertyReader sElement{};
            sElement.m_pBuffer = sBindList.m_pBuffer;
            
            const auto* pElementsArray = static_cast<const ZRPropertyReader::SProperty*>(sBindList.m_pBuffer->GetData(pBindProperty->lData));
            sElement.m_pProperty = const_cast<ZRPropertyReader::SProperty*>(&pElementsArray[i]);

            // Is enabled?
            ZRPropertyReader sEnabElem{};
            sElement.GetNamedListElement('ENAB', sEnabElem);
            ZASSERT(sEnabElem.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_UINT32);

            // lSize == 1: value is stored inline in lData, otherwise lData is a buffer offset.
            uint32_t bEnabled =
                (sEnabElem.m_pProperty->lSize == 1u)
                    ? sEnabElem.m_pProperty->lData
                    : *static_cast<const uint32_t*>(sEnabElem.m_pBuffer->GetData(sEnabElem.m_pProperty->lData));

            if (!bEnabled)
            {
                // Disabled - continue
                continue;
            }

            bool bValidationSuccess = false;

            // If enabled presented as BOOL
            if (sElement.m_pProperty->lName == 'BOOL')
            {
                ZRPropertyReader sNameElem{};
                sElement.GetNamedListElement('NAME', sNameElem);
                ZASSERT(sNameElem.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_CHAR);

                const char* pszBinderName = static_cast<const char*>(sNameElem.m_pBuffer->GetData(sNameElem.m_pProperty->lData));

                ZRPropertyReader sValuElem{};
                sElement.GetNamedListElement('VALU', sValuElem);
                if (sValuElem.m_pProperty->lType != ZRPropertyReader::PROPERTY_TYPE::PT_UINT32)
                {
                    // Expected to have U32 here
                    continue;
                }

                uint32_t lVal = (sValuElem.m_pProperty->lSize == 1u)
                    ? sValuElem.m_pProperty->lData
                    : *static_cast<const uint32_t*>(sValuElem.m_pBuffer->GetData(sValuElem.m_pProperty->lData));

                const bool bValue = (lVal != 0);
                if (m_lNumBinderValidators == 0)
                {
                    return false;
                }

                for (uint32_t v = 0; v < m_lNumBinderValidators; ++v)
                {
                    if (strcmp(m_BinderValidators[v].m_pszBinderName, pszBinderName) == 0)
                    {
                        if (m_BinderValidators[v].m_bBinderValue == bValue)
                        {
                            bValidationSuccess = true;
                        }
                        break;
                    }
                }
            }
            else if (sElement.m_pProperty->lName == 'RSTA') // Check by RenderState (RSTA)
            {
                ZRPropertyReader sBenaElem{};
                sElement.GetNamedListElement('BENA', sBenaElem);
                ZASSERT(sBenaElem.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_UINT32);

                uint32_t lBenaVal = (sBenaElem.m_pProperty->lSize == 1u)
                    ? sBenaElem.m_pProperty->lData
                    : *static_cast<const uint32_t*>(sBenaElem.m_pBuffer->GetData(sBenaElem.m_pProperty->lData));

                bool bBlendEnabled = (lBenaVal != 0);

                // Lookup for "BlendEnabled", not for empty ass
                if (m_lNumBinderValidators == 0)
                {
                    return false;
                }

                for (uint32_t v = 0; v < m_lNumBinderValidators; ++v)
                {
                    if (strcmp(m_BinderValidators[v].m_pszBinderName, "BlendEnabled") == 0)
                    {
                        if (m_BinderValidators[v].m_bBinderValue == bBlendEnabled)
                        {
                            bValidationSuccess = true;
                        }
                        break;
                    }
                }
            }
            else
            {
                // Unknown tag - skip
                continue;
            }

            // No acceptable validator found - fuck this
            if (!bValidationSuccess)
            {
                return false;
            }
        }

        return true;
    }
}