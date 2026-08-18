#include <Glacier/Debug/ZMemReadOut.h>
#include <Glacier/System/ZSysMem.h>


namespace Glacier
{
    namespace 
    {
        // SysMemReadOutInfo
        struct SysMemReadOutInfo
        {
            const char* pzName;
            uint32_t lColor;
        };

        RE_VERIFY_SIZE(SysMemReadOutInfo, 0x8);

        static constexpr int SYS_MEM_INFO_NR = 42;

        // PC at 0x007F4F58
        static SysMemReadOutInfo g_SysMemInfo[SYS_MEM_INFO_NR]
        {
            SysMemReadOutInfo{"SCE Reserved", 0x800000},
            SysMemReadOutInfo{"Reserved SCE alloc", 0x0A04000},
            SysMemReadOutInfo{"Used SCE alloc", 0x0FFC000},
            SysMemReadOutInfo{"Global constructors", 0x0CDC0C0},
            SysMemReadOutInfo{"Code", 0x8000},
            SysMemReadOutInfo{"Scripts", 0x8F0FFF},
            SysMemReadOutInfo{"System", 0x0FF8000},
            SysMemReadOutInfo{"Draw buffers", 0x0FF60A0},
            SysMemReadOutInfo{"DMA draw", 0x0F00000},
            SysMemReadOutInfo{"DMA draw used", 0x0FF8080},
            SysMemReadOutInfo{"Visible", 0x8040F0},
            SysMemReadOutInfo{"Visible used", 0x0C070FF},
            SysMemReadOutInfo{"Geometry", 0x0E0E000},
            SysMemReadOutInfo{"Shadows", 0x0F0F000},
            SysMemReadOutInfo{"Textures", 0x0E0E0},
            SysMemReadOutInfo{"StaticBuffer", 0x0FF0080},
            SysMemReadOutInfo{"BaseGeoms free", 0x7F007F},
            SysMemReadOutInfo{"BaseGeoms used", 0x0AF00AF},
            SysMemReadOutInfo{"ExtraGeoms free", 0x7F0000},
            SysMemReadOutInfo{"ExtraGeoms used", 0x0AF0000},
            SysMemReadOutInfo{"Geom Lists", 0x4F0000},
            SysMemReadOutInfo{"Events free", 0x20A0FF},
            SysMemReadOutInfo{"Events used", 0x40C0FF},
            SysMemReadOutInfo{"Bone Anims", 0x808040},
            SysMemReadOutInfo{"Room Bound Trees", 0x4080A0},
            SysMemReadOutInfo{"Octrees", 0x8080FF},
            SysMemReadOutInfo{"Path Finder", 0x0FF00},
            SysMemReadOutInfo{"Initialize geoms", 0x0A0A0A0},
            SysMemReadOutInfo{"CreateGCreate geomseoms", 0x606060},
            SysMemReadOutInfo{"Lights Info", 0x20FF7F},
            SysMemReadOutInfo{"Sound Graph", 0x7777},
            SysMemReadOutInfo{"Sound Structs", 0x0FF7777},
            SysMemReadOutInfo{"Reserved Stack", 0x4040A0},
            SysMemReadOutInfo{"Used Stack", 0x0FF},
            SysMemReadOutInfo{"RunTime", 0x0FFFFFF},
            SysMemReadOutInfo{"GlacierAlloc", 0x80FF80},
            SysMemReadOutInfo{"Render init", 0x0FFFF20},
            SysMemReadOutInfo{"Static game data", 0x0FFFF40},
            SysMemReadOutInfo{"Script vars", 0x0FFFF60},
            SysMemReadOutInfo{"Enginedata init", 0x0FFFF80},
            SysMemReadOutInfo{"Enginedata startup", 0x0FFFFA0},
            SysMemReadOutInfo{"Sound Dll init", 0x0FFFFC0}
        };
    }

    ZMemReadOut::ZMemReadOut()
    {
        // TODO: Finish me
        SetMemColor(0x808080u);

        ZSysMem::Instance().AddDebugHandler(this);
    }

    ZMemReadOut::~ZMemReadOut() = default;

    void ZMemReadOut::PreAllocChange(uint32_t&)
    {
        // TODO: Finish me
    }

    void ZMemReadOut::PostAllocChange(char*&, uint32_t&)
    {
        // TODO: Finish me
    }

    void ZMemReadOut::PreFreeChange(char*&)
    {
        // TODO: Finish me
    }

    void ZMemReadOut::PostAlloc(char* pStart, uint32_t iSize, char* pRealStart, char* pRealEnd)
    {
        // TODO: Finish me
    }

    void ZMemReadOut::PreFree(char* pStart)
    {
        // TODO: Finish me
    }

    void ZMemReadOut::PreShrinkChange(char*& pStart, uint32_t& lNewSize)
    {
        // TODO: Finish me
    }

    void ZMemReadOut::Reset()
    {
        // TODO: Finish me
    }

    void ZMemReadOut::MarkReadoutBlock(char* pStart, char* pEnd)
    {
        // TODO: Finish me
    }

    void ZMemReadOut::UnMarkReadoutBlock(char* pStart, const char* pEnd)
    {
        // TODO: Finish me
    }

    void ZMemReadOut::OverrideMemColors(char* pStart, uint32_t lSize, uint32_t lColor)
    {
        // TODO: Finish me
    }

