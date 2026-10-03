#pragma once

#include <cstdint>
#include <Glacier/ReGlacier.h>
#include <Glacier/Render/ZTextureBase.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZSTL/MYSTR.h>
#include <Glacier/LoaderSequence/ZLoader_Sequence_Player_Base.h>
#include <Glacier/LoaderSequence/ZLoader_Sequence_Script.h>

// D3D9 types for the sprite resources (the Glacier target links d3d9/d3dx9).
#include <Glacier/Render/D3D9.h>

namespace Glacier
{
    class FsZip_t;

    // One sprite record inside a loader-sequence object (PC verified).
    // The layout is ZTextureBase-compatible: InstallTextureBuffer hands these
    // blocks to ZTextureManagerD3D::CreateTexture() as ZTextureD3D* (PC wraps
    // the call at 0x48D790), and the manager only writes the 0x44-byte
    // ZTextureBase prefix, leaving the sprite-specific tail intact.
    // Original source: engine\drawing\_wintel\renderd3d\loader_sequence_player_wintel_d3d.cpp
    struct ZGeometrySubObject : public ZTextureBase
    {
        int32_t m_iCustomParamX;                 // +0x44 sprite top-left X (design-resolution pixels, from the prim shape record)
        int32_t m_iCustomParamY;                 // +0x48 sprite top-left Y (design-resolution pixels, from the prim shape record)
        IDirect3DVertexBuffer9* pVertexBuffer;   // +0x4C triangle-strip quad (4 vertices, 0x1C stride)
    };
    RE_VERIFY_SIZE(ZGeometrySubObject, 0x50);
    RE_VERIFY_OFFSET(ZGeometrySubObject, m_iCustomParamX, 0x44);
    RE_VERIFY_OFFSET(ZGeometrySubObject, m_iCustomParamY, 0x48);
    RE_VERIFY_OFFSET(ZGeometrySubObject, pVertexBuffer, 0x4C);

    struct ZGeometryObjectHeader
    {
        ZGeometrySubObject* pSubObjectsArray; // +0x00
        int iSubObjectCount;                  // +0x04
    };
    RE_VERIFY_SIZE(ZGeometryObjectHeader, 0x8);

    // Wintel/D3D implementation of the loading-sequence player.
    // PC vtable 0x00765DBC (6 slots): deleting dtor + Start/Finish/Progress/
    // Disable_Render/Enable_Render, indices match ZLoader_Sequence_Player_Base.
    // Original source: engine\drawing\_wintel\renderd3d\loader_sequence_player_wintel_d3d.cpp
    class ZLoader_Sequence_Wintel_D3D : public ZLoader_Sequence_Player_Base
    {
    public:
        // constants
        static constexpr uint32_t kTexture_Pool_Size = 0x00E4E1C0u; // PC Ctor pool allocation (~15 MB)
        static constexpr uint32_t kTexture_Pool_Align = 1024u;      // PC Ctor aligns m_pPoolEnd up to 1024

        // static
        STATIC_CLASS_VAR(ZLoader_Sequence_Wintel_D3D, bool, m_bRender_Enabled);      // PC 0x0090DC88
        STATIC_CLASS_VAR(ZLoader_Sequence_Wintel_D3D, bool, m_bEnable_Render);       // PC 0x0090DC89
        STATIC_CLASS_VAR(ZLoader_Sequence_Wintel_D3D, bool, m_bDisable_Render);      // PC 0x0090DC8A
        STATIC_CLASS_VAR(ZLoader_Sequence_Wintel_D3D, bool, m_bStopIt);              // PC 0x0090DC8B
        STATIC_CLASS_VAR(ZLoader_Sequence_Wintel_D3D, void*, m_hLoadPictureVBEvent); // PC 0x0090DC8C

