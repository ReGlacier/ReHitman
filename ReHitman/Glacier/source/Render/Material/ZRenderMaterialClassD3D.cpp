#include <Glacier/Render/Material/ZRenderMaterialClassD3D.h>

#include <Glacier/Render/Material/ZRenderMaterialSubClassD3D.h>
#include <Glacier/Render/Material/ZRenderMaterialResourceD3DFX.h>
#include <Glacier/Render/SRMaterialProperties.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/ZRPropertyReader.h>
#include <Glacier/Filesystem/IBuffer.h>
#include <cstring>


namespace Glacier
{
    namespace
    {
        // Checks whether the BIND list contains an enabled element whose NAME equals pszName
        // and whose VALU equals bValue.
        bool CheckBinderValidator(const ZRPropertyReader& rBindList, const char* pszName, bool bValue)
        {
            const ZRPropertyReader::SProperty* pRoot = rBindList.m_pProperty;
            ZASSERT(pRoot && pRoot->lType == ZRPropertyReader::PROPERTY_TYPE::PT_LIST);

            const auto* pElements = static_cast<const ZRPropertyReader::SProperty*>(rBindList.m_pBuffer->GetData(pRoot->lData));

            for (uint32_t i = 0; i < pRoot->lSize; ++i)
            {
                ZRPropertyReader sElement{};
                sElement.m_pBuffer = rBindList.m_pBuffer;
                sElement.m_pProperty = const_cast<ZRPropertyReader::SProperty*>(&pElements[i]);

                ZRPropertyReader sEnabElem{};
                sElement.GetNamedListElement('ENAB', sEnabElem);
                ZASSERT(sEnabElem.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_UINT32);

                const uint32_t bEnabled =
                    (sEnabElem.m_pProperty->lSize == 1u)
                        ? sEnabElem.m_pProperty->lData
                        : *static_cast<const uint32_t*>(sEnabElem.m_pBuffer->GetData(sEnabElem.m_pProperty->lData));

                if (!bEnabled)
                {
                    continue;
                }

                ZRPropertyReader sNameElem{};
                sElement.GetNamedListElement('NAME', sNameElem);
                ZASSERT(sNameElem.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_CHAR);

                const char* psz = static_cast<const char*>(sNameElem.m_pBuffer->GetData(sNameElem.m_pProperty->lData));
                if (strcmp(psz, pszName) != 0)
                {
                    continue;
                }

                ZRPropertyReader sValuElem{};
                sElement.GetNamedListElement('VALU', sValuElem);
                ZASSERT(sValuElem.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_UINT32);

                const uint32_t lVal =
                    (sValuElem.m_pProperty->lSize == 1u)
                        ? sValuElem.m_pProperty->lData
                        : *static_cast<const uint32_t*>(sValuElem.m_pBuffer->GetData(sValuElem.m_pProperty->lData));

                return bValue == (lVal != 0u);
            }

            return false;
        }
    }

    ZRenderMaterialClassD3D::ZRenderMaterialClassD3D() = default;

    ZRenderMaterialClassD3D::~ZRenderMaterialClassD3D()
    {
        for (uint32_t i = 0; i < m_lNumSubClasses; ++i)
        {
            ZUniMemory::Delete(m_pSubClasses[i]);
        }

        for (uint32_t i = 0; i < m_lNumSubClassTemplates; ++i)
        {
            for (uint32_t v = 0; v < m_SubClassTemplates[i].m_lNumBinderValidators; ++v)
            {
                ZUniMemory::Delete(m_SubClassTemplates[i].m_pBinderValidators[v]);
            }
        }
    }

