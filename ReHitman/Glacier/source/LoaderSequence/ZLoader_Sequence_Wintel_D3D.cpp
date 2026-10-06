#include <Glacier/LoaderSequence/ZLoader_Sequence_Wintel_D3D.h>
#include <Glacier/LoaderSequence/ZLoader_Sequence_Player.h>
#include <Glacier/LoaderSequence/ZLoader_Sequence_Setup.h>
#include <Glacier/Data/SCompiledGeom.h>
#include <Glacier/Data/SPackedGeomsHeader.h>
#include <Glacier/Data/SPackedGeomsTree.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRender.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Filesystem/FsZip_t.h>
#include <Glacier/Filesystem/ZSysFile.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Render/ZTextureManagerD3D.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Bitmap/ZBitmap.h>
#include <Glacier/Render/Bitmap/ZBitmap32.h>
#include <Glacier/Render/Bitmap/ZBitmapU8V8.h>
#include <Glacier/Render/Bitmap/ZBitmapI8.h>
#include <Glacier/Render/Bitmap/ZBitmapPal.h>
#include <Glacier/Render/Bitmap/ZBitmapPalOpac.h>
#include <Glacier/Render/Bitmap/ZBitmapDXT1.h>
#include <Glacier/Render/Bitmap/ZBitmapDXT3.h>
#include <Glacier/ZPackedDataChunk.h>
#include <Glacier/ZTextureType.h>
#include <Glacier/ZUniAssert.h>
#include <Windows.h>
#include <cstring>

namespace Glacier
{
    namespace
    {
        // --- magic numbers (PC) -------------------------------------------------
        constexpr const char* kSequence_Zip_File = "Loader_Sequence.ZIP";
        constexpr const char* kSequence_Gms_Pattern = "*Loader_Sequence.GMS";
        constexpr const char* kSequence_Prm_Pattern = "*Loader_Sequence.PRM";
        constexpr const char* kSequence_Tex_Pattern = "*Loader_Sequence.TEX";
        constexpr const char* kZip_Extension = "zip";
        constexpr const char* kMain_Level_Name = "main";

        // 16-byte allocation granularity used by the original ZSysMem::allocate
        // call sites (size + 15) & ~15.
        constexpr int kAlloc_Align_Mask = ~15;

        // lCompiledGeomOffset is a dword-scaled offset into the GMS body; only its
        // low 24 bits carry the record location (see Get_My_Data).
        constexpr uint32_t kCompiled_Geom_Offset_Mask = 0x00FFFFFFu;

        // InstallTextureBuffer sprite collection.
        constexpr uint16_t kSprite_Record_Type = 2u;      // prim record type the loader keeps (sprite)
        constexpr uint32_t kPrim_Element_Floats = 10u;    // floats per shape element in a prim record
        constexpr uint32_t kCube_Texture_Flag = 0x400u;   // ZBitmap params: cube map (no sprite texture path)

        // Sprite vertex buffer (InstallPrimBuffer / InitGeometry).
        constexpr uint32_t kQuad_Vertex_Buffer_Size = 0x70u; // 4 vertices
        constexpr uint32_t kVertex_Stride = 0x1Cu;
        constexpr uint32_t kVertex_Buffer_Usage = 8u;        // D3DUSAGE_WRITEONLY
        constexpr uint32_t kVertex_FVF = 0x144u;
        constexpr uint32_t kVertex_Color = 0xFFFFFFFFu;

        // Render() texture-factor blending.
        constexpr uint32_t kGray_Channel_Mask = 0x010101u;
        constexpr float kColor_Channel_Scale = 255.0f;

        // Progress() fallback progress-bar color (orange, ARGB).
        constexpr uint32_t kProgress_Bar_Color = 0xFF7F0000u;

        // Thread handshake timing.
        constexpr DWORD kHandshake_Sleep_Ms = 1u;
        constexpr DWORD kRender_Loop_Sleep_Ms = 10u;
        constexpr int kRender_Thread_Priority = 2; // THREAD_PRIORITY_HIGHEST

        // Pool advance granularity applied when the TEX buffer is loaded (Ctor).
        constexpr int kTexture_Pool_Step = 1024;

        // InitGeometry vertex layout: position + w + diffuse + uv (0x1C bytes).
        struct SSequence_Vertex
        {
            float m_Pos[3];
            float m_W;
            uint32_t m_Color;
            float m_UV[2];
        };
        RE_VERIFY_SIZE(SSequence_Vertex, kVertex_Stride);

        // TEX bitmap record header (mirrors the manager's STextureBufferEntry);
        // the ZBitmap::LoadBin() payload follows right after it.
        struct STex_Record_Header
        {
            uint32_t m_lReserved;   // not read by the loaders
            uint32_t m_dwType;      // ZTextureType of the stored bitmap
        };
        RE_VERIFY_SIZE(STex_Record_Header, 0x8);

