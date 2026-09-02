#include <Glacier/GUI/ZFRAME.h>
#include <Glacier/GUI/ZWINPIC.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Render/Prim/SPrims.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/REFTAB32.h>

#include <cstdlib>
#include <cstring>


namespace Glacier
{
    namespace
    {
        struct STextureRef
        {
            uint32_t lTextureId;
            uint32_t iSprite;
        };

        int CompareTextures(const void* pLeft, const void* pRight)
        {
            const uint32_t left = static_cast<const STextureRef*>(pLeft)->lTextureId;
            const uint32_t right = static_cast<const STextureRef*>(pRight)->lTextureId;
            return left < right ? -1 : left > right ? 1 : 0;
        }

        uint32_t CreateWinFaces(STextureRef* pTextures, SSpriteArrayElementUV* pSprites,
            uint32_t*& pPrims, SSpriteArray*& pSpriteArrays, uint32_t lNumSprites,
            uint32_t lDrawMode, ZWINOBJ* pOwner)
        {
            if (!lNumSprites)
                return 0;

            std::qsort(pTextures, lNumSprites, sizeof(STextureRef), CompareTextures);
            SSpriteArrayElementUV sortedSprites[9];
            for (uint32_t i = 0; i < lNumSprites; ++i)
                sortedSprites[i] = pSprites[pTextures[i].iSprite];
            std::memcpy(pSprites, sortedSprites, sizeof(SSpriteArrayElementUV) * lNumSprites);

            uint32_t lNumArrays = 1;
            for (uint32_t i = 1; i < lNumSprites; ++i)
                lNumArrays += pTextures[i].lTextureId != pTextures[i - 1].lTextureId;

            if (!pPrims)
            {
                pPrims = g_pRenderDll->m_pPrimControl->AllocPrimList(lNumArrays);
                pSpriteArrays = g_pRenderDll->m_pPrimControl->AllocSpriteArrays(lNumArrays);
            }

            uint32_t iArray = 0;
            uint32_t iFirstSprite = 0;
            for (uint32_t i = 1; i <= lNumSprites; ++i)
            {
                if (i != lNumSprites && pTextures[i].lTextureId == pTextures[iFirstSprite].lTextureId)
                    continue;

                pPrims[iArray] = pOwner->CreateSpriteArray(
                    pTextures[iFirstSprite].lTextureId, lDrawMode | 0xC000u, SPRITETYPE_ARRAY_UV);
                pSpriteArrays[iArray].lNumSprites = i - iFirstSprite;
                pSpriteArrays[iArray].pSpritesUV = pSprites + iFirstSprite;
                ++iArray;
                iFirstSprite = i;
            }
            return lNumArrays;
        }
    }

    ZFRAME::ZFRAME(const char* psName, ZBaseGeom* pBaseGeom)
        : ZWINOBJ(psName, pBaseGeom)
        , m_lTextSize(8)
    {
    }

    ZFRAME::~ZFRAME() = default;

    const RTP::ZPropertyInfo& ZFRAME::GetProperties() const
    {
        return ZFRAME::Info;
    }

    uint32_t ZFRAME::GetObjectId() const
    {
        return ZFRAME::m_Id;
    }

