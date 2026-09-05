#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    class ZBitmap
    {
    public:
        // types
        struct TDataBlock
        {
            int dwPosX{};
            int dwPosY{};
            int dwSizeX{};
            int dwSizeY{};
            int dwPich{};
            int dwMaskR{};
            int dwMaskG{};
            int dwMaskB{};
            int dwMaskA{};
            int dwBitCount{};
            bool bInvertAlpha{};
            RE_ADD_PADDING(3);
            void* pData { nullptr };
        };
        RE_VERIFY_SIZE(TDataBlock, 0x30);

        struct TMipLevel
        {
            uint32_t dwSize {0};
            int dwSizeX{0};
            int dwSizeY{0};
            bool bAllocated {false};
            RE_ADD_PADDING(3);
            void* pData{nullptr};
        };
        RE_VERIFY_SIZE(TMipLevel, 0x14);

        // vtbl
        virtual ~ZBitmap();
        virtual const char* GetName();
        virtual int GetType();
        virtual void Create(const ZBitmap& Bitmap, bool bBitmap);
        virtual void GetRGBA(uint8_t* pDst, int px, int py, int dx, int dy, int pitch, int dwLevel) const;
        virtual void GetData(ZBitmap::TDataBlock* pData, int dwMipLevel);
        virtual void SetName(const char* pszName);
        virtual int GetSizeX() const;
        virtual int GetSizeY() const;
        virtual void SetChecksum(int);
        virtual int GetChecksum() const;
        virtual void SetScaleFactor(float fScaleFactor);
        virtual float GetScaleFactor() const;
        virtual int GetMipLevelCount() const;
        virtual TMipLevel* GetMIPLevel(int lMip);
        virtual uint32_t GetBinSize();
        virtual void SaveBin(char* pBuffer);
        virtual void LoadBin(const char* pBuffer);
        virtual int GetParams();
        virtual void SetParams(int);
        virtual uint32_t GetId() const;
        virtual void SetId(uint32_t lId);
        virtual void SetData(void* pData, int dwSize, int dwSizeX, int dwSizeY, int dwLevel, bool bAllocate);
        virtual void SetMIPLevels(int lLevels);
        virtual void CreateMipLevels(uint32_t dwWidth, uint32_t dwHeight, bool bMipMap);
        virtual void Swizzle();
        virtual void SetSizeX(int32_t lX);
        virtual void SetSizeY(int32_t lY);
        virtual void SetMipLevelCount(int32_t lCount);
        virtual void CreateMipLevel(int level, int x1, int y1, const uint32_t* pRGBA);
        virtual void PackMipLevel(int level, int sx, int sy, const uint32_t* pRGBA) = 0;
        
        // methods

        // members
        int m_dwType { 0 };
        int m_dwSize[2] { 0 };
        int m_dwMipLevels { 0 };
        TMipLevel* m_pMipLevels { nullptr };
        char* m_pszName { nullptr };
        uint32_t m_dwId { 0u };
        int m_dwParams { 0 };
        int m_dwChecksum { 0 };
        bool m_bNameAllocated { false };
        RE_ADD_PADDING(3);
        float m_fScaleFactor { 1.f };
    };
    RE_VERIFY_SIZE(ZBitmap, 0x30);


#if 0
    // iOS build datadumps
    ZBitmap
    ZBitmapPal
    ZBitmapPalOpac
    ZBitmap32
    ZBitmapXBase
    ZBitmap16A0
    ZBitmap16A1
    ZBitmap16A4
    ZBitmapU8V8
    ZBitmapI8
    ZBitmapDXT1
    ZBitmapDXT3
    ZBitmapDXT5
    ZBitmapA8
    ZBitmapASTC
    ZBitmapASTC4
    ZBitmapASTC8
    ZBitmapASTC12
#endif
}