        // Pending sprite entry collected for one picture. The PC original builds
        // a REFTAB(&tab, 32, 3) (PC 0x42E320) per picture: a chained,
        // append-only table whose Add() never deduplicates, so a plain array
        // bounded by the prim record's sub-offset count is equivalent.
        struct SSequence_Sprite_Entry
        {
            const void* m_pTexture_Record; // TEX bitmap record (TEX buffer base + offset-table entry)
            float m_fPos_X;                // sprite top-left X in design-resolution pixels
            float m_fPos_Y;                // sprite top-left Y in design-resolution pixels
        };
    }

    STATIC_CLASS_VAR_IMPL(ZLoader_Sequence_Wintel_D3D, bool, m_bRender_Enabled, 0x0090DC88, false);
    STATIC_CLASS_VAR_IMPL(ZLoader_Sequence_Wintel_D3D, bool, m_bEnable_Render, 0x0090DC89, false);
    STATIC_CLASS_VAR_IMPL(ZLoader_Sequence_Wintel_D3D, bool, m_bDisable_Render, 0x0090DC8A, false);
    STATIC_CLASS_VAR_IMPL(ZLoader_Sequence_Wintel_D3D, bool, m_bStopIt, 0x0090DC8B, false);
    STATIC_CLASS_VAR_IMPL(ZLoader_Sequence_Wintel_D3D, void*, m_hLoadPictureVBEvent, 0x0090DC8C, nullptr);

    // PC 0x4AE2C0.
    ZLoader_Sequence_Wintel_D3D::ZLoader_Sequence_Wintel_D3D()
    {
        m_hLoadPictureVBEvent = nullptr;
        m_hEvent = nullptr;
        m_hThread = nullptr;
        pReader = nullptr;
        m_pTexture_Pool = static_cast<uint8_t*>(ZUniMemory::Allocate(kTexture_Pool_Size));
        m_fCurrentTime = 0.0f;
        m_aObjects = nullptr;
        m_iTotalObjects = 0;
        m_iTexture_Pool_Step = kTexture_Pool_Step;
        m_pPoolEnd = reinterpret_cast<uint8_t*>(
            (reinterpret_cast<uintptr_t>(m_pTexture_Pool) + kTexture_Pool_Align - 1) &
            ~(static_cast<uintptr_t>(kTexture_Pool_Align) - 1));
        m_bDisable_Render = false;
        m_bEnable_Render = false;
        m_bRender_Enabled = true;
    }

    // PC 0x4AEDD0. Factory for ZLoader_Sequence_Player::m_pCreate_Func.
    ZLoader_Sequence_Player_Base* ZLoader_Sequence_Wintel_D3D::Produce()
    {
        return ZUniMemory::New<ZLoader_Sequence_Wintel_D3D>();
    }

    // PC 0x4AEFC0 (complete object destructor; deleting dtor at 0x4AF090).
    ZLoader_Sequence_Wintel_D3D::~ZLoader_Sequence_Wintel_D3D()
    {
        Free_All();
    }

    // PC 0x4AEE30.
    void ZLoader_Sequence_Wintel_D3D::Free_All()
    {
        m_bStopIt = true;

        if (m_aObjects)
        {
            for (int i = 0; i < m_iTotalObjects; ++i)
            {
                ZGeometryObjectHeader& rHeader = m_aObjects[i];
                for (int j = 0; j < rHeader.iSubObjectCount; ++j)
                {
                    ZGeometrySubObject& rSub = rHeader.pSubObjectsArray[j];
                    if (rSub.m_pUserData)
                        static_cast<IDirect3DTexture9*>(rSub.m_pUserData)->Release();
                    if (rSub.pVertexBuffer)
                        rSub.pVertexBuffer->Release();
                }
            }

            // new[]-style teardown: the element-count cookie lives at
            // (m_aObjects - 4) and each ZGeometryObjectHeader "scalar dtor"
            // (PC 0x4F7380) frees the header's pSubObjectsArray.
            const int iCookieCount = reinterpret_cast<const int*>(m_aObjects)[-1];
            for (int i = 0; i < iCookieCount; ++i)
                ZUniMemory::Free(m_aObjects[i].pSubObjectsArray);
            ZUniMemory::Free(reinterpret_cast<uint8_t*>(m_aObjects) - 4);
            m_aObjects = nullptr;
        }

        ZUniMemory::Delete(pReader);
        pReader = nullptr;

        ZUniMemory::Free(m_pTexture_Pool);
        m_pTexture_Pool = nullptr;
    }

    // PC 0x4AE260. Waits for the sprite script to reach its end, stops the
    // render thread and clears the handshake flags.
    void ZLoader_Sequence_Wintel_D3D::Finish_0()
    {
        if (m_hThread)
        {
            while (!pReader->Is_Finished(m_fCurrentTime))
                ;
            m_bStopIt = true;
            WaitForSingleObject(static_cast<HANDLE>(m_hThread), INFINITE);
            m_hLoadPictureVBEvent = nullptr;
            m_hEvent = nullptr;
            m_hThread = nullptr;
        }
        m_bDisable_Render = false;
        m_bEnable_Render = false;
        m_bRender_Enabled = false;
    }

