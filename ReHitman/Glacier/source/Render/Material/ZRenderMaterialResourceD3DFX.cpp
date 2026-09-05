#include <Glacier/Render/Material/ZRenderMaterialResourceD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialSubClassD3D.h>
#include <Glacier/Render/Material/ZRenderMaterialEffectD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialLayerD3DFX.h>
#include <Glacier/Render/ZSharedResourcesD3D.h>
#include <Glacier/Render/ZRPropertyReader.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Filesystem/IBuffer.h>
#include <Glacier/Filesystem/ZSysFile.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <cstring>


namespace Glacier
{
    namespace
    {
        // PC: 0x7F7408 - effect file suffixes by shader quality level (shared with CreateEffect)
        const char* const s_aShaderVersionSuffix[4] = { "00", "13", "20", "30" };

        // PC: 0x7F7418 (s_aShaderQualityTable) - quality level names for the "Quality" technique annotation
        const char* const s_aShaderQualityTable[4] = { "Low", "Medium", "High", "VeryHigh" };

        // PC: 0x438F70 - case-insensitive substring search; returns pointer to the match or nullptr
        const char* cstrcmp(const char* pszString, const char* pszSubStr)
        {
            const size_t lSubStrLen = strlen(pszSubStr);
            const size_t lStringLen = strlen(pszString);
            if (lStringLen < lSubStrLen)
            {
                return nullptr;
            }

            const char* pLastMatch = pszString + (lStringLen - lSubStrLen);
            while (memicmp(pszString, pszSubStr, lSubStrLen) != 0)
            {
                if (++pszString > pLastMatch)
                {
                    return nullptr;
                }
            }
            return pszString;
        }
    }

    ZRenderMaterialResourceD3DFX::ZRenderMaterialResourceD3DFX()
        : ZRenderMaterialResourceD3D()
        , m_lNumLayers(0)
        , m_lNumEffects(0)
        , m_EffectCache()
        , m_pEffectPool(nullptr)
    {
        m_EffectCache.m_bDisableShaderCache = ZSysInterface::GetOption("DisableShaderCache", nullptr);
    }

    ZRenderMaterialResourceD3DFX::~ZRenderMaterialResourceD3DFX()
    {
        FreeResources();
    }
    
    void ZRenderMaterialResourceD3DFX::GetShaderPath(zstring& sPath)
    {
        MYSTR sComputedPath = g_pSysInterface->ProjectPath() + MYSTR("Shaders\\PC\\FX");
        sPath = sComputedPath.String;
    }

