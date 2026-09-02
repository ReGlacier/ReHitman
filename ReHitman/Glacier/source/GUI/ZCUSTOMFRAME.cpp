#include <Glacier/GUI/ZCUSTOMFRAME.h>
#include <Glacier/GUI/ZWINPIC.h>
#include <Glacier/Render/Prim/SPrimSpritesArray.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Sprite/SSpriteArray.h>
#include <Glacier/Render/Sprite/SSpriteArrayElementUV.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/ZTextureBase.h>
#include <Glacier/RTP/VirtualTables.h>


namespace Glacier
{
    namespace
    {
        struct STextureRef
        {
            uint32_t texture;
            uint32_t sprite;
        };

        uint16_t CreateCustomFrameFaces(ZCUSTOMFRAME* pFrame, STextureRef (&textures)[9])
        {
            for (uint32_t i = 1; i < 9; ++i)
            {
                const STextureRef item = textures[i];
                uint32_t j = i;
                while (j && textures[j - 1].texture > item.texture)
                {
                    textures[j] = textures[j - 1];
                    --j;
                }
                textures[j] = item;
            }

            SSpriteArrayElementUV sortedSprites[9];
            for (uint32_t i = 0; i < 9; ++i)
                sortedSprites[i] = pFrame->m_pSprites[textures[i].sprite];
            for (uint32_t i = 0; i < 9; ++i)
                pFrame->m_pSprites[i] = sortedSprites[i];

            uint16_t arrayCount = 0;
            for (uint32_t i = 0; i < 9; ++i)
            {
                if (!i || textures[i].texture != textures[i - 1].texture)
                    ++arrayCount;
            }

            pFrame->m_pPrims = g_pRenderDll->m_pPrimControl->AllocPrimList(arrayCount);
            pFrame->m_pSpriteArrays = g_pRenderDll->m_pPrimControl->AllocSpriteArrays(arrayCount);

            uint16_t arrayIndex = 0;
            uint32_t firstSprite = 0;
            while (firstSprite < 9)
            {
                uint32_t nextSprite = firstSprite + 1;
                while (nextSprite < 9 && textures[nextSprite].texture == textures[firstSprite].texture)
                    ++nextSprite;

                pFrame->m_pPrims[arrayIndex] = pFrame->CreateSpriteArray(
                    textures[firstSprite].texture,
                    pFrame->m_dwDrawMode | 0x2C000u,
                    SPRITETYPE_ARRAY_UV);
                pFrame->m_pSpriteArrays[arrayIndex].lNumSprites = nextSprite - firstSprite;
                pFrame->m_pSpriteArrays[arrayIndex].lDrawMask = 0;
                pFrame->m_pSpriteArrays[arrayIndex].pSpritesUV = pFrame->m_pSprites + firstSprite;

                ++arrayIndex;
                firstSprite = nextSprite;
            }

            return arrayCount;
        }
    }

    ZCUSTOMFRAME::ZCUSTOMFRAME(const char* psName, ZBaseGeom* pBaseGeom)
        : ZFRAME(psName, pBaseGeom)
    {
        m_lTextSize = 8;
    }

    ZCUSTOMFRAME::~ZCUSTOMFRAME() = default;

    const RTP::ZPropertyInfo& ZCUSTOMFRAME::GetProperties() const
    {
        return ZCUSTOMFRAME::Info;
    }

    uint32_t ZCUSTOMFRAME::GetObjectId() const
    {
        return ZCUSTOMFRAME::m_Id;
    }