    uint32_t ZMemReadOut::SetAllocColor(uint32_t lColor)
    {
        const auto lOldColor = m_iColor;
        m_iColor = lColor;
        m_iAllocColor = (lColor & 0xF8 | ((lColor & 0xF800 | ((lColor & 0xF80000 | (lColor >> 7) & 0x1000000) >> 3)) >> 3)) >> 3;

        return lOldColor;
    }

    void ZMemReadOut::GetTotalPrCol(uint32_t* pSumBuffer)
    {
        if (!pSumBuffer)
            return;
            
        std::memset(pSumBuffer, 0, sizeof(uint32_t) * SYS_MEM_INFO_NR);

        uint16_t aPackedColors[SYS_MEM_INFO_NR];
        for (int i = 0; i < SYS_MEM_INFO_NR; ++i)
        {
            const uint32_t color = g_SysMemInfo[i].lColor;
            aPackedColors[i] = static_cast<uint16_t>(
                ((color >> 16) & 0x8000) |
                ((color >> 3)  & 0x001F) |
                ((color >> 6)  & 0x03E0) |
                ((color >> 9)  & 0x7C00)
            );
        }

        constexpr uint32_t kMaxBlocks = 8192; // m_iNrBlocks
        for (uint32_t i = 0; i < kMaxBlocks; ++i)
        {
            const uint16_t blockColor = m_ReadOutBlock[i].m_iColor;
            
            for (int j = 0; j < SYS_MEM_INFO_NR; ++j)
            {
                if (aPackedColors[j] == blockColor)
                {
                    pSumBuffer[j] += m_ReadOutBlock[i].m_iUsed;
                    break;
                }
            }
        }
    }

    void ZMemReadOut::PrintStatus(const char* pszTitle)
    {
        printf("Begin PrintMemoryUsed\n");
        printf("---------------------------\n");

        uint32_t aColorSums[SYS_MEM_INFO_NR];
        GetTotalPrCol(aColorSums);

        uint32_t iTotalUsedBytes = 0;
        constexpr double kBytesToMb = 1.0 / (1024.0 * 1024.0); // 0.00000095367431640625

        for (int i = 0; i < SYS_MEM_INFO_NR; ++i)
        {
            const char* pzName = g_SysMemInfo[i].pzName;
            const uint32_t iBytes = aColorSums[i];
            iTotalUsedBytes += iBytes;

            char szAlignedName[21];
            std::memset(szAlignedName, ' ', 20);
            szAlignedName[20] = '\0';

            const size_t nameLen = std::strlen(pzName);
            std::memcpy(szAlignedName, pzName, std::min(nameLen, static_cast<size_t>(20)));

            char szBuffer[1024];
            std::snprintf(szBuffer, sizeof(szBuffer), "%s%.3fMb (%u)\n", 
                        szAlignedName, static_cast<double>(iBytes) * kBytesToMb, iBytes);
            printf("%s", szBuffer);
        }

        char szDest[1024];
        std::snprintf(szDest, sizeof(szDest), "Total               %2.3fMb (%u)\n", 
                    static_cast<double>(iTotalUsedBytes) * kBytesToMb, iTotalUsedBytes);
        printf("%s", szDest);
        printf("---------------------------\n");

        // Get allocator
        auto& pSysMem = ZSysMem::Instance();

        ZAllocatorBase* pGlobalAllocator = pSysMem.GetAllocator(DEFAULT_MEM); 
        const int iFreeTotal = pGlobalAllocator->GetFreeTotal();
        const int iLargestBlock = pGlobalAllocator->GetLargestBlock();

        std::snprintf(szDest, sizeof(szDest), "Free Mem           %6.3fMb (%d)\n", 
                    static_cast<double>(iFreeTotal) * kBytesToMb, iFreeTotal);
        printf("%s", szDest);

        std::snprintf(szDest, sizeof(szDest), "Largest Block      %6.3fMb (%d)\n", 
                    static_cast<double>(iLargestBlock) * kBytesToMb, iLargestBlock);
        printf("%s", szDest);

        const int iFreeMemAt1024 = iFreeTotal + 0x3C000000;
        std::snprintf(szDest, sizeof(szDest), "Free Mem at %d MB  %6.3fMb (%d)\n", 
                    1024, static_cast<double>(iFreeMemAt1024) * kBytesToMb, iFreeMemAt1024);
        printf("%s", szDest);

        const int iDiff = 0x40000000 - iFreeMemAt1024 - iTotalUsedBytes;
        std::snprintf(szDest, sizeof(szDest), "Diff:               %6.3fMb (%d)\n", 
                    static_cast<double>(iDiff) * kBytesToMb, iDiff);
        printf("%s", szDest);

        printf("---------------------------\n");
        printf("End PrintMemoryUsed\n");
    }
    
    uint32_t ZMemReadOut::GetNumNonContBlocks()
    {
        // TODO: Finish me
        return 0;
    }
    
    uint32_t ZMemReadOut::GetBlockSize() const
    {
        return m_iBlockSize;
    }

    const ZMemReadOut::SMemBlock* ZMemReadOut::GetNonContMemoryBlocks()
    {
        // TODO: Finish me
        return nullptr;
    }

    template <>
    ZMemReadOut* ZComponentSingleton<ZMemReadOut, ZSysmemDebugHandler>::m_pInstance = nullptr;
}