    void ZFRAME::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZFRAME::m_Id;
        mask = ZFRAME::m_Mask;
    }

    ZGEOMCLASSINFO* ZFRAME::GetOldClassInfo() const
    {
        return ZFRAME::m_OldClassInfo;
    }

    void ZFRAME::PostClassInit()
    {
        ZGEOM::PostClassInit();
        ZASSERT(m_lWidth >= 0);
        ZASSERT(m_lHeight >= 0);
        ZASSERT(m_aPieces[0] != uint32_t(-1));
        SetDrawing();
        CreateGeometry();
    }

    void ZFRAME::CopyData(const ZGEOM* pSource)
    {
        ZWINOBJ::CopyData(pSource);
        const ZFRAME* pFrame = geom_cast<ZFRAME>(pSource);
        if (!pFrame)
            return;

        std::memcpy(m_aPieces, pFrame->m_aPieces, sizeof(m_aPieces));
        m_lWidth = pFrame->m_lWidth;
        m_lHeight = pFrame->m_lHeight;
    }

    void ZFRAME::SetSize(int lWidth, int lHeight)
    {
        if (m_lWidth == lWidth && m_lHeight == lHeight)
            return;

        m_lWidth = lWidth;
        m_lHeight = lHeight;
        CreateGeometry();
    }

    void ZFRAME::SetOuterSize(int lWidth, int lHeight)
    {
        SetSize(lWidth - 2 * int(m_lTextSize), lHeight - 2 * int(m_lTextSize));
    }

    void ZFRAME::CreateGeometry()
    {
        RemoveGeometry();
        m_iNumSprites = 9;
        m_pSprites = g_pRenderDll->m_pPrimControl->AllocSpriteArrayUV(m_iNumSprites);
        SetDrawing();

        STextureRef textures[9];
        float textureScale = 0.0f;
        for (uint32_t i = 0; i < 9; ++i)
        {
            ZGEOM* pPiece = ZGEOM::RefToPtr(m_aPieces[i]);
            ZASSERT(pPiece != nullptr);

            uint32_t lTextureId;
            if (pPiece->IsDerivedFrom<ZWINPIC>())
            {
                const auto* pWinPic = static_cast<const ZWINPIC*>(pPiece);
                ZASSERT(pWinPic->m_pPrims != nullptr);
                lTextureId = ZPrimControlBase::GetPrimitive<const SPrims>(pWinPic->m_pPrims[0])->lTextureId;
            }
            else
            {
                lTextureId = g_pRenderDll->m_pPrimControl->GetPrimTextureId(pPiece->Prim(), "*");
            }

            textures[i] = { lTextureId, i };
            const auto* pWinPiece = static_cast<const ZWINOBJ*>(pPiece);
            m_lTextSize = reinterpret_cast<const ZWINPIC*>(pWinPiece)->m_iOriginalSizeX;
            textureScale = float(m_lTextSize) / (pPiece->Size().x * 4.0f);
        }

        ZWINDOWS* pWindows = nullptr;
        if (ZGROUP* pParent = Parent(); pParent && pParent->IsDerivedFrom<ZWINGROUP>())
            pWindows = static_cast<ZWINGROUP*>(pParent)->GetSystem();
        if (!pWindows)
        {
            int rWindows = 0;
            g_pEngineData->GetSceneCom()->GetVal("rWindows", &rWindows);
            pWindows = static_cast<ZWINDOWS*>(ZGEOM::RefToPtr(rWindows));
        }
        ZASSERT(pWindows != nullptr);

        static constexpr float positions[9][2] =
        {
            { -1.0f, -1.0f }, { -1.0f, 1.0f }, { 1.0f, 1.0f },
            { 1.0f, -1.0f }, { 0.0f, -1.0f }, { -1.0f, 0.0f },
            { 0.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f }
        };
        static constexpr float scales[9][2] =
        {
            { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f },
            { 0.0f, 0.0f }, { 2.0f, 0.0f }, { 0.0f, 2.0f },
            { 2.0f, 0.0f }, { 0.0f, 2.0f }, { 2.0f, 2.0f }
        };

        const int lScaledWidth = int(float(m_lWidth) * pWindows->m_fBgScale);
        const float fHalfWidth = float(lScaledWidth) * 0.5f;
        const float fHalfHeight = float(m_lHeight) * 0.5f;
        const float fPieceSize = float(m_lTextSize);
        const float fWidthHalfPixel = float(lScaledWidth & 1) * 0.5f;
        const float fHeightHalfPixel = float(m_lHeight & 1) * 0.5f;

        for (uint32_t i = 0; i < 9; ++i)
        {
            SSpriteArrayElementUV& sprite = m_pSprites[i];
            const float x = positions[i][0];
            const float y = positions[i][1];
            sprite.p.x = x < 0.0f ? x * fHalfWidth - fWidthHalfPixel - fPieceSize * 0.5f
                : x > 0.0f ? x * fHalfWidth + fPieceSize * 0.5f - fWidthHalfPixel : 0.0f;
            sprite.p.y = y < 0.0f ? y * fHalfHeight - fHeightHalfPixel - fPieceSize * 0.5f
                : y > 0.0f ? y * fHalfHeight + fPieceSize * 0.5f - fHeightHalfPixel : 0.0f;
            sprite.p.z = 0.0f;
            sprite.u = { 0.0f, textureScale };
            sprite.v = { 1.0f, 0.0f };
            sprite.fScale.x = scales[i][0] == 0.0f ? fPieceSize : fHalfWidth * scales[i][0];
            sprite.fScale.y = scales[i][1] == 0.0f ? fPieceSize : fHalfHeight * scales[i][1];
            if (scales[i][0] != 0.0f)
                sprite.p.x -= fWidthHalfPixel;
            if (scales[i][1] != 0.0f)
                sprite.p.y -= fHeightHalfPixel;
            g_pRenderDll->m_pPrimControl->ColorUTOIU(&sprite.lColor, &m_dwFaceColor);
        }

        m_iNumSpriteArrays = static_cast<uint16_t>(CreateWinFaces(textures, m_pSprites, m_pPrims,
            m_pSpriteArrays, m_iNumSprites, m_dwDrawMode, this));
        CalcCenSize();
        m_bModified = true;
    }

    void ZFRAME::GetPieces(REFTAB32&)
    {
        ZASSERT(false);
    }

    void ZFRAME::SetPieces(const REFTAB32& pieces)
    {
        const int lCount = pieces.Count();
        ZASSERT(lCount > 0);
        ZASSERT(lCount <= 9);

        int i = 0;
        for (; i < lCount; ++i)
            m_aPieces[i] = pieces.GetRefNr(i);
        for (; i < 9; ++i)
            m_aPieces[i] = m_aPieces[i - 1];
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZVirtualProperty<REFTAB32> Pieces
        {
            .m_Node = { .m_Next = nullptr, .m_Name = "Pieces", .m_Filter = 1 },
            .m_VirtualTable = VirtualTable_VP__4,
            .m_Get = &ZFRAME::GetPieces,
            .m_Set = &ZFRAME::SetPieces
        };

        static RTP::ZDataProperty<int> Height
        {
            .m_Node = { .m_Next = Pieces, .m_Name = "m_lHeight", .m_Filter = 3 },
            .m_VirtualTable = VirtualTable_DP__7,
            .m_Offset = CLASS_PROPERTY(ZFRAME, m_lHeight)
        };

        static RTP::ZDataProperty<int> Width
        {
            .m_Node = { .m_Next = Height, .m_Name = "m_lWidth", .m_Filter = 3 },
            .m_VirtualTable = VirtualTable_DP__7,
            .m_Offset = CLASS_PROPERTY(ZFRAME, m_lWidth)
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZFRAME,
        ZWINOBJ,
        0x009A2C50,
        "ZFRAME",
        0x0077EEF4,
        cProperties::Width,
        0x00809188,
        0x009A2BF0,
        0x009A2BF4
    );
#   pragma endregion
}