    ZRenderMaterialEffectD3DFX* ZRenderMaterialResourceD3DFX::CreateEffect(const char* pszFileName, const D3DXMACRO* pDefined, uint32_t lNumDefines)
    {
        // Already created with the same name and defines?
        for (uint32_t i = 0; i < m_lNumEffects; ++i)
        {
            if (m_pEffects[i]->IsSame(pszFileName, pDefined, lNumDefines))
            {
                return m_pEffects[i];
            }
        }

        // Full path of the shader source
        zstring sShadersPath;
        GetShaderPath(sShadersPath);
        const zstring sSourcePath = sShadersPath + "\\" + pszFileName;

        // Load the source into the effect cache (also resolves #include deps for compilation)
        void* pSrcData = m_EffectCache.Load(sSourcePath.c_str(), nullptr);
        if (!pSrcData)
        {
            return nullptr;
        }

        // Compiled-effect cache directory: <temp>\<ProjectName>
        CHAR szTempPath[512];
        GetTempPathA(sizeof(szTempPath), szTempPath);
        MYSTR sProjectName = g_pSysInterface->ProjectName();
        const zstring sCompiledDir = (MYSTR(szTempPath) + sProjectName).String;

        // Compiled-effect cache file name: <dir>\<file>V2_Q<quality suffix><per-macro suffixes>[_PP]
        zstring sCompiledName = sCompiledDir + "\\" + pszFileName;
        sCompiledName = sCompiledName + "V2" + "_Q";

        uint32_t lQualityLevel = ZSharedResourcesD3D::g_pInstance->m_lShaderQuality;
        if (lQualityLevel > 3)
        {
            lQualityLevel = 3;
        }
        sCompiledName = sCompiledName + s_aShaderVersionSuffix[lQualityLevel];

        g_pSysFile->MakeDirNested(sCompiledDir.c_str());

        for (uint32_t i = 0; i < lNumDefines; ++i)
        {
            const zstring sMacroName(pDefined[i].Name);
            const char* pEnabled = strstr(sMacroName.c_str(), "ENABLED");
            const uint32_t lStripPos = pEnabled
                ? static_cast<uint32_t>(pEnabled - sMacroName.c_str())
                : sMacroName.length();
            const zstring sMacroSuffix = "_" + zstring(sMacroName, 0, lStripPos);
            sCompiledName = sCompiledName + sMacroSuffix.c_str();
        }

        const uint32_t lShaderFlagsBase = D3DXSHADER_NO_PRESHADER | D3DXSHADER_PREFER_FLOW_CONTROL; // 0x500
        uint32_t lShaderFlags = lShaderFlagsBase;
        if (ZSharedResourcesD3D::g_pInstance->m_bDebugVertexShader)
        {
            lShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT; // -> 0x540
        }
        if (ZSharedResourcesD3D::g_pInstance->m_bDebugPixelShader)
        {
            lShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT; // | 0x80
        }
        if (ZSharedResourcesD3D::g_pInstance->m_bUsePartialPrecisionShader)
        {
            lShaderFlags |= D3DXSHADER_PARTIALPRECISION; // | 0x20
            sCompiledName = sCompiledName + "_PP";
        }

        // Reuse the compiled effect when the sources have not changed
        void* pCompiledData = nullptr;
        int lCompiledSize = g_pSysFile->GetSize(sCompiledName.c_str(), false);
        if (lCompiledSize != -1)
        {
            const uint64_t iCompiledTime = g_pSysFile->GetFileTime64(sCompiledName.c_str(), false);
            if (m_EffectCache.IsUpToDate(m_EffectCache.FindFile(sSourcePath.c_str()), iCompiledTime))
            {
                pCompiledData = ZUniMemory::Allocate(lCompiledSize);
                g_pSysFile->Load(sCompiledName.c_str(), pCompiledData, lCompiledSize, 0, false);
            }
        }

        ID3DXBuffer* pErrors = nullptr;
        if (!pCompiledData)
        {
            // Compile the effect (includes are resolved against the effect cache)
            const ZEffectCacheD3DFX::SFile* pSrcFile = m_EffectCache.FindFile(sSourcePath.c_str());
            const int lSrcLen = pSrcFile ? pSrcFile->iSize : -1;

            ID3DXEffectCompiler* pCompiler = nullptr;
            if (D3DXCreateEffectCompiler(static_cast<LPCSTR>(pSrcData), lSrcLen, pDefined, &m_EffectCache, lShaderFlags, &pCompiler, &pErrors) < 0)
            {
                if (pErrors)
                {
                    pErrors->Release();
                }
                return nullptr;
            }

            ID3DXBuffer* pCompiledBuffer = nullptr;
            if (pCompiler->CompileEffect(lShaderFlags, &pCompiledBuffer, &pErrors) < 0 && pErrors)
            {
                pErrors->Release();
                pCompiler->Release();
                return nullptr;
            }

            lCompiledSize = static_cast<int>(pCompiledBuffer->GetBufferSize());
            pCompiledData = ZUniMemory::Allocate(lCompiledSize);
            memcpy(pCompiledData, pCompiledBuffer->GetBufferPointer(), lCompiledSize);
            pCompiledBuffer->Release();
            pCompiler->Release();

            // Store to the compiled-effect cache
            g_pSysFile->Save(sCompiledName.c_str(), pCompiledData, lCompiledSize, 0);
        }

        // Create the D3DX effect
        ID3DXEffect* pD3DXEffect = nullptr;
        const HRESULT hResult = D3DXCreateEffect(g_pd3dDevice->m_pDevice, pCompiledData, lCompiledSize, nullptr, nullptr,
                                                 lShaderFlags | D3DXFX_NOT_CLONEABLE, m_pEffectPool, &pD3DXEffect, &pErrors);
        ZUniMemory::Free(pCompiledData);
        if (hResult < 0)
        {
            return nullptr;
        }

        pD3DXEffect->SetStateManager(g_pd3dDevice);

        auto* pEffect = ZUniMemory::New<ZRenderMaterialEffectD3DFX>(pszFileName, sSourcePath.c_str(), pD3DXEffect, pDefined, lNumDefines);
        ZASSERT(pEffect);
        ZASSERT(m_lNumEffects < MAX_UNM_FX_EFFECTS);
        m_pEffects[m_lNumEffects++] = pEffect;

        return pEffect;
    }