        // vtbl
        ~ZLoader_Sequence_Wintel_D3D() override; // PC 0x4AEFC0 (complete), 0x4AF090 (deleting)
        void Start() override;                   // PC 0x4AF0B0
        void Finish() override;                  // PC 0x4AE360
        void Progress(float fProgress) override; // PC 0x4ADAF0
        void Disable_Render() override;          // PC 0x4ADB70
        void Enable_Render() override;           // PC 0x4ADBB0

        // methods
        static ZLoader_Sequence_Player_Base* Produce(); // rev @ 0x4AEDD0 (factory: allocate + ctor)

        // The original code only creates instances through Produce(); the ctor
        // stays public because ZUniMemory::New<T>() cannot reach a private one.
        ZLoader_Sequence_Wintel_D3D(); // rev @ 0x4AE2C0

    private:
        // methods
        void Free_All();                                                                                                            // rev @ 0x4AEE30
        void Finish_0();                                                                                                            // rev @ 0x4AE260 (eax carries WaitForSingleObject result in PC; unused)
        bool LoadScript(const char* pScript);                                                                                       // rev @ 0x4ADDB0
        bool Create_Sprites(Glacier::FsZip_t& rZip, const char* pPicture_Names, uint32_t uiPrim_Base);                              // rev @ 0x4AEF10
        bool InstallTextureBuffer(Glacier::FsZip_t& rZip, const char* pPicture_Names, uint32_t uiPrim_Base);                        // rev @ 0x4AE3D0
        bool InstallPrimBuffer();                                                                                                   // rev @ 0x4AE1F0
        static void Get_Prim_Data(void** ppBuffer, int* pSize, Glacier::FsZip_t& rZip);                                             // rev @ 0x4ADBF0 (__stdcall, no this)
        static const char* Get_My_Data(const char* pGms_Buffer, const char* pPrim_Buffer);                                          // rev @ 0x4ADD60 (__stdcall, no this)
        void Get_Geoms_Data(char** ppGeoms_Data, int* pGeoms_Data_Size, Glacier::FsZip_t& rZip);                                    // rev @ 0x4ADC50
        bool Create_Render_Thread();                                                                                                // rev @ 0x4AF010
        static uint32_t __stdcall Render_Thread(void* pParam);                                                                      // rev @ 0x4AEF50 (CreateThread entry, never returns)
        void Render();                                                                                                              // rev @ 0x4AEBD0
        void InitGeometry(int iObject_Index, int iSubObject_Index);                                                                 // rev @ 0x4AE930
        MYSTR Calc_Total_Name(const char* pFile_Name) const;                                                                        // rev @ 0x4ADE60 (reconstructed)

    public:
        // data
        void* m_hEvent;                    // +0x04 manual-reset event, render thread handshake
        void* m_hThread;                   // +0x08 render thread handle
        ZLoader_Sequence_Script* pReader;  // +0x0C runtime script data (NOT a Script_Reader)
        uint8_t* m_pPoolEnd;               // +0x10 next free byte in m_pTexture_Pool (1024-aligned at ctor)
        float m_fCurrentTime;              // +0x14 seconds since render thread start
        int64_t m_liStartTick;             // +0x18 g_pSysInterface->TimeStampCounter() baseline
        ZGeometryObjectHeader* m_aObjects; // +0x20 new[]-style array (4-byte count cookie at -4)
        int m_iTotalObjects;               // +0x24
        uint8_t* m_pTexture_Pool;          // +0x28 staging pool for the loading-sequence TEX/prim data (kTexture_Pool_Size)
        int m_iTexture_Pool_Step;          // +0x2C pool advance granularity (bytes) used when loading TEX
    };
    RE_VERIFY_SIZE(ZLoader_Sequence_Wintel_D3D, 0x30);
    RE_VERIFY_OFFSET(ZLoader_Sequence_Wintel_D3D, m_fCurrentTime, 0x14);
    RE_VERIFY_OFFSET(ZLoader_Sequence_Wintel_D3D, m_aObjects, 0x20);
}