    // PC 0x4ADDB0. Allocates and fills the runtime script; the parser output is
    // stored directly in pReader (ZLoader_Sequence_Script).
    bool ZLoader_Sequence_Wintel_D3D::LoadScript(const char* pScript)
    {
        if (!pScript)
            return false;

        const size_t uiScript_Length = strlen(pScript);
        if (uiScript_Length == static_cast<size_t>(-1))
            return false;

        pReader = ZUniMemory::New<ZLoader_Sequence_Script>();
        pReader->Load_Script(pScript, static_cast<uint32_t>(uiScript_Length + 1));
        return true;
    }

    // PC 0x4AEF10.
    bool ZLoader_Sequence_Wintel_D3D::Create_Sprites(FsZip_t& rZip, const char* pPicture_Names, uint32_t uiPrim_Base)
    {
        m_iTotalObjects = static_cast<int>(pReader->Get_Nr_Pictures());
        if (!InstallTextureBuffer(rZip, pPicture_Names, uiPrim_Base))
            return false;
        return InstallPrimBuffer();
    }

    // PC 0x4AE3D0. Loads the sequence TEX buffer at the end of the raw pool and,
    // for every script picture, resolves its prim shape record, gathers the
    // referenced sprite bitmaps and creates one ZGeometrySubObject each. The D3D
    // texture is filled by ZTextureManagerD3D::CreateTexture(), which writes the
    // ZTextureBase prefix of the subobject (PC routes this through the one-bitmap
    // wrapper at 0x48D790).
    bool ZLoader_Sequence_Wintel_D3D::InstallTextureBuffer(FsZip_t& rZip, const char* pPicture_Names, uint32_t uiPrim_Base)
    {
        // The name blob starts with an entry count; the names follow right after it.
        const uint32_t uiName_Count = *reinterpret_cast<const uint32_t*>(pPicture_Names);
        const char* const pFirst_Name = pPicture_Names + sizeof(uint32_t);

        // m_aObjects is a new[]-style header array: the element-count cookie lives
        // in the 4 bytes just before the first element.
        const size_t uiArray_Bytes = sizeof(ZGeometryObjectHeader) * static_cast<size_t>(m_iTotalObjects);
        auto* pCount_Cookie = static_cast<int32_t*>(
            ZUniMemory::Allocate(static_cast<int>(uiArray_Bytes + sizeof(int32_t))));
        if (pCount_Cookie)
        {
            *pCount_Cookie = m_iTotalObjects;
            m_aObjects = reinterpret_cast<ZGeometryObjectHeader*>(pCount_Cookie + 1);
        }
        else
        {
            m_aObjects = nullptr;
        }
        memset(m_aObjects, 0, uiArray_Bytes);

        // Load the TEX buffer at the pool end; the buffer base used for every
        // record lookup is saved before m_pPoolEnd is advanced.
        uint8_t* const pTex_Base = m_pPoolEnd;
        rZip.Load(kSequence_Tex_Pattern, pTex_Base, 0, 0);

        const int iTex_Size = rZip.GetSize(kSequence_Tex_Pattern);
        if (iTex_Size == -1)
            return false;

        // Advance the pool end by the TEX size rounded up to the pool's free step.
        const uint32_t uiStep = static_cast<uint32_t>(m_iTexture_Pool_Step);
        m_pPoolEnd += (0u - uiStep) & (uiStep + static_cast<uint32_t>(iTex_Size) - 1u);
        ZASSERT(m_pPoolEnd < m_pTexture_Pool + kTexture_Pool_Size); // PC: __debugbreak on overflow

        // TEX header: the first u32 is the byte offset of the texture-id to
        // record-offset table (see ZTextureManagerD3D TEX buffer format docs).
        const char* const pTex_Buffer = reinterpret_cast<const char*>(pTex_Base);
        const uint32_t* const p_Offset_Table = reinterpret_cast<const uint32_t*>(
            pTex_Buffer + *reinterpret_cast<const uint32_t*>(pTex_Buffer));

        auto* pTex_Manager = static_cast<ZTextureManagerD3D*>(g_pRenderDll->m_pTexCon);

        for (int i = 0; i < m_iTotalObjects; ++i)
        {
            const char* const pPic_Name = pReader->Get_Picture_Name(static_cast<uint32_t>(i));

            // Walk the name blob; each entry is
            //   [char name[]][pad to 4][u32 shape record offset (-1 = none)]
            //   [u32 gap, in u32s, to the next entry][u32 sprite sub-offsets...]
            const char* p_Name_Entry = pFirst_Name;
            bool b_Found = false;
            if (uiName_Count != 0)
            {
                for (uint32_t ui = 0;;)
                {
                    if (strcmp(p_Name_Entry, pPic_Name) == 0)
                    {
                        b_Found = true;
                        break;
                    }
                    if (++ui == uiName_Count)
                        break;
                    const uint32_t ui_Aligned = (static_cast<uint32_t>(strlen(p_Name_Entry)) + 4u) & ~3u;
                    p_Name_Entry += ui_Aligned + 2u * sizeof(uint32_t) + sizeof(uint32_t) *
                        *reinterpret_cast<const uint32_t*>(p_Name_Entry + ui_Aligned + sizeof(uint32_t));
                }
            }
            if (!b_Found)
                continue;

            const uint32_t ui_Aligned = (static_cast<uint32_t>(strlen(p_Name_Entry)) + 4u) & ~3u;
            const uint32_t* const p_Name_Header = reinterpret_cast<const uint32_t*>(p_Name_Entry + ui_Aligned);
            const int32_t iRecord_Offset = static_cast<int32_t>(p_Name_Header[0]);
            if (iRecord_Offset == -1)
                continue; // picture carries no sprite data (header already zeroed)

            const uint32_t* const p_Sub_Table = p_Name_Header + 2;
            ZASSERT(p_Sub_Table != nullptr);

            // Shape record in the prim buffer:
            //   [u32 element count][elements x 10 floats][u32 sub-offset count]
            const char* const p_Record = reinterpret_cast<const char*>(uiPrim_Base) + iRecord_Offset;
            const uint32_t ui_Element_Count = *reinterpret_cast<const uint32_t*>(p_Record);
            const float* const p_Elements = reinterpret_cast<const float*>(p_Record + sizeof(uint32_t));
            const uint32_t ui_Sub_Count = *reinterpret_cast<const uint32_t*>(
                p_Record + sizeof(uint32_t) + kPrim_Element_Floats * sizeof(float) * ui_Element_Count);

            // Keep sub-offsets that reference a type-2 (sprite) record with a
            // resolvable texture id.
            SSequence_Sprite_Entry* p_Entries = nullptr;
            if (ui_Sub_Count != 0)
            {
                p_Entries = static_cast<SSequence_Sprite_Entry*>(
                    ZUniMemory::Allocate(static_cast<int>(sizeof(SSequence_Sprite_Entry) * ui_Sub_Count)));
            }

            uint32_t ui_Sprite_Count = 0;
            for (uint32_t j = 0; j < ui_Sub_Count; ++j)
            {
                const uint32_t ui_Sprite_Offset = p_Sub_Table[j];
                if (ui_Sprite_Offset == 0)
                    continue;

                // Sprite record in the prim buffer: u16 type at +2, u16 texture id at +4.
                const char* const p_Sprite = reinterpret_cast<const char*>(uiPrim_Base) + ui_Sprite_Offset;
                if (*reinterpret_cast<const uint16_t*>(p_Sprite + 2) != kSprite_Record_Type)
                    continue;

                const uint32_t ui_Tex_Offset = p_Offset_Table[*reinterpret_cast<const uint16_t*>(p_Sprite + 4)];
                if (ui_Tex_Offset == 0)
                    continue;

                // Shape element floats: [0] center X, [1] center Y, [7] width,
                // [8] height (the Y-up center half-extents). Verified against the
                // Loader_Sequence prim data: [6] is unused (0), and using [7]/[8]
                // reproduces the on-screen sprite tiling exactly; reading [6]/[7]
                // shifts every sprite half a width to the right.
                const float* const p_Element = p_Elements + kPrim_Element_Floats * j;
                p_Entries[ui_Sprite_Count].m_pTexture_Record = pTex_Buffer + ui_Tex_Offset;
                p_Entries[ui_Sprite_Count].m_fPos_X = p_Element[0] - p_Element[7] * 0.5f;
                p_Entries[ui_Sprite_Count].m_fPos_Y = -p_Element[1] - p_Element[8] * 0.5f;
                ++ui_Sprite_Count;
            }

            ZGeometryObjectHeader& rHeader = m_aObjects[i];
            rHeader.iSubObjectCount = static_cast<int>(ui_Sprite_Count);
            if (ui_Sprite_Count != 0)
            {
                rHeader.pSubObjectsArray = static_cast<ZGeometrySubObject*>(
                    ZUniMemory::Allocate(static_cast<int>(sizeof(ZGeometrySubObject) * ui_Sprite_Count)));
                memset(rHeader.pSubObjectsArray, 0, sizeof(ZGeometrySubObject) * ui_Sprite_Count);
            }

            // One reusable stack bitmap per TEX record type (the PC original
            // constructs all seven per picture; LoadBin resets the previous state).
            ZBitmap32 bitmap32;
            ZBitmapU8V8 bitmapU8V8;
            ZBitmapI8 bitmapI8;
            ZBitmapPal bitmapPal;
            ZBitmapPalOpac bitmapPalOpac;
            ZBitmapDXT1 bitmapDXT1;
            ZBitmapDXT3 bitmapDXT3;

            for (uint32_t k = 0; k < ui_Sprite_Count; ++k)
            {
                ZGeometrySubObject& rSub = rHeader.pSubObjectsArray[k];
                rSub.m_iCustomParamX = static_cast<int32_t>(p_Entries[k].m_fPos_X);
                rSub.m_iCustomParamY = static_cast<int32_t>(p_Entries[k].m_fPos_Y);

                const auto* p_Rec_Header = static_cast<const STex_Record_Header*>(p_Entries[k].m_pTexture_Record);
                const char* const p_Bitmap_Data = reinterpret_cast<const char*>(p_Rec_Header + 1);
                const uint32_t dw_Type = p_Rec_Header->m_dwType;

                ZBitmap* p_Bitmap = nullptr;
                if (dw_Type > BITMAP_PAL)
                {
                    switch (dw_Type)
                    {
                    case BITMAP_PAL_OPAC:
                        bitmapPalOpac.LoadBin(p_Bitmap_Data);
                        p_Bitmap = &bitmapPalOpac;
                        break;
                    case BITMAP_32:
                        bitmap32.LoadBin(p_Bitmap_Data);
                        p_Bitmap = &bitmap32;
                        break;
                    case BITMAP_U8V8:
                        bitmapU8V8.LoadBin(p_Bitmap_Data);
                        p_Bitmap = &bitmapU8V8;
                        break;
                    default:
                        ZASSERT(false);
                        break;
                    }
                }
                else if (dw_Type == BITMAP_PAL)
                {
                    bitmapPal.LoadBin(p_Bitmap_Data);
                    p_Bitmap = &bitmapPal;
                }
                else if (dw_Type == BITMAP_DXT1)
                {
                    bitmapDXT1.LoadBin(p_Bitmap_Data);
                    p_Bitmap = &bitmapDXT1;
                }
                else if (dw_Type == BITMAP_DXT3)
                {
                    bitmapDXT3.LoadBin(p_Bitmap_Data);
                    p_Bitmap = &bitmapDXT3;
                }
                else if (dw_Type == BITMAP_I8)
                {
                    bitmapI8.LoadBin(p_Bitmap_Data);
                    p_Bitmap = &bitmapI8;
                }
                else
                {
                    ZASSERT(false);
                }

                // Cube bitmaps have no sprite path on PC; the D3D texture lands in
                // the subobject's ZTextureBase::m_pUserData.
                if (p_Bitmap && (p_Bitmap->GetParams() & kCube_Texture_Flag) == 0)
                    pTex_Manager->CreateTexture(p_Bitmap, reinterpret_cast<ZTextureD3D*>(&rSub));
            }

            if (p_Entries)
                ZUniMemory::Free(p_Entries);
        }
        return true;
    }