    ZRenderMaterialInstance* ZRenderMaterialClassD3D::CreateMaterialInstance(const SRMaterialProperties* pMatProperties, uint32_t lMaterialId)
    {
        ZRPropertyReader sRoot{};
        sRoot.m_pBuffer = reinterpret_cast<IBuffer*>(g_pRenderDll->m_pMaterialBuffer);
        sRoot.m_pProperty = fuck_cast<ZRPropertyReader::SProperty>(sRoot.m_pBuffer->GetData(pMatProperties->lNameOffset));

        ZRPropertyReader sBindList{};
        sRoot.GetNamedListElement('BIND', sBindList);

        // Reuse an existing subclass that can already create this material.
        for (uint32_t i = 0; i < m_lNumSubClasses; ++i)
        {
            auto* pSubClass = static_cast<ZRenderMaterialSubClassD3D*>(m_pSubClasses[i]);
            if (pSubClass->CanCreateMaterialInstance(pMatProperties))
            {
                return pSubClass->CreateMaterialInstance(pMatProperties, lMaterialId);
            }
        }

        // Match a sub-class template and create a fresh subclass.
        ZRenderMaterialSubClass* pSubClass = nullptr;
        const SSubClassTemplate* pTemplate = nullptr;

        for (uint32_t i = 0; i < m_lNumSubClassTemplates; ++i)
        {
            const SSubClassTemplate& rTemplate = m_SubClassTemplates[i];
            if (rTemplate.m_lObjectType != pMatProperties->lObjectType ||
                rTemplate.m_lObjectSubType != pMatProperties->lObjectSubType)
            {
                continue;
            }

            bool bMatches = true;
            for (uint32_t v = 0; v < rTemplate.m_lNumBinderValidators; ++v)
            {
                const SRenderMaterialBinderValidator* pValidator = rTemplate.m_pBinderValidators[v];
                if (!CheckBinderValidator(sBindList, pValidator->m_pszBinderName, pValidator->m_bBinderValue))
                {
                    bMatches = false;
                    break;
                }
            }

            if (!bMatches)
            {
                continue;
            }

            ZRPropertyReader sName{};
            rTemplate.m_Properties.GetNamedListElement('NAME', sName);
            ZASSERT(sName.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_CHAR);
            const char* pszName = static_cast<const char*>(sName.m_pBuffer->GetData(sName.m_pProperty->lData));

            pSubClass = CreateMaterialSubClass(pszName, pMatProperties->lObjectType, pMatProperties->lObjectSubType, pMatProperties->lMaterialClassFlags);
            if (pSubClass)
            {
                pTemplate = &rTemplate;
                break;
            }
        }

        if (pSubClass)
        {
            ZASSERT(m_lNumSubClasses < MAX_NUM_SUB_CLASSES);
            pSubClass->m_lIndex = static_cast<uint16_t>(m_lNumSubClasses);
            m_pSubClasses[m_lNumSubClasses++] = pSubClass;

            auto* pD3DSubClass = static_cast<ZRenderMaterialSubClassD3D*>(pSubClass);
            pD3DSubClass->SetBinderValidators(&sBindList);

            // Create the FX layers referenced by this sub-class template.
            ZRenderMaterialResourceD3DFX* pResourceFX = ZSharedResourcesD3D::g_pInstance->m_pResourceFX;
            const ZRPropertyReader& rProperties = pTemplate->m_Properties;

            ZASSERT(rProperties.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_LIST);
            const auto* pElements = static_cast<const ZRPropertyReader::SProperty*>(rProperties.m_pBuffer->GetData(rProperties.m_pProperty->lData));

            for (uint32_t i = 0; i < rProperties.m_pProperty->lSize; ++i)
            {
                ZRPropertyReader sElement{};
                sElement.m_pBuffer = rProperties.m_pBuffer;
                sElement.m_pProperty = const_cast<ZRPropertyReader::SProperty*>(&pElements[i]);

                if (sElement.m_pProperty->lName != 'LAYE')
                {
                    continue;
                }

                ZASSERT(sElement.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_LIST);

                ZRPropertyReader sVali{};
                sElement.GetNamedListElement('VALI', sVali);
                ZASSERT(sVali.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_CHAR);
                const char* pszVali = static_cast<const char*>(sVali.m_pBuffer->GetData(sVali.m_pProperty->lData));

                if (*pszVali && !CheckBinderValidator(sBindList, pszVali, true))
                {
                    continue;
                }

                ZRPropertyReader sType{};
                sElement.GetNamedListElement('TYPE', sType);
                ZASSERT(sType.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_CHAR);
                const char* pszType = static_cast<const char*>(sType.m_pBuffer->GetData(sType.m_pProperty->lData));

                if (strcmp(pszType, "FX") == 0)
                {
                    pResourceFX->CreateMaterialLayer(pD3DSubClass, &sElement);
                }
            }

            pSubClass->Initialize();
        }

        if (pSubClass)
        {
            return pSubClass->CreateMaterialInstance(pMatProperties, lMaterialId);
        }

        // No matching sub-class. The original read the material NAME here for a (stripped)
        // debug message and then returned nullptr.
        return nullptr;
    }

    void ZRenderMaterialClassD3D::Initialize()
    {
        // Do nothing
    }