    void ZCUSTOMFRAME::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZCUSTOMFRAME::m_Id;
        mask = ZCUSTOMFRAME::m_Mask;
    }

    ZGEOMCLASSINFO* ZCUSTOMFRAME::GetOldClassInfo() const
    {
        return ZCUSTOMFRAME::m_OldClassInfo;
    }

    void ZCUSTOMFRAME::CopyData(const ZGEOM* pSource)
    {
        ZWINOBJ::CopyData(pSource);

        const ZFRAME* pBaseFrame = geom_cast<ZFRAME>(pSource);
        if (pBaseFrame)
        {
            for (uint32_t i = 0; i < 9; ++i)
                m_aPieces[i] = pBaseFrame->m_aPieces[i];
            m_lWidth = pBaseFrame->m_lWidth;
            m_lHeight = pBaseFrame->m_lHeight;
        }

        const ZCUSTOMFRAME* pFrame = geom_cast<ZCUSTOMFRAME>(pSource);
        if (!pFrame)
            return;

        for (uint32_t i = 0; i < 3; ++i)
        {
            m_aWidths[i] = pFrame->m_aWidths[i];
            m_aHeights[i] = pFrame->m_aHeights[i];
        }
    }

    void ZCUSTOMFRAME::SetOuterSize(int width, int height)
    {
        SetSize(width - m_aWidths[0] - m_aWidths[2],
                height - m_aHeights[0] - m_aHeights[2]);
    }

    void ZCUSTOMFRAME::GetOuterSize(int32_t& width, int32_t& height) const
    {
        width = m_aWidths[0] + m_lWidth + m_aWidths[2];
        height = m_aHeights[0] + m_lHeight + m_aHeights[2];
    }

    void ZCUSTOMFRAME::CreateGeometry()
    {
        RemoveGeometry();
        m_iNumSprites = 9;
        m_pSprites = g_pRenderDll->m_pPrimControl->AllocSpriteArrayUV(m_iNumSprites);
        SetDrawing();

        STextureRef textures[9] {};
        for (uint32_t i = 0; i < 9; ++i)
        {
            ZWINOBJ* pPiece = geom_cast<ZWINOBJ>(ZGEOM::RefToPtr(m_aPieces[i]));
            ZASSERT(pPiece != nullptr);

            if (const ZWINPIC* pPicture = geom_cast<ZWINPIC>(pPiece))
            {
                ZASSERT(pPiece->m_pPrims != nullptr);
                const auto* pPrim = ZPrimControlBase::GetPrimitive<const SPrimSpritesArray>(pPiece->m_pPrims[0]);
                ZASSERT(pPrim != nullptr);
                ZASSERT(pPrim->lType == 2);
                textures[i].texture = pPrim->lTextureId;
                m_lTextSize = pPicture->m_iOriginalSizeX;
            }
            else
            {
                textures[i].texture = g_pRenderDll->m_pPrimControl->GetPrimTextureId(pPiece->BaseGeom()->m_lPrim, "*");
                m_lTextSize = g_pRenderDll->m_pPrimControl->GetTextureFromPrim(pPiece->BaseGeom()->m_lPrim)->m_usSize[0];
            }
            textures[i].sprite = i;

            ZASSERT(g_pRenderDll->GetTexture(textures[i].texture) != nullptr);
        }

        const int32_t outerWidth = m_aWidths[0] + m_lWidth + m_aWidths[2];
        const int32_t outerHeight = m_aHeights[0] + m_lHeight + m_aHeights[2];
        ZTextureBase* pTexture = g_pRenderDll->GetTexture(textures[0].texture);
        const float textureWidth = pTexture->m_usSize[0] ? static_cast<float>(pTexture->m_usSize[0]) : 1.0f;
        const float textureHeight = pTexture->m_usSize[1] ? static_cast<float>(pTexture->m_usSize[1]) : 1.0f;

        float y = static_cast<float>(-m_aHeights[0]);
        float sourceY = textureHeight - static_cast<float>(m_aHeights[0] + m_aHeights[1] + m_aHeights[2]);
        for (uint32_t row = 0; row < 3; ++row)
        {
            const int32_t height = row == 1 ? outerHeight - m_aHeights[0] - m_aHeights[2] : m_aHeights[row];
            const float insetY = row == 1 ? 1.0f : 0.0f;
            float x = static_cast<float>(-m_aWidths[0]);
            float sourceX = 0.0f;

            for (uint32_t column = 0; column < 3; ++column)
            {
                const int32_t width = column == 1 ? outerWidth - m_aWidths[0] - m_aWidths[2] : m_aWidths[column];
                const float insetX = column == 1 ? 1.0f : 0.0f;
                SSpriteArrayElementUV& sprite = m_pSprites[row * 3 + column];

                sprite.p = {x + static_cast<float>(width) * 0.5f,
                            y + static_cast<float>(height) * 0.5f,
                            0.0f};
                sprite.fScale = {static_cast<float>(width), static_cast<float>(height)};
                sprite.u = {(sourceX + insetX) / textureWidth,
                            (sourceX + static_cast<float>(m_aWidths[column]) - insetX) / textureWidth};
                sprite.v = {1.0f - (sourceY + insetY) / textureHeight,
                            1.0f - (sourceY + static_cast<float>(m_aHeights[row]) - insetY) / textureHeight};
                sprite.lColor = m_dwFaceColor;
                g_pRenderDll->m_pPrimControl->ColorUTOIU(&sprite.lColor, &m_dwFaceColor);

                x += static_cast<float>(width);
                sourceX += static_cast<float>(m_aWidths[column]);
            }

            y += static_cast<float>(height);
            sourceY += static_cast<float>(m_aHeights[row]);
        }

        m_iNumSpriteArrays = CreateCustomFrameFaces(this, textures);
        RecalcMaxMin();
        SetModified(true);
    }

#   pragma region " --- RTTI ---"
    namespace cProperties
    {
        static RTP::ZDataProperty<int32_t[3]> Heights{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_aHeights", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_int_3,
            .m_Offset = CLASS_PROPERTY(ZCUSTOMFRAME, m_aHeights)};
        static RTP::ZDataProperty<int32_t[3]> Widths{
            .m_Node = {.m_Next = Heights, .m_Name = "m_aWidths", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_int_3,
            .m_Offset = CLASS_PROPERTY(ZCUSTOMFRAME, m_aWidths)};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZCUSTOMFRAME,
        ZFRAME,
        0x009A2CA0,
        "ZCUSTOMFRAME",
        0x0077EEFC,
        cProperties::Widths,
        0x008091BC,
        0x009A2BF8,
        0x009A2BFC
    );
#   pragma endregion
}