    ZRenderMaterialEffectD3DFX* ZRenderMaterialResourceD3DFX::AddEffect(const char* pszFileName, ZRenderMaterialSubClassD3D* pMaterialSubClass)
    {
        constexpr int MAX_MACROS_NR = 64;

        const char* pszMaterialClassName = [&]() -> const char*
        {
            if (!pMaterialSubClass)
            {
                return "UNDEFINED";
            }

            if (pMaterialSubClass->m_lObjectType == 2)
            {
                return "SPRITES";
            }

            if (pMaterialSubClass->m_lObjectType == 8)
            {
                switch (pMaterialSubClass->m_lObjectSubType)
                {
                    case 0: return "MESH_STANDARD";
                    case 1: return "MESH_TWEENED";
                    case 2: return "MESH_RIGID";
                    case 3: return "MESH_WEIGHTED";
                    default: return "MESH";
                }
            }

            return "UNDEFINED";
        }();
        
        char* pszName = (char*)ZUniMemory::Allocate(strlen(pszMaterialClassName) + 1);
        strcpy(pszName, pszMaterialClassName);

        D3DXMACRO aMacros[MAX_MACROS_NR];

        // Macro #0
        aMacros[0].Name = pszName;
        aMacros[0].Definition = "1";

        // Macro #1
        aMacros[1].Name = "BlendEnabled";
        aMacros[1].Definition = "0";

        uint32_t lMacroCount = 2;

        // Init other macros
        if (pMaterialSubClass && pMaterialSubClass->m_lNumBinderValidators > 0)
        {
            for (uint32_t i = 0; i < pMaterialSubClass->m_lNumBinderValidators; ++i)
            {
                SRenderMaterialBinderValidator* pValidator = &pMaterialSubClass->m_BinderValidators[i];
                if (pValidator->m_bBinderValue && lMacroCount < (MAX_MACROS_NR - 1))
                {
                    const uint32_t lBinderLen = strlen(pValidator->m_pszBinderName);
                    char* pszUpperBinderName = (char*)ZUniMemory::Allocate(lBinderLen + 1);

                    for (size_t k = 0; k < lBinderLen; ++k)
                    {
                        pszUpperBinderName[k] = (char)toupper((unsigned char)pValidator->m_pszBinderName[k]);
                    }
                    pszUpperBinderName[lBinderLen] = '\0';

                    aMacros[lMacroCount].Name = pszUpperBinderName;
                    aMacros[lMacroCount].Definition = "1";
                    ++lMacroCount;
                }
            }
        }

        // Add terminator macro
        aMacros[lMacroCount].Name = nullptr;
        aMacros[lMacroCount].Definition = nullptr;

        // Compile effect
        ZRenderMaterialEffectD3DFX* pEffect = CreateEffect(pszFileName, aMacros, lMacroCount);

        // Cleanup (macro #1 is the "BlendEnabled" literal - only it is not allocated by ZUniMemory)
        for (uint32_t j = 0; j < lMacroCount; ++j)
        {
            if (j != 1)
            {
                ZUniMemory::Free((void*)aMacros[j].Name);
            }
        }

        return pEffect;
    }

    ZRenderMaterialLayerD3DFX* ZRenderMaterialResourceD3DFX::AddLayer(ZRenderMaterialLayerD3DFX* pLayer)
    {
        for (uint32_t i = 0; i < m_lNumLayers; ++i)
        {
            if (m_pMaterialLayers[i]->m_hTechnique == pLayer->m_hTechnique)
            {
                // delete layer because layer with current technique already exists
                ZUniMemory::Delete(pLayer);

                return m_pMaterialLayers[i];
            }
        }

        // Check for out of bounds and store at new index
        ZASSERT(m_lNumLayers < MAX_NUM_FX_MATERIAL_LAYERS);
        m_pMaterialLayers[m_lNumLayers++] = pLayer;

        return pLayer;
    }