    // PC 0x4AE1F0. One write-only managed vertex buffer (triangle-strip quad)
    // per sprite.
    bool ZLoader_Sequence_Wintel_D3D::InstallPrimBuffer()
    {
        if (!pReader)
            return false;

        for (int i = 0; i < m_iTotalObjects; ++i)
        {
            ZGeometryObjectHeader& rHeader = m_aObjects[i];
            for (int j = 0; j < rHeader.iSubObjectCount; ++j)
            {
                g_pd3dDevice->CreateVertexBuffer(
                    kQuad_Vertex_Buffer_Size,
                    kVertex_Buffer_Usage,
                    kVertex_FVF,
                    D3DPOOL_MANAGED,
                    &rHeader.pSubObjectsArray[j].pVertexBuffer,
                    nullptr);
            }
        }
        return true;
    }

    // PC 0x4ADBF0 (__stdcall helper, no this).
    void ZLoader_Sequence_Wintel_D3D::Get_Prim_Data(void** ppBuffer, int* pSize, FsZip_t& rZip)
    {
        *ppBuffer = nullptr;
        const int iSize = rZip.GetSize(kSequence_Prm_Pattern);
        *pSize = iSize;
        if (iSize > 0)
        {
            void* pBuffer = ZUniMemory::Allocate((iSize + 15) & kAlloc_Align_Mask);
            *ppBuffer = pBuffer;
            rZip.Load(kSequence_Prm_Pattern, pBuffer, 0, 0);
        }
    }

