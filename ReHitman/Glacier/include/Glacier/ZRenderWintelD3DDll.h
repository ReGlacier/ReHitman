#pragma once

#include <cstdint>

#include <Glacier/LINKREFTAB.h>
#include <Glacier/ZTextureManagerD3D.h>

namespace Glacier
{
    class ZRenderMaterialBufferD3D;
    class ZRenderWintelD3D;
    class ZPrimControlWintel;

    class ZRenderWintelD3DDll
    {
    public:
        // vftable
        virtual void Release();
        virtual void Function_01();
        virtual void Function_02();
        virtual ZRenderMaterialBufferD3D* CreateMaterialBuffer();
        virtual void CleanupBeforeCloseDown();
        virtual void CreateRagdolls(int);
        virtual int Function_06();
        virtual void Function_07();
        virtual void Function_08();
        virtual void Function_09();
        virtual void Function_10();
        virtual int* Function_11();
        virtual int SetName(int dllHandle, const char*);
        virtual void* GetFunctionByName(const char*);
        virtual void Function_14();
        virtual void Function_15();
        virtual bool Function_16();
        virtual bool Function_17();
        virtual int ParseOptions();
        virtual int Function_19(int, int);
        virtual void Function_020();
        virtual ZRenderWintelD3D* CreateD3D(int);
        virtual void Function_022();
        virtual void Cleanup();
        virtual void FreePrimitivesBufferMemory();
        virtual ZPrimControlWintel* CreatePrimitivesControl();
        virtual int  LoadGeometry(char* buffer, int size);
        virtual int  InstallPrimBuffer(char* buffer, int size);
        virtual void Function_028();
        virtual void FreeField0x30();
        virtual int  Function_030(int, int);
        virtual void PreloadTexturesFromTEX(char* buffer, int size);
        virtual void Function_032();
        virtual void Function_033();
        virtual void Function_034();
        virtual int  Function_035(int, int, int);
        virtual void SetPopSceneFade(float, int);
        virtual int* GetTexture(int);
        virtual int  ReserveTexture(int, int);
        virtual int  UpdateTexture(int, ZBitmap*);
        virtual int  CreateBoneModifier(int);
        virtual int* InitBoneModifier();
        virtual int* GetRagdollContainer();
        virtual void Function_43();
        virtual int  SetTextureLevel(int mipLevel);
        virtual void Function_45();
        virtual void Function_46();

        // data
        char *m_dllName;
        HMODULE m_dllHandle;
        int m_fieldC;
        int m_field10;
        ZTextureManagerD3D *m_textureManager;
        int m_primControlWintel;
        int m_materialBuffer;
        char *m_prmBuffer;
        char *m_prmEndOfBuffer;
        int m_prmPtrOfChunk2;
        int m_prmEndOfBuffer2;
        char *m_texBuffer;
        int m_field34;
        unsigned int m_field38;
        int m_ragdolls;
        int m_currentTextureLevel;
        LINKREFTAB m_textureLevels;
        int m_field6C;
        int m_field70;
        char m_stereoView;
        char m_disableDXT;
        char m_disablePAL;
        char m_disableWBuffer;
        char m_disableMultiTexture;
        char field_79;
        char m_disableTrilinearFiltering;
        char m_disableHWTnL;
        char m_disableEMBM;
        char m_enableTripleBuffering;
        char m_debugVideo;
        char m_disableNVExt;
        char m_enableVSync;
        char m_showFrameRate;
        char field_82;
        char field_83;
        float m_postFilterLOD;
        int m_field88;
        int m_isHitman2;
        int m_field90;
        int m_field94;
        float m_field98;
        int m_field9C;
        int m_antialias;
        int m_anisotropy;
        float m_gammaValue;
        int m_fieldAC;
        int m_fieldB0;
        int m_fieldB4;
        int m_fieldB8;
    };

    static_assert(offsetof(ZRenderWintelD3DDll, m_fieldB8) == 0x00B8, "STATIC TEST FAILED! Await 0x00B8 but actual value isn't right");
}