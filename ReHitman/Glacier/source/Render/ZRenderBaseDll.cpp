#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/DLLTEXCON.h>
#include <Glacier/Render/ZTextureD3D.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Prim/EPrimType.h>
#include <Glacier/Render/Prim/SHandleTableEntry.h>
#include <Glacier/Render/Prim/SPrimObjectHeader.h>
#include <Glacier/Render/Prim/ZPrimAccess.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Render/Material/ZRenderMaterialBuffer.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Physics/ZRagdollContainer.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/StringUtils.h>
#include <Glacier/ZUniAssert.h>
#include <cstdlib>
#include <cstring>


namespace Glacier
{
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZRenderBaseDll*, g_pRenderDll, 0x008ACA30, nullptr);

    // PC 0x004699B0. Base DLL object ctor: creates the per-scene buffer/saved-state stack
    // (LINKREFTAB with 8 entries of 7 dwords each) and resets all render options/state.
    ZRenderBaseDll::ZRenderBaseDll()
        : m_rtSceneStack(8, 7)
    {
        m_lSceneDepth = -1;
        SetTextureLevel(0);

        m_lActiveAxis = 0;
        m_lDrawConAnd = 0;
        m_pTexCon = nullptr;
        m_pPrimControl = nullptr;
        m_pMaterialBuffer = nullptr;
        m_pPrimBuffer = nullptr;
        m_pPrimBufferEnd = nullptr;
        m_pCurrentPrimBuffer = nullptr;
        m_pCurrentPrimBufferBack = nullptr;
        m_pTextureBuffer = nullptr;
        m_pTextureBufferEnd = nullptr;
        m_pCurrentTextureBuffer = nullptr;
        m_pRagdollContainer = nullptr;
        m_pTexturePack = nullptr;
        m_pMaterialPack = nullptr;

        m_bStereoView = false;
        m_bDisableDXT = false;
        m_bDisablePAL = false;
        m_bDisableWBuffer = false;
        m_bDisableMultiTexture = false;
        m_bTextureCompression = false;
        m_bTrilinearFiltering = true;
        m_bDisableHWTnL = false;
        m_bDisableEMBM = false;
        m_bEnableTripleBuffering = false;
        m_bDebugVideo = false;
        m_bDisableNVExt = false;
        m_bDisableVSync = true;
        m_bShowFrameRate = false;
        m_fPostFilterLOD = 2.0f;
        m_fShowULL = 0.0f;
        m_bLightDynamicHW = false;
        m_lBackColor = -14671824;   // 0xFF202030 (PC ctor m_field90)
        m_lFadeEndColor = 0;
        m_fFadeTime = 0.0f;
        m_bFadeInPopScene = false;
        m_lAntialias = 0;
        m_lAnisotropy = 0;
        m_fGammaValue = 1.0f;
    }

    // PC 0x00469AC0. Destroys the prim/texture buffers and the material/prim controls held by
    // the DLL object, then clears the scene stack.
    ZRenderBaseDll::~ZRenderBaseDll()
    {
        if (m_pPrimControl)
        {
            ZUniMemory::Delete(m_pPrimControl);
            m_pPrimControl = nullptr;
        }
        if (m_pPrimBuffer)
        {
            ZUniMemory::Free(m_pPrimBuffer);
            m_pPrimBuffer = nullptr;
        }
        if (m_pTextureBuffer)
        {
            ZUniMemory::Free(m_pTextureBuffer);
            m_pTextureBuffer = nullptr;
        }
        if (m_pMaterialBuffer)
        {
            ZUniMemory::Delete(m_pMaterialBuffer);
            m_pMaterialBuffer = nullptr;
        }

        m_rtSceneStack.Clear();
        m_lSceneDepth = -1;
    }

    // PC 0x00469330. Base Init only parses the configuration options; platform/device setup is
    // deferred to the render-DLL subclasses.
    void ZRenderBaseDll::Init()
    {
        ParseOptions();
    }

    // PC 0x00469F00 (End override in ZDllBase slot). Base render DLL has no periodic end work.
    void ZRenderBaseDll::End()
    {}

    // PC (ZDllBase slot, base default) - the base object cannot be fully cleaned by itself.
    bool ZRenderBaseDll::CleanupBeforeCloseDown()
    {
        return false;
    }

    // PC 0x00469500. Base PushScene keeps the current texture/buffer level so that
    // InitBoneModifier + SetTextureLevel can enter a new scene.
    void ZRenderBaseDll::PushScene(const char*)
    {
        InitBoneModifier();
        SetTextureLevel(m_lSceneDepth + 1);
    }

    // PC 0x00469E80.
    void ZRenderBaseDll::PopScene()
    {
        if (m_pRagdollContainer)
        {
            ZUniMemory::Delete(m_pRagdollContainer);
            m_pRagdollContainer = nullptr;
        }

        if (m_bFadeInPopScene)
            g_pSysInterface->WindowFirst->FadeScreen(m_fFadeTime, m_lFadeEndColor);

        if (g_pSysInterface->WindowFirst)
            g_pSysInterface->WindowFirst->m_lMaxFrameInterval = 1;

        m_bFadeInPopScene = false;
        SetTextureLevel(m_lSceneDepth - 1);
    }

    // PC 0x00469520.
    void ZRenderBaseDll::SetPopSceneFade(float fFadeTime, uint32_t lFadeEndColor)
    {
        m_lFadeEndColor = lFadeEndColor;
        m_fFadeTime = fFadeTime;
        m_bFadeInPopScene = fFadeTime != 0.0f;
    }

    // PC 0x00469560. Saves/restores the prim/texture buffer state when the engine enters or leaves
    // a scene (each stack entry stores tex/prims cursors and buffer pointers).
    void ZRenderBaseDll::SetTextureLevel(int lLevel)
    {
        if (m_lSceneDepth < 0 || lLevel < 0)
        {
            m_rtSceneStack.Clear();
            m_lSceneDepth = lLevel;
            return;
        }

        if (lLevel > m_lSceneDepth)
        {
            do
            {
                uint32_t* pEntry = m_rtSceneStack.AddStart(static_cast<uint32_t>(m_lSceneDepth));
                if (!pEntry)
                    break;
                pEntry[0] = 0;
                pEntry[1] = reinterpret_cast<uint32_t>(m_pTextureBuffer);
                pEntry[2] = reinterpret_cast<uint32_t>(m_pPrimBuffer);
                pEntry[3] = reinterpret_cast<uint32_t>(m_pPrimBufferEnd);
                pEntry[4] = reinterpret_cast<uint32_t>(m_pCurrentPrimBuffer);
                pEntry[5] = reinterpret_cast<uint32_t>(m_pTextureBufferEnd);
                pEntry[6] = reinterpret_cast<uint32_t>(m_pCurrentTextureBuffer);

                m_pTextureBuffer = nullptr;
                m_pPrimBuffer = nullptr;
                m_pPrimBufferEnd = nullptr;
                m_pCurrentPrimBuffer = nullptr;
                m_pTextureBufferEnd = nullptr;
                m_pCurrentTextureBuffer = nullptr;
                ++m_lSceneDepth;
            }
            while (m_lSceneDepth < lLevel);
        }
        else if (m_lSceneDepth > lLevel)
        {
            while (m_lSceneDepth > lLevel)
            {
                uint32_t* pEntry = m_rtSceneStack.GetRefPtrNr(0);
                if (!pEntry)
                    break;
                ZASSERT(pEntry[0] == static_cast<uint32_t>(m_lSceneDepth - 1));
                ++pEntry;
                if (m_pTextureBuffer)
                {
                    ZUniMemory::Free(m_pTextureBuffer);
                    m_pTextureBuffer = nullptr;
                }
                if (m_pPrimBuffer)
                {
                    ZUniMemory::Free(m_pPrimBuffer);
                    m_pPrimBuffer = nullptr;
                }
                m_pTextureBuffer = reinterpret_cast<char*>(pEntry[1]);
                m_pPrimBuffer = reinterpret_cast<char*>(pEntry[2]);
                m_pPrimBufferEnd = reinterpret_cast<char*>(pEntry[3]);
                m_pCurrentPrimBuffer = reinterpret_cast<char*>(pEntry[4]);
                m_pTextureBufferEnd = reinterpret_cast<char*>(pEntry[5]);
                m_pCurrentTextureBuffer = reinterpret_cast<char*>(pEntry[6]);
                m_rtSceneStack.Remove(static_cast<uint32_t>(m_lSceneDepth - 1));
                --m_lSceneDepth;
            }
        }
    }

    // PC 0x00469770.
    ZRagdollContainer* ZRenderBaseDll::GetRagdollContainer()
    {
        return m_pRagdollContainer;
    }

    // PC 0x004696F0. Creates the ragdoll container used by IK/ragdoll code while in a scene.
    void ZRenderBaseDll::InitBoneModifier()
    {
        ZASSERT(m_pRagdollContainer == nullptr);
        m_pRagdollContainer = ZUniMemory::New<ZRagdollContainer>();
    }

    // PC 0x00469680.
    ZBoneModifyBase* ZRenderBaseDll::CreateBoneModifier(uint32_t lNrBones)
    {
        return ZUniMemory::New<ZBoneModifyBase>(lNrBones);
    }

    // PC 0x00469490. Looks a texture up through the texture container installed by the render DLL.
    ZTextureBase* ZRenderBaseDll::GetTexture(uint32_t lTextureId)
    {
        return m_pTexCon->GetTexture(lTextureId, 0);
    }

    // PC (ZDllBase override slot; empty in base - only the platform render DLLs install data).
    void ZRenderBaseDll::FinalizeTextureBuffer()
    {}

    uint32_t ZRenderBaseDll::CalcTextureBufferLength()
    {
        return 0;
    }

    void* ZRenderBaseDll::CalcTextureBuffer()
    {
        return nullptr;
    }

    const void* ZRenderBaseDll::GetTextureData(uint32_t)
    {
        return nullptr;
    }

    void ZRenderBaseDll::CalcRoutsTable()
    {}

    void ZRenderBaseDll::SetActiveAxis(uint32_t lActiveAxis)
    {
        m_lActiveAxis = lActiveAxis;
    }

    // PC 0x00469340. Frees the prim buffer/prim control and texture buffer owned by the base DLL.
    void ZRenderBaseDll::Cleanup()
    {
        if (m_pPrimBuffer)
        {
            ZUniMemory::Free(m_pPrimBuffer);
            m_pPrimBuffer = nullptr;
        }
        if (m_pPrimControl)
        {
            ZUniMemory::Delete(m_pPrimControl);
            m_pPrimControl = nullptr;
        }
        if (m_pTextureBuffer)
        {
            ZUniMemory::Free(m_pTextureBuffer);
            m_pTextureBuffer = nullptr;
        }
    }

    // The following methods are the base render-DLL interface. Everything that is not already
    // implemented above is a base default (empty or returned 0/null) - the real platform
    // behaviour is installed by the platform render DLL subclasses when they are reversed.

    // PC 0x00469030. Reads the render options from the global configuration into the members.
    void ZRenderBaseDll::ParseOptions()
    {
        char* pszValue = nullptr;

        if (ZSysInterface::GetOption("Anisotropy", &pszValue))
        {
            const long lAnisotropy = atol(pszValue);
            m_lAnisotropy = static_cast<uint32_t>(lAnisotropy);
            if (lAnisotropy > 0x10)
                m_lAnisotropy = 0;
        }
        if (ZSysInterface::GetOption("Antialias", &pszValue))
            m_lAntialias = static_cast<uint32_t>(atol(pszValue));
        if (ZSysInterface::GetOption("DebugVideo", &pszValue))
            m_bDebugVideo = 1;
        if (ZSysInterface::GetOption("DisableDXT", &pszValue))
            m_bDisableDXT = 1;
        if (ZSysInterface::GetOption("DisableHWTnL", &pszValue))
            m_bDisableHWTnL = pszValue[0] ? atol(pszValue) != 0 : 1;
        if (ZSysInterface::GetOption("DisableMultiTexture", &pszValue))
            m_bDisableMultiTexture = 1;
        if (ZSysInterface::GetOption("DisableNVExt", &pszValue))
            m_bDisableNVExt = 1;
        if (ZSysInterface::GetOption("DisablePAL", &pszValue))
            m_bDisablePAL = 1;
        if (ZSysInterface::GetOption("EnableVSync", &pszValue))
            m_bDisableVSync = atoi(pszValue) == 0;
        if (ZSysInterface::GetOption("DisableWBuffer", &pszValue))
            m_bDisableWBuffer = 1;
        if (ZSysInterface::GetOption("DisableEMBM", &pszValue))
            m_bDisableEMBM = 1;
        if (ZSysInterface::GetOption("EnableTripleBuffering", &pszValue))
            m_bEnableTripleBuffering = 1;
        if (ZSysInterface::GetOption("GammaValue", &pszValue))
            m_fGammaValue = static_cast<float>(atof(pszValue));
        if (ZSysInterface::GetOption("ShowFrameRate", &pszValue))
            m_bShowFrameRate = 1;
        if (ZSysInterface::GetOption("PostFilterLOD", &pszValue))
            m_fPostFilterLOD = static_cast<float>(atol(pszValue));
        if (ZSysInterface::GetOption("StereoView", &pszValue))
            m_bStereoView = 1;
        if (ZSysInterface::GetOption("DisableTrilinearFiltering", &pszValue))
            m_bTrilinearFiltering = 0;
        if (ZSysInterface::GetOption("BlurShadows", &pszValue))
            g_bBlurShadows = atoi(pszValue) == 0;
        if (ZSysInterface::GetOption("SelfShadows", &pszValue))
            g_bSelfShadows = atoi(pszValue) == 0;

        if (striwcmp(static_cast<const char*>(g_pSysInterface->m_sProjectPath), "*Hitman2*") == 0)
            m_bLightDynamicHW = 1;
    }

    void* ZRenderBaseDll::PackPrimBuffer(uint32_t* pPackSize, char*, uint32_t)
    {
        // PC 0x00469AB0. Base PackPrimBuffer only zeroes the size slot; the packing itself is
        // done by the platform render DLL after the prim buffer is installed.
        if (pPackSize)
            *pPackSize = 0;
        return nullptr;
    }

    void ZRenderBaseDll::RestorePrimBuffer(char*)
    {}

    ZRender* ZRenderBaseDll::SetupWindow(void*)
    {
        return nullptr;
    }

    void ZRenderBaseDll::InitPrimPack()
    {
        // PC 0x004693B0. Base InitPrimPack only discards a previously installed prim buffer;
        // the platform subclasses create/own the actual prim control and pack.
        if (m_pPrimBuffer)
        {
            ZUniMemory::Free(m_pPrimBuffer);
            m_pPrimBuffer = nullptr;
        }
    }

    void ZRenderBaseDll::CreatePrimControl()
    {
        // PC base slot 25 target 0x4715C0 (empty). The prim control is created by the platform
        // render DLL subclasses; the base class leaves it to them.
    }

    uint32_t ZRenderBaseDll::CompactPrimBuffer(void* pPrimBuffer, uint32_t lPrimBufferSize)
    {
        // PC 0x00469B70.
        if (!g_lPrimToFreeCount)
            return lPrimBufferSize;

        auto* pBuffer = static_cast<char*>(pPrimBuffer);
        char* pPreviousPrimBuffer = m_pPrimBuffer;
        m_pPrimBuffer = pBuffer;

        auto* pHeader = reinterpret_cast<uint32_t*>(pBuffer);
        const uint32_t lHandleCount = pHeader[1];
        g_pPrimHandleTable = reinterpret_cast<SHandleTableEntry*>(pBuffer + pHeader[0]);
        std::memset(g_apPrimHandleToPointerTable, 0, sizeof(g_apPrimHandleToPointerTable));
        g_lPrimHandleToPointerCount = lHandleCount;

        ZASSERT(lHandleCount < STATIC_ARR_LEN(g_apPrimHandleToPointerTable));
        for (uint32_t i = 0; i < lHandleCount; ++i)
        {
            const SHandleTableEntry& entry = g_pPrimHandleTable[i];
            if (entry.lRefCount)
                g_apPrimHandleToPointerTable[i] = pBuffer + entry.lOffset;
        }

        for (uint32_t i = 0; i < g_lPrimToFreeCount; ++i)
        {
            const uint32_t lPrim = g_lPrimToFreeList[i];
            const auto* pObjectHeader = reinterpret_cast<const SPrimObjectHeader*>(
                (lPrim & 0x80000000u) ? reinterpret_cast<const void*>(lPrim & 0x7FFFFFFFu)
                                      : g_apPrimHandleToPointerTable[lPrim]);

            if (pObjectHeader->lType == EPrimType::PTOBJECTHEADER)
            {
                if (g_pMaterialInstanceRefCountTable)
                {
                    const uint32_t lSubPrimCount = m_pPrimControl->CountSubPrims(lPrim);
                    for (uint32_t lVariation = 0; lVariation < lSubPrimCount; ++lVariation)
                    {
                        const uint32_t lMaterialId = m_pPrimControl->GetMaterialIdFromPrim(lPrim, lVariation);
                        if (!lMaterialId)
                            continue;

                        ZRenderMaterialBuffer::g_pMaterialBufferInstance->GetMaterialInstance(lMaterialId);
                        ZASSERT(g_pMaterialInstanceRefCountTable[lMaterialId]);
                        if (--g_pMaterialInstanceRefCountTable[lMaterialId] == 0)
                        {
                            uint32_t aTextures[256];
                            const uint32_t lTextureCount = m_pMaterialBuffer->GetAllTexturesFromMaterial(
                                lMaterialId, aTextures, STATIC_ARR_LEN(aTextures));
                            for (uint32_t lTexture = 0; lTexture < lTextureCount; ++lTexture)
                                FreeTextureData(aTextures[lTexture]);
                        }
                    }
                }

                const uint32_t lPropertyData = pObjectHeader->lPropertyData;
                const uint32_t lObjectCount = pObjectHeader->lNumObjects;
                const uint32_t lObjectTable = pObjectHeader->lObjectTable;
                const uint32_t* pObjects = ZPrimHandle{lObjectTable};
                for (uint32_t lObject = 0; lObject < lObjectCount; ++lObject)
                {
                    ZPrimAccess* pAccess = ZPrimAccess::Create(ZPrimHandle{pObjects[lObject]});
                    pAccess->ReleasePrim();
                    pAccess->Destroy();
                }

                m_pPrimControl->FreePrimData(lObjectTable);
                m_pPrimControl->FreePrimData(lPropertyData);
                m_pPrimControl->FreePrimData(lPrim);
            }
            else
            {
                ZASSERT(pObjectHeader->lType == EPrimType::PTLIGHT);
            }
        }

        uint32_t lDataSize = 0;
        for (uint32_t i = 0; i < lHandleCount; ++i)
        {
            SHandleTableEntry& entry = g_pPrimHandleTable[i];
            if (!entry.lRefCount)
                continue;

            std::memmove(pBuffer + lDataSize, pBuffer + entry.lOffset, entry.lSize);
            entry.lOffset = lDataSize;
            lDataSize += entry.lSize;
        }

        pHeader[0] = lDataSize;
        const uint32_t lHandleTableSize = lHandleCount * sizeof(SHandleTableEntry);
        std::memmove(pBuffer + lDataSize, g_pPrimHandleTable, lHandleTableSize);
        const uint32_t lCompactedSize = lDataSize + lHandleTableSize;
        pHeader[2] = lCompactedSize;

        g_pPrimHandleTable = nullptr;
        m_pPrimBuffer = pPreviousPrimBuffer;
        g_lPrimToFreeCount = 0;
        return lCompactedSize;
    }

    void ZRenderBaseDll::InstallPrimBuffer(void* pPrimBuffer, uint32_t lPrimBufferSize)
    {
        // PC 0x004693D0 (called by ZRenderWintelD3DDll::InstallPrimBuffer at 0x00487AE0).
        if (m_pPrimBuffer)
            ZUniMemory::Free(m_pPrimBuffer);

        auto* pBuffer = static_cast<char*>(pPrimBuffer);
        m_pPrimBuffer = pBuffer;
        m_pPrimBufferEnd = pBuffer + lPrimBufferSize;
        m_pCurrentPrimBuffer = pBuffer + reinterpret_cast<uint32_t*>(pBuffer)[2];
        m_pCurrentPrimBufferBack = m_pPrimBufferEnd;

        const auto* pHeader = reinterpret_cast<const uint32_t*>(pBuffer);
        g_pPrimHandleTable = reinterpret_cast<SHandleTableEntry*>(pBuffer + pHeader[0]);
        std::memset(g_apPrimHandleToPointerTable, 0, sizeof(g_apPrimHandleToPointerTable));
        g_lPrimHandleToPointerCount = pHeader[1];
        g_iCurrentDynamicPrimBuffersCount = static_cast<int32_t>(g_lPrimHandleToPointerCount);
        g_lPrimHandleToPointerFreeBack = STATIC_ARR_LEN(g_apPrimHandleToPointerTable) - 1;

        ZASSERT(g_lPrimHandleToPointerCount < STATIC_ARR_LEN(g_apPrimHandleToPointerTable));
        for (uint32_t i = 0; i < g_lPrimHandleToPointerCount; ++i)
        {
            const SHandleTableEntry& entry = g_pPrimHandleTable[i];
            if (entry.lRefCount)
                g_apPrimHandleToPointerTable[i] = pBuffer + entry.lOffset;
        }

        g_apPrimHandleToPointerTable[0] = nullptr;
        g_pPrimHandleTable = nullptr;
    }

    void ZRenderBaseDll::SetGlobalMessage(GLOBALMESSAGECALLBACK, void*)
    {}

    void ZRenderBaseDll::InitTexturePack()
    {
        // PC 0x00469390. Base InitTexturePack frees the previously installed texture buffer.
        if (m_pTextureBuffer)
        {
            ZUniMemory::Free(m_pTextureBuffer);
            m_pTextureBuffer = nullptr;
        }
    }

    uint32_t ZRenderBaseDll::CompactTextureBuffer(void* pBuffer, uint32_t lSize)
    {
        return lSize;
    }

    void ZRenderBaseDll::InstallTextureBuffer(void* pTextureBuffer, uint32_t lAllocatedTextureBufferSize)
    {
        // PC 0x004694B0. Base InstallTextureBuffer stores the raw texture pack buffer:
        // the platform render DLL later parses/installs the individual textures from it.
        char* pBuffer = static_cast<char*>(pTextureBuffer);
        if (m_pTextureBuffer)
        {
            ZUniMemory::Free(m_pTextureBuffer);
            m_pTextureBuffer = nullptr;
        }
        m_pTextureBuffer = pBuffer;
        m_pCurrentTextureBuffer = pBuffer + *reinterpret_cast<uint32_t*>(pBuffer + 8);
        m_pTextureBufferEnd = pBuffer + lAllocatedTextureBufferSize;
    }

    void ZRenderBaseDll::FreeTextureData(uint32_t)
    {}

    void ZRenderBaseDll::InitMaterialPack()
    {
        // PC 0x0046? - base leaves the material pack to the platform render DLL.
    }

    void ZRenderBaseDll::InstallShaders(void*, uint32_t, uint32_t)
    {}

    void ZRenderBaseDll::InstallMaterialBuffer(void* pBuffer, uint32_t lBufferSize, uint32_t lBufferSizeAllocated)
    {
        // PC 0x004694E0
        if (auto* pMaterialBuffer = g_pRenderDll->m_pMaterialBuffer)
        {
            pMaterialBuffer->InstallBuffer(pBuffer, lBufferSize, lBufferSizeAllocated);
        }
    }

    uint32_t ZRenderBaseDll::ReserveTexture(uint32_t, uint32_t)
    {
        return 0;
    }

    void ZRenderBaseDll::UpdateTexture(uint32_t, const ZBitmap*)
    {}
}
