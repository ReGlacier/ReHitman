#include <Glacier/Render/Material/ZRenderMaterialBinderParser.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderList.h>
#include <Glacier/Render/Material/ZRenderMaterialBuffer.h>
#include <Glacier/Render/Material/ZRenderMaterialInstance.h>
#include <Glacier/Render/ZRPropertyReader.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZTextureD3D.h>
#include <Glacier/Render/SRMaterialProperties.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderMaterialBinderParser::ZRenderMaterialBinderParser()
        : m_pTranslatorMapper(g_TranslatorMapper)
    {
    }

    ZRenderMaterialBinderParser::~ZRenderMaterialBinderParser() = default;

    void ZRenderMaterialBinderParser::CreateBinders(ZRenderMaterialBinderList* pMaterialBinderList, ZRenderMaterialInstance* pMaterialInstance)
    {
        ZASSERT(pMaterialBinderList->m_BindersMaterial.m_pBinders == nullptr);
        ZASSERT(pMaterialBinderList->m_BindersObject.m_pBinders == nullptr);

        ZRenderBinder* aMaterialBindersStack[256];
        ZRenderBinder* aObjectBindersStack[256];

        // Attach to stack allocated stuff
        pMaterialBinderList->m_BindersMaterial.m_pBinders = aMaterialBindersStack;
        pMaterialBinderList->m_BindersObject.m_pBinders   = aObjectBindersStack;

        // Run parsers
        CreatePropertyBinders(pMaterialBinderList, pMaterialInstance);
        CreateBuiltInBinders(pMaterialBinderList, pMaterialInstance);

        // Alloc&copy materials
        const uint32_t lNumMaterialBinders = pMaterialBinderList->m_BindersMaterial.m_lNumBinders;
        if (lNumMaterialBinders > 0)
        {
            ZRenderBinder** ppBindersHeap = static_cast<ZRenderBinder**>(ZUniMemory::Allocate(sizeof(ZRenderBinder*) * lNumMaterialBinders));

            for (uint32_t i = 0; i < lNumMaterialBinders; ++i)
            {
                ZASSERT(i < pMaterialBinderList->m_BindersMaterial.m_lNumBinders);
                ppBindersHeap[i] = pMaterialBinderList->m_BindersMaterial.m_pBinders[i];
            }

            pMaterialBinderList->m_BindersMaterial.m_pBinders = ppBindersHeap;
        }
        else
        {
            pMaterialBinderList->m_BindersMaterial.m_pBinders = nullptr;
        }

        // Allocate&copy objects
        const uint32_t lNumObjectBinders = pMaterialBinderList->m_BindersObject.m_lNumBinders;
        if (lNumObjectBinders > 0)
        {
            ZRenderBinder** ppBindersHeap = static_cast<ZRenderBinder**>(ZUniMemory::Allocate(sizeof(ZRenderBinder*) * lNumObjectBinders));

            for (uint32_t j = 0; j < lNumObjectBinders; ++j)
            {
                ZASSERT(j < pMaterialBinderList->m_BindersObject.m_lNumBinders);
                ppBindersHeap[j] = pMaterialBinderList->m_BindersObject.m_pBinders[j];
            }

            pMaterialBinderList->m_BindersObject.m_pBinders = ppBindersHeap;
        }
        else
        {
            pMaterialBinderList->m_BindersObject.m_pBinders = nullptr;
        }
    }

    void ZRenderMaterialBinderParser::CreatePropertyBinders(ZRenderMaterialBinderList* pMaterialBinderList, ZRenderMaterialInstance* pMaterialInstance)
    {
        ZRenderMaterialBuffer* pMaterialBuffer = g_pRenderDll->m_pMaterialBuffer;

        const auto* pRootProperty = static_cast<const ZRPropertyReader::SProperty*>(
            pMaterialBuffer->GetData(pMaterialInstance->m_pMaterialProperties->lNameOffset));

        // The original scans the root property list for every 'BIND' list (there can be
        // several of them): once one BIND list is exhausted, the root scan continues.
        for (uint32_t i = 0; i < pRootProperty->lSize; ++i)
        {
            ZASSERT(pRootProperty->lType == ZRPropertyReader::PT_LIST);
            ZASSERT(i < pRootProperty->lSize);

            const auto* pRootElements = static_cast<const ZRPropertyReader::SProperty*>(pMaterialBuffer->GetData(pRootProperty->lData));
            const auto* pBindProperty = &pRootElements[i];

            // Empty BIND lists are skipped too.
            if (pBindProperty->lName != 'BIND' || !pBindProperty->lSize)
            {
                continue;
            }

            for (uint32_t j = 0; j < pBindProperty->lSize; ++j)
            {
                ZASSERT(pBindProperty->lType == ZRPropertyReader::PT_LIST);
                ZASSERT(j < pBindProperty->lSize);

                const auto* pBindElements = static_cast<const ZRPropertyReader::SProperty*>(pMaterialBuffer->GetData(pBindProperty->lData));

                ZRPropertyReader sElement{};
                sElement.m_pBuffer = pMaterialBuffer;
                sElement.m_pProperty = const_cast<ZRPropertyReader::SProperty*>(&pBindElements[j]);

                ZRPropertyReader sEnabElem{};
                sElement.GetNamedListElement('ENAB', sEnabElem);
                ZASSERT(sEnabElem.m_pProperty->lType == ZRPropertyReader::PT_UINT32);

                // lSize == 1: value is stored inline in lData, otherwise lData is a buffer offset.
                const uint32_t* pEnabData = (sEnabElem.m_pProperty->lSize == 1)
                    ? &sEnabElem.m_pProperty->lData
                    : static_cast<const uint32_t*>(sEnabElem.m_pBuffer->GetData(sEnabElem.m_pProperty->lData));

                if (!*pEnabData)
                {
                    // Binder is disabled - skip it.
                    continue;
                }

                switch (sElement.m_pProperty->lName)
                {
                    case 'BOOL':
                    {
                        ZRPropertyReader sNameElem{};
                        sElement.GetNamedListElement('NAME', sNameElem);
                        ZASSERT(sNameElem.m_pProperty->lType == ZRPropertyReader::PT_CHAR);
                        const char* pszBinderName = static_cast<const char*>(sNameElem.m_pBuffer->GetData(sNameElem.m_pProperty->lData));

                        ZRPropertyReader sValuElem{};
                        sElement.GetNamedListElement('VALU', sValuElem);
                        ZASSERT(sValuElem.m_pProperty->lType == ZRPropertyReader::PT_UINT32);

                        const uint32_t lNumBools = sValuElem.m_pProperty->lSize;
                        const uint32_t* pBools = (lNumBools == 1)
                            ? &sValuElem.m_pProperty->lData
                            : static_cast<const uint32_t*>(sValuElem.m_pBuffer->GetData(sValuElem.m_pProperty->lData));

                        CreatePropertyBinderBool(&pMaterialBinderList->m_BindersMaterial, pszBinderName, pBools, lNumBools);
                        break;
                    }

                    case 'COLO':
                    {
                        ZRPropertyReader sNameElem{};
                        sElement.GetNamedListElement('NAME', sNameElem);
                        ZASSERT(sNameElem.m_pProperty->lType == ZRPropertyReader::PT_CHAR);
                        const char* pszBinderName = static_cast<const char*>(sNameElem.m_pBuffer->GetData(sNameElem.m_pProperty->lData));

                        ZRPropertyReader sValuElem{};
                        sElement.GetNamedListElement('VALU', sValuElem);
                        if (sValuElem.m_pProperty->lType == ZRPropertyReader::PT_FLOAT)
                        {
                            ZASSERT(sValuElem.m_pProperty->lSize == 4);
                            const float* pColor = (sValuElem.m_pProperty->lSize == 1)
                                ? reinterpret_cast<const float*>(&sValuElem.m_pProperty->lData)
                                : static_cast<const float*>(sValuElem.m_pBuffer->GetData(sValuElem.m_pProperty->lData));

                            CreatePropertyBinderColor(&pMaterialBinderList->m_BindersMaterial, pszBinderName, pColor);
                        }
                        break;
                    }

                    case 'DMAP':
                    {
                        ZRPropertyReader sNameElem{};
                        sElement.GetNamedListElement('NAME', sNameElem);
                        ZASSERT(sNameElem.m_pProperty->lType == ZRPropertyReader::PT_CHAR);
                        sNameElem.m_pBuffer->GetData(sNameElem.m_pProperty->lData);

                        ZRPropertyReader sTxidElem{};
                        sElement.GetNamedListElement('TXID', sTxidElem);
                        ZASSERT(sTxidElem.m_pProperty->lType == ZRPropertyReader::PT_UINT32);
                        if (sTxidElem.m_pProperty->lSize != 1)
                        {
                            sTxidElem.m_pBuffer->GetData(sTxidElem.m_pProperty->lData);
                        }

                        // 'DMAP' binders are not supported by the original code.
                        ZASSERT(false);
                        break;
                    }

                    case 'ENUM':
                    {
                        ZRPropertyReader sNameElem{};
                        sElement.GetNamedListElement('NAME', sNameElem);
                        ZASSERT(sNameElem.m_pProperty->lType == ZRPropertyReader::PT_CHAR);
                        const char* pszBinderName = static_cast<const char*>(sNameElem.m_pBuffer->GetData(sNameElem.m_pProperty->lData));

                        ZRPropertyReader sValuElem{};
                        sElement.GetNamedListElement('VALU', sValuElem);
                        ZASSERT(sValuElem.m_pProperty->lType == ZRPropertyReader::PT_CHAR);
                        const char* pszEnum = static_cast<const char*>(sValuElem.m_pBuffer->GetData(sValuElem.m_pProperty->lData));

                        CreatePropertyBinderEnum(&pMaterialBinderList->m_BindersMaterial, pszBinderName, pszEnum);
                        break;
                    }

                    case 'FLTV':
                    {
                        ZRPropertyReader sNameElem{};
                        sElement.GetNamedListElement('NAME', sNameElem);
                        ZASSERT(sNameElem.m_pProperty->lType == ZRPropertyReader::PT_CHAR);
                        const char* pszBinderName = static_cast<const char*>(sNameElem.m_pBuffer->GetData(sNameElem.m_pProperty->lData));

                        ZRPropertyReader sValuElem{};
                        sElement.GetNamedListElement('VALU', sValuElem);
                        ZASSERT(sValuElem.m_pProperty->lType == ZRPropertyReader::PT_FLOAT);

                        const uint32_t lNumFloats = sValuElem.m_pProperty->lSize;
                        const float* pFloats = (lNumFloats == 1)
                            ? reinterpret_cast<const float*>(&sValuElem.m_pProperty->lData)
                            : static_cast<const float*>(sValuElem.m_pBuffer->GetData(sValuElem.m_pProperty->lData));

                        CreatePropertyBinderFloat(&pMaterialBinderList->m_BindersMaterial, pszBinderName, pFloats, lNumFloats);
                        break;
                    }

                    case 'TEXT':
                    {
                        ZRPropertyReader sNameElem{};
                        sElement.GetNamedListElement('NAME', sNameElem);
                        ZASSERT(sNameElem.m_pProperty->lType == ZRPropertyReader::PT_CHAR);
                        const char* pszBinderName = static_cast<const char*>(sNameElem.m_pBuffer->GetData(sNameElem.m_pProperty->lData));

                        ZRPropertyReader sTxidElem{};
                        sElement.GetNamedListElement('TXID', sTxidElem);
                        ZASSERT(sTxidElem.m_pProperty->lType == ZRPropertyReader::PT_UINT32);

                        const uint32_t* pTxidData = (sTxidElem.m_pProperty->lSize == 1)
                            ? &sTxidElem.m_pProperty->lData
                            : static_cast<const uint32_t*>(sTxidElem.m_pBuffer->GetData(sTxidElem.m_pProperty->lData));

                        const char* pszMinFilter = nullptr;
                        const char* pszMagFilter = nullptr;
                        const char* pszMipFilter = nullptr;
                        const char* pszTilingU    = nullptr;
                        const char* pszTilingV    = nullptr;
                        const char* pszTilingW    = nullptr;

                        // All sampler states are optional.
                        ZRPropertyReader sOptElem{};
                        if (sElement.TryGetNamedListElement('FMIN', sOptElem))
                        {
                            ZASSERT(sOptElem.m_pProperty->lType == ZRPropertyReader::PT_CHAR);
                            pszMinFilter = static_cast<const char*>(sOptElem.m_pBuffer->GetData(sOptElem.m_pProperty->lData));
                        }
                        if (sElement.TryGetNamedListElement('FMAG', sOptElem))
                        {
                            ZASSERT(sOptElem.m_pProperty->lType == ZRPropertyReader::PT_CHAR);
                            pszMagFilter = static_cast<const char*>(sOptElem.m_pBuffer->GetData(sOptElem.m_pProperty->lData));
                        }
                        if (sElement.TryGetNamedListElement('FMIP', sOptElem))
                        {
                            ZASSERT(sOptElem.m_pProperty->lType == ZRPropertyReader::PT_CHAR);
                            pszMipFilter = static_cast<const char*>(sOptElem.m_pBuffer->GetData(sOptElem.m_pProperty->lData));
                        }
                        if (sElement.TryGetNamedListElement('TILU', sOptElem))
                        {
                            ZASSERT(sOptElem.m_pProperty->lType == ZRPropertyReader::PT_CHAR);
                            pszTilingU = static_cast<const char*>(sOptElem.m_pBuffer->GetData(sOptElem.m_pProperty->lData));
                        }
                        if (sElement.TryGetNamedListElement('TILV', sOptElem))
                        {
                            ZASSERT(sOptElem.m_pProperty->lType == ZRPropertyReader::PT_CHAR);
                            pszTilingV = static_cast<const char*>(sOptElem.m_pBuffer->GetData(sOptElem.m_pProperty->lData));
                        }
                        if (sElement.TryGetNamedListElement('TILW', sOptElem))
                        {
                            ZASSERT(sOptElem.m_pProperty->lType == ZRPropertyReader::PT_CHAR);
                            pszTilingW = static_cast<const char*>(sOptElem.m_pBuffer->GetData(sOptElem.m_pProperty->lData));
                        }

                        const int32_t lTextureId = static_cast<int32_t>(*pTxidData);
                        if (lTextureId < 0)
                        {
                            // Negative texture ids reference built-in context textures (see RT_NAME).
                            CreatePropertyBinderContext(
                                &pMaterialBinderList->m_BindersMaterial,
                                pszBinderName,
                                static_cast<uint32_t>(lTextureId & 0x7FFFFFFF) + 34u);
                        }
                        else
                        {
                            const ZTextureD3D* pTexture = g_pRenderDll->m_pTexCon->GetTexture(static_cast<uint32_t>(lTextureId), 0u);

                            // Animated textures are bound per object, static ones per material.
                            ZRenderBinderList* pBinderList = &pMaterialBinderList->m_BindersMaterial;
                            if (pTexture->m_pAnimData)
                            {
                                pBinderList = &pMaterialBinderList->m_BindersObject;
                            }

                            CreatePropertyBinderTexture(
                                pBinderList,
                                pszBinderName,
                                static_cast<uint32_t>(lTextureId),
                                pszMinFilter,
                                pszMagFilter,
                                pszMipFilter,
                                pszTilingU,
                                pszTilingV,
                                pszTilingW);
                        }
                        break;
                    }

                    case 'SPRI':
                        CreatePropertyBinderSprite(&pMaterialBinderList->m_BindersMaterial);
                        break;

                    case 'RSTA':
                    {
                        const char* pszBlendMode = nullptr;
                        float fOpacity = 1.0f;

                        ZRPropertyReader sBenaElem{};
                        sElement.GetNamedListElement('BENA', sBenaElem);
                        ZASSERT(sBenaElem.m_pProperty->lType == ZRPropertyReader::PT_UINT32);
                        const uint32_t* pBenaData = (sBenaElem.m_pProperty->lSize == 1)
                            ? &sBenaElem.m_pProperty->lData
                            : static_cast<const uint32_t*>(sBenaElem.m_pBuffer->GetData(sBenaElem.m_pProperty->lData));
                        const bool bBlendEnabled = (*pBenaData != 0);

                        if (bBlendEnabled)
                        {
                            ZRPropertyReader sBmodElem{};
                            sElement.GetNamedListElement('BMOD', sBmodElem);
                            ZASSERT(sBmodElem.m_pProperty->lType == ZRPropertyReader::PT_CHAR);
                            pszBlendMode = static_cast<const char*>(sBmodElem.m_pBuffer->GetData(sBmodElem.m_pProperty->lData));

                            ZRPropertyReader sOpacElem{};
                            sElement.GetNamedListElement('OPAC', sOpacElem);
                            ZASSERT(sOpacElem.m_pProperty->lType == ZRPropertyReader::PT_FLOAT);
                            const float* pOpacity = (sOpacElem.m_pProperty->lSize == 1)
                                ? reinterpret_cast<const float*>(&sOpacElem.m_pProperty->lData)
                                : static_cast<const float*>(sOpacElem.m_pBuffer->GetData(sOpacElem.m_pProperty->lData));
                            fOpacity = *pOpacity;
                        }

                        ZRPropertyReader sAtstElem{};
                        sElement.GetNamedListElement('ATST', sAtstElem);
                        ZASSERT(sAtstElem.m_pProperty->lType == ZRPropertyReader::PT_UINT32);
                        const uint32_t* pAtstData = (sAtstElem.m_pProperty->lSize == 1)
                            ? &sAtstElem.m_pProperty->lData
                            : static_cast<const uint32_t*>(sAtstElem.m_pBuffer->GetData(sAtstElem.m_pProperty->lData));
                        const bool bAlphaTestEnabled = (*pAtstData != 0);

                        ZRPropertyReader sArefElem{};
                        sElement.GetNamedListElement('AREF', sArefElem);
                        ZASSERT(sArefElem.m_pProperty->lType == ZRPropertyReader::PT_UINT32);
                        const uint32_t* pArefData = (sArefElem.m_pProperty->lSize == 1)
                            ? &sArefElem.m_pProperty->lData
                            : static_cast<const uint32_t*>(sArefElem.m_pBuffer->GetData(sArefElem.m_pProperty->lData));
                        const uint32_t lAlphaTestRef = *pArefData;

                        ZRPropertyReader sFenaElem{};
                        sElement.GetNamedListElement('FENA', sFenaElem);
                        ZASSERT(sFenaElem.m_pProperty->lType == ZRPropertyReader::PT_UINT32);
                        const uint32_t* pFenaData = (sFenaElem.m_pProperty->lSize == 1)
                            ? &sFenaElem.m_pProperty->lData
                            : static_cast<const uint32_t*>(sFenaElem.m_pBuffer->GetData(sFenaElem.m_pProperty->lData));
                        const bool bFogEnabled = (*pFenaData != 0);

                        ZRPropertyReader sCullElem{};
                        sElement.GetNamedListElement('CULL', sCullElem);
                        ZASSERT(sCullElem.m_pProperty->lType == ZRPropertyReader::PT_CHAR);
                        const char* pszCullMode = static_cast<const char*>(sCullElem.m_pBuffer->GetData(sCullElem.m_pProperty->lData));

                        ZRPropertyReader sZbiaElem{};
                        sElement.GetNamedListElement('ZBIA', sZbiaElem);
                        ZASSERT(sZbiaElem.m_pProperty->lType == ZRPropertyReader::PT_UINT32);
                        const uint32_t* pZbiaData = (sZbiaElem.m_pProperty->lSize == 1)
                            ? &sZbiaElem.m_pProperty->lData
                            : static_cast<const uint32_t*>(sZbiaElem.m_pBuffer->GetData(sZbiaElem.m_pProperty->lData));
                        const uint32_t lZBias = *pZbiaData;

                        ZRPropertyReader sZoffElem{};
                        sElement.GetNamedListElement('ZOFF', sZoffElem);
                        ZASSERT(sZoffElem.m_pProperty->lType == ZRPropertyReader::PT_FLOAT);
                        const float* pfZOffset = (sZoffElem.m_pProperty->lSize == 1)
                            ? reinterpret_cast<const float*>(&sZoffElem.m_pProperty->lData)
                            : static_cast<const float*>(sZoffElem.m_pBuffer->GetData(sZoffElem.m_pProperty->lData));

                        CreatePropertyBinderRenderState(
                            &pMaterialBinderList->m_BindersMaterial,
                            bBlendEnabled,
                            pszBlendMode,
                            fOpacity,
                            bAlphaTestEnabled,
                            lAlphaTestRef,
                            bFogEnabled,
                            pszCullMode,
                            lZBias,
                            *pfZOffset);
                        break;
                    }

                    case 'SCRL':
                    {
                        ZRPropertyReader sSpedElem{};
                        sElement.GetNamedListElement('SPED', sSpedElem);
                        ZASSERT(sSpedElem.m_pProperty->lType == ZRPropertyReader::PT_FLOAT);

                        const float* pfScrollSpeed = (sSpedElem.m_pProperty->lSize == 1)
                            ? reinterpret_cast<const float*>(&sSpedElem.m_pProperty->lData)
                            : static_cast<const float*>(sSpedElem.m_pBuffer->GetData(sSpedElem.m_pProperty->lData));

                        CreatePropertyBinderScroll(&pMaterialBinderList->m_BindersMaterial, pfScrollSpeed);
                        break;
                    }
                }
            }
        }
    }

    void ZRenderMaterialBinderParser::CreateBuiltInBinders(ZRenderMaterialBinderList* pMaterialBinderList, ZRenderMaterialInstance* pMaterialInstance)
    {
        if (!m_pTranslatorMapper)
            return;

        for (const SMapper* pMapper = m_pTranslatorMapper; pMapper->Type != RTT_NONE; ++pMapper)
        {
            if (pMapper->Type == RTT_MATERIAL)
            {
                CreatePropertyBinderContext(
                    &pMaterialBinderList->m_BindersMaterial,
                    pMapper->pszName, 
                    static_cast<uint32_t>(pMapper->Name)
                );
            }
            else if (pMapper->Type == RTT_OBJECT)
            {
                CreatePropertyBinderContext(
                    &pMaterialBinderList->m_BindersObject, 
                    pMapper->pszName, 
                    static_cast<uint32_t>(pMapper->Name)
                );
            }
        }
    }

    void ZRenderMaterialBinderParser::VerifyBinders(int lLayer, ZRenderMaterialBinderList* pMaterialBinderList, ZRenderMaterialInstance* pMaterialInstance)
    {
        // Do nothing or lost debug stuff
    }
}