    void ZRenderMaterialResourceD3DFX::CreateMaterialLayer(ZRenderMaterialSubClassD3D* pMaterialSubClass, const ZRPropertyReader* pMatPropReader)
    {
        // Layer name table (PC Blood Money; the Mini Ninjas XBOX table is extended and reordered - do not copy)
        ZRPropertyReader sNameElem{};
        pMatPropReader->GetNamedListElement('NAME', sNameElem);
        ZASSERT(sNameElem.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_CHAR);
        const char* pszName = static_cast<const char*>(sNameElem.m_pBuffer->GetData(sNameElem.m_pProperty->lData));

        uint32_t lLayerIndex;
        if (!strcmp(pszName, "ZFill")) lLayerIndex = 0;
        else if (!strcmp(pszName, "Ambient")) lLayerIndex = 1;
        else if (!strcmp(pszName, "ShadowMap")) lLayerIndex = 2;
        else if (!strcmp(pszName, "ShadowDrop")) lLayerIndex = 3;
        else if (!strcmp(pszName, "ShadowAdd")) lLayerIndex = 4;
        else if (!strcmp(pszName, "Lighting")) lLayerIndex = 5;
        else if (!strcmp(pszName, "Refraction")) lLayerIndex = 6;
        else if (!strcmp(pszName, "Reflection")) lLayerIndex = 7;
        else if (!strcmp(pszName, "Illuminate")) lLayerIndex = 8;
        else if (!strcmp(pszName, "PostFilterZPass")) lLayerIndex = 9;
        else if (!strcmp(pszName, "PostFilterZPassAlpha")) lLayerIndex = 10;
        else if (!strcmp(pszName, "PostFilterBloom")) lLayerIndex = 11;
        else if (!strcmp(pszName, "FurFins")) lLayerIndex = 14;
        else if (!strcmp(pszName, "FurShells")) lLayerIndex = 15;
        else if (!strcmp(pszName, "TextureProjection")) lLayerIndex = 12;
        else if (!strcmp(pszName, "TextureProjectionAdditive")) lLayerIndex = 16;
        else if (!strcmp(pszName, "ShadowSub")) lLayerIndex = 17;
        else
        {
            if (strcmp(pszName, "RefractionMask"))
            {
                return;
            }
            lLayerIndex = 13;
        }

        ZRPropertyReader sPathElem{};
        pMatPropReader->GetNamedListElement('PATH', sPathElem);
        ZASSERT(sPathElem.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_CHAR);
        const char* pszPath = static_cast<const char*>(sPathElem.m_pBuffer->GetData(sPathElem.m_pProperty->lData));

        // Ambient layer at lowest shader quality falls back to "Standard" for cloth effects
        if (lLayerIndex == 1 && ZSharedResourcesD3D::g_pInstance->m_lShaderQuality == ZSharedResourcesD3D::SHADERQUALITY::SHADERQUALITY_LOW && cstrcmp(pszPath, "cloth"))
        {
            pszPath = "Standard";
        }

        ZRPropertyReader sIdenElem{};
        pMatPropReader->GetNamedListElement('IDEN', sIdenElem);
        ZASSERT(sIdenElem.m_pProperty->lType == ZRPropertyReader::PROPERTY_TYPE::PT_CHAR);
        const char* pszIdentifier = static_cast<const char*>(sIdenElem.m_pBuffer->GetData(sIdenElem.m_pProperty->lData));

        // Try descending shader quality levels until an effect with a matching technique is found
        D3DXHANDLE hTechnique = nullptr;
        ZRenderMaterialEffectD3DFX* pEffect = nullptr;
        int lShaderQuality = ZSharedResourcesD3D::g_pInstance->m_lShaderQuality;
        int lNextQuality = lShaderQuality;

        while (lShaderQuality != -1)
        {
            zstring sEffectName;
            if (strstr(pszPath, ".fx"))
            {
                // Full effect file name given - single attempt
                sEffectName = zstring(pszPath);
                lNextQuality = -1;
            }
            else
            {
                int lSuffixIndex = lShaderQuality;
                if (lSuffixIndex > 3)
                {
                    lSuffixIndex = 3;
                }

                char szEffectName[512];
                strcpy(szEffectName, pszPath);
                strcat(szEffectName, s_aShaderVersionSuffix[lSuffixIndex]);
                strcat(szEffectName, ".fx");
                sEffectName = zstring(szEffectName);
            }

            pEffect = AddEffect(sEffectName.c_str(), pMaterialSubClass);
            if (pEffect)
            {
                ID3DXEffect* pD3DXEffect = pEffect->m_pD3DXEffect;

                D3DXEFFECT_DESC effectDesc;
                pD3DXEffect->GetDesc(&effectDesc);

                const uint32_t lQualityLevelRaw = ZSharedResourcesD3D::g_pInstance->m_lShaderQuality;
                uint32_t lQualityLevelClamped = lQualityLevelRaw;
                if (lQualityLevelClamped > 3)
                {
                    lQualityLevelClamped = 3;
                }
                const char* pszQuality = s_aShaderQualityTable[lQualityLevelClamped];

                for (uint32_t i = 0; i < effectDesc.Techniques; ++i)
                {
                    D3DXHANDLE hCurrentTechnique = pD3DXEffect->GetTechnique(i);

                    D3DXTECHNIQUE_DESC currentTechniqueDesc;
                    pD3DXEffect->GetTechniqueDesc(hCurrentTechnique, &currentTechniqueDesc);

                    D3DXHANDLE hIdentifierAnnotation = pD3DXEffect->GetAnnotationByName(hCurrentTechnique, "Identifier");
                    if (!hIdentifierAnnotation)
                    {
                        continue;
                    }

                    const char* pszTechniqueIdentifier = nullptr;
                    pD3DXEffect->GetString(hIdentifierAnnotation, &pszTechniqueIdentifier);
                    if (stricmp(pszIdentifier, pszTechniqueIdentifier))
                    {
                        continue;
                    }

                    bool bAccepted = false;

                    D3DXHANDLE hQualityAnnotation = pD3DXEffect->GetAnnotationByName(hCurrentTechnique, "Quality");
                    if (hQualityAnnotation)
                    {
                        const char* pszTechniqueQuality = nullptr;
                        pD3DXEffect->GetString(hQualityAnnotation, &pszTechniqueQuality);
                        if (!stricmp(pszTechniqueQuality, pszQuality))
                        {
                            bAccepted = true;
                        }
                    }

                    D3DXHANDLE hPerformanceAnnotation = pD3DXEffect->GetAnnotationByName(hCurrentTechnique, "Performance");
                    if (!bAccepted && hPerformanceAnnotation)
                    {
                        const char* pszTechniquePerformance = nullptr;
                        pD3DXEffect->GetString(hPerformanceAnnotation, &pszTechniquePerformance);
                        if ((!stricmp(pszTechniquePerformance, "Fastest") && lQualityLevelRaw <= 1)
                            || (!stricmp(pszTechniquePerformance, "Blend") && lQualityLevelRaw >= 2)
                            || (!stricmp(pszTechniquePerformance, "Quality") && lQualityLevelRaw >= 2))
                        {
                            bAccepted = true;
                        }
                    }

                    // Techniques without Quality/Performance annotations match any quality level
                    if (!bAccepted && !hQualityAnnotation && !hPerformanceAnnotation)
                    {
                        bAccepted = true;
                    }

                    if (bAccepted)
                    {
                        hTechnique = hCurrentTechnique;
                        break;
                    }
                }
            }

            if (lNextQuality == -1)
            {
                lShaderQuality = -1;
            }
            else
            {
                lShaderQuality = lNextQuality - 1;
            }
            lNextQuality = lShaderQuality;

            if (hTechnique)
            {
                break;
            }
        }

        if (!hTechnique)
        {
            return;
        }

        // Result is unused in the original code
        D3DXTECHNIQUE_DESC techniqueDesc;
        pEffect->m_pD3DXEffect->GetTechniqueDesc(hTechnique, &techniqueDesc);

        auto* pLayer = ZUniMemory::New<ZRenderMaterialLayerD3DFX>(pszName, hTechnique, pEffect);
        ZASSERT(pLayer);

        pMaterialSubClass->m_pLayers[lLayerIndex] = AddLayer(pLayer);
    }

    void ZRenderMaterialResourceD3DFX::FreeResources()
    {
        for (int i = 0; i < m_lNumLayers; ++i)
        {
            if (m_pMaterialLayers[i])
            {
                ZUniMemory::Delete(m_pMaterialLayers[i]);
                m_pMaterialLayers[i] = nullptr;
            }
        }
        m_lNumLayers = 0;

        for (int i = 0; i < m_lNumEffects; ++i)
        {
            if (m_pEffects[i])
            {
                ZUniMemory::Delete(m_pEffects[i]);
                m_pEffects[i] = nullptr;
            }
        }
        m_lNumEffects = 0;

        if (!g_bIsResettingDevice)
        {
            m_EffectCache.Clear();
        }
    }
}