    // PC 0x4ADD60 (__stdcall helper, no this). Scans the GMS entity table for
    // the first ZLoader_Sequence_Setup entity and returns its prim payload.
    const char* ZLoader_Sequence_Wintel_D3D::Get_My_Data(const char* pGms_Buffer, const char* pPrim_Buffer)
    {
        // The GMS body is an SPackedGeomsHeader; its lEntriesOffset points at the
        // entity table { u32 lTotalGeoms; SPackedGeomsTree aGeoms[lTotalGeoms] }.
        const auto* pHeader = reinterpret_cast<const SPackedGeomsHeader*>(pGms_Buffer);
        const auto* pEntries = reinterpret_cast<const uint32_t*>(pGms_Buffer + pHeader->lEntriesOffset);
        const uint32_t uiTotal_Geoms = pEntries[0];
        const auto* pGeoms = reinterpret_cast<const SPackedGeomsTree*>(pEntries + 1);

        for (uint32_t i = 0; i < uiTotal_Geoms; ++i)
        {
            // lCompiledGeomOffset is a dword-scaled offset to the record's
            // SCompiledGeom entry.
            const auto* pCompiledGeom = reinterpret_cast<const SCompiledGeom*>(
                pGms_Buffer + 4 * (pGeoms[i].lCompiledGeomOffset & kCompiled_Geom_Offset_Mask));
            if (pCompiledGeom->lGeomType == ZLoader_Sequence_Setup::m_TypeId)
                return pPrim_Buffer + pCompiledGeom->lPrim;
        }
        return nullptr;
    }