    void ZRenderMaterialClassD3D::AddMaterialSubClassTemplate(const ZRPropertyReader* pMatProperties)
    {
        if (m_lNumSubClassTemplates == MAX_NUM_SUB_CLASS_TEMPLATES)
        {
            return;
        }

        SSubClassTemplate& rTemplate = m_SubClassTemplates[m_lNumSubClassTemplates++];
        rTemplate.m_Properties = *pMatProperties;
        rTemplate.m_lNumBinderValidators = 0u;

        ZASSERT(pMatProperties->m_pProperty && pMatProperties->m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_LIST);
        const auto* pElements = static_cast<const ZRPropertyReader::SProperty*>(pMatProperties->m_pBuffer->GetData(pMatProperties->m_pProperty->lData));

        for (uint32_t i = 0; i < pMatProperties->m_pProperty->lSize; ++i)
        {
            ZRPropertyReader sElement{};
            sElement.m_pBuffer = pMatProperties->m_pBuffer;
            sElement.m_pProperty = const_cast<ZRPropertyReader::SProperty*>(&pElements[i]);

            if (sElement.m_pProperty->lName != 'VALI')
            {
                continue;
            }

            auto* pValidator = ZUniMemory::New<SRenderMaterialBinderValidator>();

            ZRPropertyReader sBind{};
            sElement.GetNamedListElement('BIND', sBind);
            ZASSERT(sBind.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_CHAR);
            pValidator->m_pszBinderName = static_cast<const char*>(sBind.m_pBuffer->GetData(sBind.m_pProperty->lData));

            ZRPropertyReader sValu{};
            sElement.GetNamedListElement('VALU', sValu);
            ZASSERT(sValu.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_UINT32);
            const uint32_t lVal =
                (sValu.m_pProperty->lSize == 1u)
                    ? sValu.m_pProperty->lData
                    : *static_cast<const uint32_t*>(sValu.m_pBuffer->GetData(sValu.m_pProperty->lData));

            pValidator->m_bBinderValue = (lVal != 0u);
            rTemplate.m_pBinderValidators[rTemplate.m_lNumBinderValidators++] = pValidator;
        }

        // Object type (OTYP).
        uint32_t lObjectType = 0u;
        ZRPropertyReader sOtyp{};
        if (pMatProperties->TryGetNamedListElement('OTYP', sOtyp))
        {
            ZASSERT(sOtyp.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_CHAR);
            const char* psz = static_cast<const char*>(sOtyp.m_pBuffer->GetData(sOtyp.m_pProperty->lData));
            if (strcmp(psz, "Mesh") == 0)
            {
                lObjectType = 8u;
            }
            else if (strcmp(psz, "Sprites") == 0)
            {
                lObjectType = 2u;
            }
            else
            {
                ZASSERT(false);
            }
        }
        rTemplate.m_lObjectType = lObjectType;

        // Object sub-type (STYP).
        uint32_t lObjectSubType = 0u;
        ZRPropertyReader sStyp{};
        if (pMatProperties->TryGetNamedListElement('STYP', sStyp))
        {
            ZASSERT(sStyp.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_CHAR);
            const char* psz = static_cast<const char*>(sStyp.m_pBuffer->GetData(sStyp.m_pProperty->lData));
            if (strcmp(psz, "Standard") == 0)
            {
                lObjectSubType = 0u;
            }
            else if (strcmp(psz, "Tweened") == 0)
            {
                lObjectSubType = 1u;
            }
            else if (strcmp(psz, "Rigid") == 0)
            {
                lObjectSubType = 2u;
            }
            else if (strcmp(psz, "Weighted") == 0)
            {
                lObjectSubType = 3u;
            }
            else if (strcmp(psz, "Array") == 0)
            {
                lObjectSubType = 1u;
            }
            else if (strcmp(psz, "ArrayBox") == 0)
            {
                lObjectSubType = 3u;
            }
            else if (strcmp(psz, "ArrayBoxScale") == 0)
            {
                lObjectSubType = 4u;
            }
            else if (strcmp(psz, "ArrayUV") == 0)
            {
                lObjectSubType = 5u;
            }
            else if (strcmp(psz, "ArrayRaw") == 0)
            {
                lObjectSubType = 6u;
            }
            else if (strcmp(psz, "ArrayParticle") == 0)
            {
                lObjectSubType = 7u;
            }
        }
        rTemplate.m_lObjectSubType = lObjectSubType;
    }

    ZFactory<ZRenderMaterialClassD3D>& ZRenderMaterialClassD3D::GetFactory()
    {
        static ZFactory<ZRenderMaterialClassD3D> instance;
        return instance;
    }
}
