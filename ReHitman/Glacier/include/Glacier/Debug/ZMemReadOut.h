#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Component/ZComponentSingleton.h>
#include <Glacier/Component/ZGlobalComponentBase.h>
#include <Glacier/System/ZSysmemDebugHandler.h>
#include <cstdint>


namespace Glacier
{
    static constexpr uint32_t GEOMBASE_MEM_COLOR = 0xAF00AFu;
    static constexpr uint32_t EVENTBASE_MEM_COLOR = 0x40C0FFu;

    class ZMemReadOut : public ZComponentSingleton<ZMemReadOut, ZSysmemDebugHandler>
    {
    public:
        // types
        struct SMemBlock
        {
            uint16_t m_iUsed;
            uint16_t m_iColor;
        };

        // static
        static const uint32_t m_NRMEMORYCOLORS;
        static const uint32_t m_iNrBlocks;
        static size_t s_sizeAllocated;
        static size_t s_maxAllocated;

        // vtbl
        virtual ~ZMemReadOut();
        virtual void PreAllocChange(uint32_t&) override;
        virtual void PostAllocChange(char*&, uint32_t&) override;
        virtual void PreFreeChange(char*&) override;
        virtual void PostAlloc(char* pStart, uint32_t iSize, char* pRealStart, char* pRealEnd) override;
        virtual void PreFree(char* pStart) override;
        virtual void PreShrinkChange(char*& pStart, uint32_t& lNewSize) override;
        virtual void Reset() override;

        // methods
        ZMemReadOut();

        void MarkReadoutBlock(char* pStart, char* pEnd);
        void UnMarkReadoutBlock(char* pStart, const char* pEnd);
        void OverrideMemColors(char* pStart, uint32_t lSize, uint32_t lColor);
        uint32_t SetAllocColor(uint32_t lColor);
        void GetTotalPrCol(uint32_t* pSumBuffer);
        void PrintStatus();
        uint32_t GetNumNonContBlocks();
        uint32_t GetBlockSize() const;
        const ZMemReadOut::SMemBlock* GetNonContMemoryBlocks();

        // members
        ZMemReadOut::SMemBlock m_ReadOutBlock[8192];
        uint32_t m_iBlockSize;
        uint32_t m_iColor;
        uint32_t m_iAllocColor;
        const char* m_pStartAddr;
        uint32_t m_iMemorySize;
    };
    RE_VERIFY_OFFSET(ZMemReadOut, m_iColor, 0x8014);
}