    // PC 0x4ADC50. Loads and unzips "*Loader_Sequence.GMS" from the pack.
    void ZLoader_Sequence_Wintel_D3D::Get_Geoms_Data(char** ppGeoms_Data, int* pGeoms_Data_Size, FsZip_t& rZip)
    {
        *ppGeoms_Data = nullptr;
        *pGeoms_Data_Size = 0;

        const int iSize = rZip.GetSize(kSequence_Gms_Pattern);
        if (iSize <= 0)
            return;

        // The GMS file opens with a ZPackedDataChunk stream header; the full GMS
        // entity format lives beyond it in the unzipped payload.
        auto* pHeader = static_cast<ZPackedDataChunk::SPacked_Stream_Header*>(
            ZUniMemory::Allocate((iSize + 15) & kAlloc_Align_Mask));
        rZip.Load(kSequence_Gms_Pattern, pHeader, 0, 0);

        ZPackedDataChunk Chunk;
        Chunk.m_bCustomBuffer = true; // the header buffer below stays owned here
        Chunk.m_pPackedData = reinterpret_cast<uint8_t*>(pHeader);
        Chunk.m_iRawDataLength = pHeader->m_iRaw_Size;
        Chunk.m_iPackedDataLength = pHeader->m_iPacked_Size;
        Chunk.m_eCompression = static_cast<ZPackedDataChunk::eCompressionType>(pHeader->m_eCompression);

        *pGeoms_Data_Size = pHeader->m_iRaw_Size;
        char* pRaw = static_cast<char*>(ZUniMemory::Allocate((pHeader->m_iRaw_Size + 15) & kAlloc_Align_Mask));
        *ppGeoms_Data = pRaw;
        Chunk.unzip(pRaw, (*pGeoms_Data_Size + 15) & kAlloc_Align_Mask);

        ZUniMemory::Free(pHeader);
    }

    // PC 0x4ADE60 (reconstructed). Builds the full path of a loader-sequence
    // side file: take the current scene path up to its last separator, append
    // the file name, and prefer the cache-resolved .zip variant when it exists.
    // The PC decompile contains a double CalcCacheFileName artifact; both calls
    // receive identical arguments, so the effective result is the one below.
    MYSTR ZLoader_Sequence_Wintel_D3D::Calc_Total_Name(const char* pFile_Name) const
    {
        char szPath[MAX_PATH];
        strcpy(szPath, g_pEngineData->GetSceneName());

        char* p_Separator = strrchr(szPath, '\\');
        if (p_Separator)
            strcpy(p_Separator + 1, pFile_Name);
        else
            strcpy(szPath, pFile_Name);

        MYSTR s_Cached = g_pEngineData->CalcCacheFileName(MYSTR(szPath), kZip_Extension);
        if (g_pSysFile->Exists(static_cast<const char*>(s_Cached), false))
            return s_Cached;
        return MYSTR(szPath);
    }

