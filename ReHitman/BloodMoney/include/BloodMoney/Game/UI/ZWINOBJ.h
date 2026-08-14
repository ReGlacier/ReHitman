#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/ZCameraSpace.h>
#include <Glacier/SpriteDraw.h>

namespace Hitman::BloodMoney
{
    enum SPRITETYPE : int
    {
        SPRITETYPE_ARRAY = 0x1,
        SPRITETYPE_ARRAY_BOX = 0x3,
        SPRITETYPE_ARRAY_BOX_SCALE = 0x4,
        SPRITETYPE_ARRAY_UV = 0x5,
        SPRITETYPE_ARRAY_RAW = 0x6,
        SPRITETYPE_ARRAY_PARTICLE = 0x7,
    };

    class ZWINOBJ : public Glacier::ZSTDOBJ 
    {
    public:
        // vftable
        virtual void CreateSpriteArray(uint32_t, uint32_t, SPRITETYPE);
        virtual void FreeSpriteArray(uint32_t);
        virtual void DrawRaw(Glacier::ZDrawBuffer*, float const*, float const*, float const*, float);
        virtual void Draw(Glacier::ZDrawBuffer *, Glacier::ZCameraSpace*, float const*, float const*);
        virtual void Draw(Glacier::ZDrawBuffer*, Glacier::ZCameraSpace*, Glacier::ZMat3x3* transform, Glacier::Vector3* position, unsigned char);
        virtual void RecalcMaxMin();
        virtual void GetMouseColi(Glacier::Vector4*, float*, float*);
        virtual void RemoveGeometry();
        virtual void LoadSaveGeometry(Glacier::ZPackedInput*, bool);
        virtual void SetDrawMode(uint32_t, int);
        virtual int32_t CalcDrawMode(int a2, int a3);
        virtual void SetDrawing();
        virtual void SetColor(uint32_t, bool);
        virtual void SetColor(Glacier::Vector3* pColor); // Vec3F or other
        virtual void SetColor(uint32_t);
        virtual void SetAlpha(uint8_t);
        virtual void SetScale(Glacier::Vector2* scale, bool /* unused */);
        virtual void SetAlignment(uint8_t alignType);
        virtual uint8_t GetAlignment();
        virtual void SetPos(float, float, float);
        virtual void SetType(uint8_t);
        virtual uint32_t GetDrawMode();
        virtual void SetPriority(uint8_t);

        // api
        int GetTexture(unsigned int iTextureType);

        // data (total size is 0x88, base size is 0x10)
        unsigned int m_dwFaceColor;
        unsigned int m_dwDrawMode;
        Glacier::ZVector2 m_v2Scale;
        Glacier::SSpriteArrayElementUV *m_pSprites;
        uint16_t m_iNumSprites;
        uint16_t m_iNumSpriteArrays;
        unsigned int *m_pPrims;
        Glacier::SSpriteArray *m_pSpriteArrays;
        Glacier::ZVector2 m_vAlignmentOffset;
        Glacier::ZVector3 m_vLastCamPos;
        Glacier::ZMat3x3 m_mCache;
        Glacier::ZVector3 m_vCache;
        Glacier::ZVector3 m_vCacheScale;
        uint8_t m_dwAlignment;
        uint8_t m_dwType;
        uint8_t m_dwAmount;
        uint8_t m_iPriority;
        bool m_bModified;
        bool m_bStaticSprites;
        bool m_bAnimateAlpha;
        bool m_bScalePS2_512_448;
    }; // Size is 0x88 (verified)
    RE_VERIFY_SIZE(ZWINOBJ, 0x88);
}