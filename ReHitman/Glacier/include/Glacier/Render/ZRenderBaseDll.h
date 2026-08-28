#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/System/ZDllBase.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/DLLTEXCON.h>
#include <Glacier/ZSTL/LINKREFTAB.h>
#include <cstdint>


namespace Glacier
{
    enum GLOBALMESSAGE
    {
        NONE = 0,
        INSERTDISC = 1,
        READRETRY = 2,
        READERROR = 3,
        WAITSTABLE = 4,
    };

    using GLOBALMESSAGECALLBACK = GLOBALMESSAGE(*)(void*);

    // TODO: Finish me
    struct ZRenderBaseDll : public ZDllBase
    {
        // vtbl
        ~ZRenderBaseDll() override;
        virtual void FinalizeTextureBuffer();
        virtual uint32_t CalcTextureBufferLength();
        virtual void* CalcTextureBuffer();
        virtual void Init();
        virtual void End();
        virtual void PushScene(const char*);
        virtual void PopScene();
        virtual void ParseOptions();
        virtual void* PackPrimBuffer(uint32_t*, char*, unsigned int);
        virtual void RestorePrimBuffer(char*);
        virtual bool CleanupBeforeCloseDown();
        virtual ZRender* SetupWindow(void*);
        virtual void SetActiveAxis(unsigned int);
        virtual void Cleanup();
        virtual void InitPrimPack();
        virtual void CreatePrimControl();
        virtual uint32_t CompactPrimBuffer(void*, unsigned int);
        virtual void InstallPrimBuffer(void*, unsigned int);
        virtual void SetGlobalMessage(GLOBALMESSAGECALLBACK pCallback, void* pUserData);
        virtual void InitTexturePack();
        virtual uint32_t CompactTextureBuffer(void*, unsigned int);
        virtual void InstallTextureBuffer(void*, unsigned int);
        virtual void FreeTextureData(unsigned int);
        virtual void InitMaterialPack();
        virtual void InstallShaders(void*, unsigned int, long unsigned int);
        virtual void InstallMaterialBuffer(void*, unsigned int, unsigned int);
        virtual void SetPopSceneFade(float, unsigned int);
        virtual ZTextureBase* GetTexture(unsigned int);
        virtual uint32_t ReserveTexture(unsigned int, unsigned int);
        virtual void UpdateTexture(unsigned int, const ZBitmap*);
        virtual ZBoneModifyBase* CreateBoneModifier(unsigned int);
        virtual void InitBoneModifier();
        virtual ZRagdollContainer* GetRagdollContainer();
        virtual void CalcRoutsTable();
        virtual void SetTextureLevel(int);
        virtual const void* GetTextureData(unsigned int);

        // methods
        ZRenderBaseDll();

        // members
        uint32_t m_lActiveAxis;
        int32_t m_lDrawConAnd;
        DLLTEXCON* m_pTexCon;
        ZPrimControlBase* m_pPrimControl;
        ZRenderMaterialBuffer* m_pMaterialBuffer;
        char* m_pPrimBuffer;
        char* m_pPrimBufferEnd;
        char* m_pCurrentPrimBuffer;
        char* m_pCurrentPrimBufferBack;
        char* m_pTextureBuffer;
        char* m_pTextureBufferEnd;
        char* m_pCurrentTextureBuffer;
        ZRagdollContainer* m_pRagdollContainer;
        int32_t m_lSceneDepth;
        LINKREFTAB m_rtSceneStack;
        ZTexturePackBase* m_pTexturePack;
        ZRenderMaterialPacker* m_pMaterialPack;
        bool m_bStereoView;
        bool m_bDisableDXT;
        bool m_bDisablePAL;
        bool m_bDisableWBuffer;
        bool m_bDisableMultiTexture;
        bool m_bTextureCompression;
        bool m_bTrilinearFiltering;
        bool m_bDisableHWTnL;
        bool m_bDisableEMBM;
        bool m_bEnableTripleBuffering;
        bool m_bDebugVideo;
        bool m_bDisableNVExt;
        bool m_bDisableVSync;
        bool m_bShowFrameRate;
        float m_fPostFilterLOD;
        float m_fShowULL;
        bool m_bLightDynamicHW;
        uint32_t m_lBackColor;
        uint32_t m_lFadeEndColor;
        float m_fFadeTime;
        bool m_bFadeInPopScene;
        uint32_t m_lAntialias;
        uint32_t m_lAnisotropy;
        float m_fGammaValue;
        RE_ADD_PADDING(sizeof(uint32_t) * 5); // Add padding for weird things, idk
    };

    RE_VERIFY_SIZE(ZRenderBaseDll, 0xC0);
    RE_VERIFY_OFFSET(ZRenderBaseDll, m_pPrimBuffer, 0x20); // Verified in PC ZRenderDll::InstallPrimBuffer

    STATIC_GLOBAL_CLASS_INSTANCE(ZRenderBaseDll*, g_pRenderDll);
}