    // PC 0x4AF010.
    bool ZLoader_Sequence_Wintel_D3D::Create_Render_Thread()
    {
        m_hLoadPictureVBEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);
        m_hEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);
        m_fCurrentTime = 0.0f;
        m_liStartTick = g_pSysInterface->TimeStampCounter(__FILE__, __LINE__);
        m_bStopIt = false;

        DWORD dwThread_Id = 0;
        HANDLE hThread = CreateThread(
            nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Render_Thread), this, 0, &dwThread_Id);
        m_hThread = hThread;
        SetThreadPriority(hThread, kRender_Thread_Priority);
        return true;
    }

    // PC 0x4AEF50. Render loop; the disable/enable handshake pairs with
    // Disable_Render()/Enable_Render() so the D3D device is only touched here.
    uint32_t __stdcall ZLoader_Sequence_Wintel_D3D::Render_Thread(void* pParam)
    {
        while (!m_bStopIt)
        {
            if (m_bDisable_Render)
            {
                m_bRender_Enabled = false;
                m_bDisable_Render = false;
            }
            if (m_bEnable_Render)
            {
                m_bRender_Enabled = true;
                m_bEnable_Render = false;
            }
            if (m_bRender_Enabled)
                static_cast<ZLoader_Sequence_Wintel_D3D*>(pParam)->Render();
            Sleep(kRender_Loop_Sleep_Ms);
        }
        ExitThread(0);
    }

    // PC 0x4AEBD0. Draws every sprite of the script with a texture-factor
    // (alpha << 24 | gray replicated) modulation.
    void ZLoader_Sequence_Wintel_D3D::Render()
    {
        if (!pReader)
        {
            ZASSERT(false); // PC: __debugbreak
            return;
        }

        ZRender* pWindow = g_pSysInterface->WindowFirst;
        pWindow->BeginScene();
        g_pd3dDevice->ResetState();

        for (int i = 0; i < m_iTotalObjects; ++i)
        {
            for (int j = 0; j < m_aObjects[i].iSubObjectCount; ++j)
                InitGeometry(i, j);
        }

        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0u, 1.0f, 0u);

        for (int i = 0; i < m_iTotalObjects; ++i)
        {
            // NOTE: PC's IDA database misnames the two script getters; this is
            // the alpha getter (0x4677C0) ...
            const int iAlpha = static_cast<int>(pReader->Get_Opacity(i, m_fCurrentTime) * kColor_Channel_Scale);
            if (!iAlpha)
                continue;

            // ... and the gray multiplier (0x4678A0).
            const int iGray = static_cast<int>(pReader->Get_Multiply(i, m_fCurrentTime) * kColor_Channel_Scale);
            g_pd3dDevice->SetRenderState(
                D3DRS_TEXTUREFACTOR,
                static_cast<DWORD>((iAlpha << 24) + kGray_Channel_Mask * iGray));

            ZGeometryObjectHeader& rHeader = m_aObjects[i];
            for (int j = 0; j < rHeader.iSubObjectCount; ++j)
            {
                ZGeometrySubObject& rSub = rHeader.pSubObjectsArray[j];
                g_pd3dDevice->SetTexture(0, static_cast<IDirect3DBaseTexture9*>(rSub.m_pUserData));
                g_pd3dDevice->SetStreamSource(0, rSub.pVertexBuffer, 0, kVertex_Stride);
                g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2u);
            }
        }

        g_pd3dDevice->SetTexture(0, nullptr);
        g_pd3dDevice->SetStreamSource(0, nullptr, 0, 0);

        pWindow->EndScene();
        pWindow->Flip();

        m_fCurrentTime = static_cast<float>(
            static_cast<double>(g_pSysInterface->TimeStampCounter(__FILE__, __LINE__) - m_liStartTick) /
            g_pSysInterface->CycSec);
    }

    // PC 0x4AE930. Rewrites the sprite's triangle-strip quad so the position
    // and size follow the time-interpolated script values. The Get_PosX/Y calls
    // mutate the script's time adjustment, so the PC call order is preserved.
    void ZLoader_Sequence_Wintel_D3D::InitGeometry(int iObject_Index, int iSubObject_Index)
    {
        if (!pReader)
            return;

        ZGeometrySubObject& rSub = m_aObjects[iObject_Index].pSubObjectsArray[iSubObject_Index];
        if (!rSub.pVertexBuffer)
        {
            ZASSERT(false); // PC: __debugbreak
            return;
        }

        SSequence_Vertex* pVertices = nullptr;
        if (rSub.pVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertices), 0) < 0)
            return;

        ZRender* pWindow = g_pSysInterface->WindowFirst;

        const int iSize_X = pWindow->GetSizeX();
        const float fBase_X = pReader->Get_PosX(iObject_Index, m_fCurrentTime);
        const float fScale_X = static_cast<float>(static_cast<double>(iSize_X) / fBase_X);

        const int iSize_Y = pWindow->GetSizeY();
        const float fBase_Y = pReader->Get_PosY(iObject_Index, m_fCurrentTime);
        const float fScale_Y = static_cast<float>(static_cast<double>(iSize_Y) / fBase_Y);

        // PC multiplies size and custom offset in double precision and rounds
        // only once; the custom params convert to double as unsigned 32-bit.
        const float fWidth = static_cast<float>(static_cast<double>(rSub.m_usSize[0]) * fScale_X);
        const float fHeight = static_cast<float>(static_cast<double>(rSub.m_usSize[1]) * fScale_Y);

        const float fX = static_cast<float>(
            static_cast<double>(static_cast<uint32_t>(rSub.m_iCustomParamX)) * fScale_X +
            pReader->Get_PosX(iObject_Index, m_fCurrentTime));
        const float fY = static_cast<float>(
            static_cast<double>(static_cast<uint32_t>(rSub.m_iCustomParamY)) * fScale_Y +
            pReader->Get_PosY(iObject_Index, m_fCurrentTime));

        pVertices[0] = { fX,           fY,         0.0f, 1.0f, kVertex_Color, 0.0f, 0.0f };
        pVertices[1] = { fX,           fY + fHeight, 0.0f, 1.0f, kVertex_Color, 0.0f, 1.0f };
        pVertices[2] = { fX + fWidth,  fY,         0.0f, 1.0f, kVertex_Color, 1.0f, 0.0f };
        pVertices[3] = { fX + fWidth,  fY + fHeight, 0.0f, 1.0f, kVertex_Color, 1.0f, 1.0f };

        rSub.pVertexBuffer->Unlock();
    }

    // PC 0x4ADB70. Waits until the render thread acknowledges the pause.
    void ZLoader_Sequence_Wintel_D3D::Disable_Render()
    {
        if (pReader && m_bRender_Enabled)
        {
            m_bDisable_Render = true;
            while (m_bRender_Enabled || m_bDisable_Render)
                Sleep(kHandshake_Sleep_Ms);
        }
    }

    // PC 0x4ADBB0. Waits until the render thread acknowledges the resume.
    void ZLoader_Sequence_Wintel_D3D::Enable_Render()
    {
        if (pReader && !m_bRender_Enabled)
        {
            m_bEnable_Render = true;
            while (!m_bRender_Enabled || m_bEnable_Render)
                Sleep(kHandshake_Sleep_Ms);
        }
    }

    // PC 0x4AE360.
    void ZLoader_Sequence_Wintel_D3D::Finish()
    {
        Progress(1.0f);
        Finish_0();

        // Present one final cleared frame (black) twice.
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET, 0u, 1.0f, 0u);
        g_pSysInterface->WindowFirst->Flip();
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET, 0u, 1.0f, 0u);
        g_pSysInterface->WindowFirst->Flip();
    }

    // PC 0x4ADAF0. With a loaded script the progress drives the sprite
    // timeline; before that it paints the plain progress bar.
    void ZLoader_Sequence_Wintel_D3D::Progress(float fProgress)
    {
        // CLAMP(0, 1, fProgress)
        if (fProgress >= 0.0f)
        {
            if (fProgress > 1.0f)
                fProgress = 1.0f;
        }
        else
        {
            fProgress = 0.0f;
        }

        if (pReader)
        {
            pReader->Set_Progress(fProgress);
        }
        else if (fProgress != 0.0f)
        {
            ZRender* pWindow = g_pSysInterface->WindowFirst;
            if (pWindow)
                pWindow->ProgressBar(fProgress, kProgress_Bar_Color, kProgress_Bar_Color);
        }
    }

    // PC 0x4AF0B0. Boot-up: black double-flip, then load the per-level
    // Loader_Sequence.ZIP (script + sprites + prim buffer) and start the
    // render thread. Any failure unwinds through Free_All().
    void ZLoader_Sequence_Wintel_D3D::Start()
    {
        m_bDisable_Render = false;
        m_bEnable_Render = false;
        m_bRender_Enabled = true;
        m_bStopIt = false;

        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET, 0u, 1.0f, 0u);
        g_pSysInterface->WindowFirst->Flip();
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET, 0u, 1.0f, 0u);
        g_pSysInterface->WindowFirst->Flip();

        if (strcmp(static_cast<const char*>(g_pSysInterface->m_sDefaultScene), kMain_Level_Name) == 0)
            return; // main scene has no loader sequence

        MYSTR s_Seq_File = Calc_Total_Name(kSequence_Zip_File);
        if (!g_pSysFile->Exists(static_cast<const char*>(s_Seq_File), false))
            return;

        FsZip_t Zip;
        Zip.initFS(static_cast<const char*>(s_Seq_File), Glacier::IOFS_READONLY);

        char* pGeoms_Data = nullptr;
        int iGeoms_Data_Size = 0;
        Get_Geoms_Data(&pGeoms_Data, &iGeoms_Data_Size, Zip);

        void* pPrim_Data = nullptr;
        int iPrim_Data_Size = 0;
        Get_Prim_Data(&pPrim_Data, &iPrim_Data_Size, Zip);

        bool bSuccess = false;
        if (pGeoms_Data && pPrim_Data)
        {
            const char* pScript = Get_My_Data(pGeoms_Data, static_cast<const char*>(pPrim_Data));
            if (pScript &&
                LoadScript(pScript) &&
                pReader &&
                pReader->Get_Nr_Pictures() &&
                Create_Sprites(Zip,
                               pScript + ((strlen(pScript) + 4) & ~3u),
                               static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pPrim_Data))))
            {
                ZUniMemory::Free(pGeoms_Data);
                pGeoms_Data = nullptr;
                ZUniMemory::Free(pPrim_Data);
                pPrim_Data = nullptr;
                bSuccess = Create_Render_Thread();
            }
        }

        ZUniMemory::Free(pGeoms_Data);
        ZUniMemory::Free(pPrim_Data);

        if (!bSuccess)
            Free_All();
    }

    namespace
    {
        // PC static-init 0x73B220: register the Wintel/D3D player factory.
        struct SZLoader_Sequence_Wintel_D3D_Register
        {
            SZLoader_Sequence_Wintel_D3D_Register()
            {
                ZLoader_Sequence_Player::m_pCreate_Func = &ZLoader_Sequence_Wintel_D3D::Produce;
            }
        };
        SZLoader_Sequence_Wintel_D3D_Register g_Loader_Sequence_Wintel_D3D_Register;
    }
}
