#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/ZCameraSpace.h>
#include <Glacier/GUI/SMouseColi.h>
#include <Glacier/Render/Sprite/SSpriteArrayElementUV.h>
#include <Glacier/Render/Sprite/SSpriteArray.h>
#include <Glacier/Render/Sprite/SPRITETYPE.h>


namespace Glacier
{
    class ZWINOBJ : public ZSTDOBJ
    {
    public:
        enum EType
        {
            TYPE_Normal,
            TYPE_Pushed,
            TYPE_Hover,
            TYPE_Focused,
            TYPE_Checked,
            TYPE_Disabled,
            TYPE_Invisible,
            TYPE_Background
        };

        enum EDrawMode
        {
            DRAWMODE_NORMAL,
            DRAWMODE_ADD,
            DRAWMODE_SUB,
            DRAWMODE_TRANS,
            DRAWMODE_MUL,
            DRAWMODE_OPACITY,
            DRAWMODE_HARDOPACITY
        };

        enum EAlignment
        {
            ALIGNMENT_Top_Left,
            ALIGNMENT_Top_Right,
            ALIGNMENT_Top_Center,
            ALIGNMENT_Bottom_Left,
            ALIGNMENT_Bottom_Right,
            ALIGNMENT_Bottom_Center,
            ALIGNMENT_Middle_Left,
            ALIGNMENT_Middle_Right,
            ALIGNMENT_Middle_Center,
            ALIGNMENT_Left,
            ALIGNMENT_Right,
            ALIGNMENT_Center,
            ALIGNMENT_Top,
            ALIGNMENT_Bottom,
            ALIGNMENT_Middle,
            ALIGNMENT_None
        };

        // RTTI
        DECLARE_GEOM_CLASS(ZWINOBJ, 0x20002Cu);

        // vtbl
        ~ZWINOBJ() override;
        // ZSerializable
        bool PostLoad(ISerializerStream& stream) override;
        void LoadSave(ISerializerStream& stream, bool bSaving) override;
        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;
        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CalcCenSize() override;
        void SetMat(const ZMat3x3& mMat) override;
        void SetPos(const ZVector3& vPos) override;
        void Hide(bool bHide) override;
        bool WantDrawBufferControl() const override;
        bool DrawBufferViewUpdate(ZDrawBuffer* pDrawBuffer, ZCameraSpace* pCameraSpace) override;
        void ClassInit() override;
        void CopyData(const ZGEOM* pSource) override;
        // ZSTDOBJ
        // ZWINOBJ
        virtual uint32_t CreateSpriteArray(uint32_t lTextureId, uint32_t lDrawMode, SPRITETYPE lSpriteType);
        virtual void FreeSpriteArray(uint32_t iPrimId);
        virtual void DrawRaw(ZDrawBuffer* pDrawBuffer, const ZMat3x3& mMat, const ZVector3& vPos, const ZVector3& vCen, float fRadius);
        virtual void Draw(ZDrawBuffer* pDrawBuffer, ZCameraSpace* pCameraSpace, const ZMat3x3& mMat, const ZVector3& vPos);
        virtual void Draw(ZDrawBuffer* pDrawBuffer, ZCameraSpace* pCameraSpace, const ZMat3x3& mMat, const ZVector3& vPos, uint8_t lPriority);
        virtual void RecalcMaxMin();
        virtual void GetMouseColi(SMouseColi& sColi, const ZVector3& vPos, const ZMat3x3& mMat);
        virtual void RemoveGeometry();
        virtual void LoadSaveGeometry(ISerializerStream& stream, bool bSaving);
        virtual void SetDrawMode(uint32_t mode, int pctval);
        virtual int32_t CalcDrawMode(int dwDrawMode, int dwAmount) const;
        virtual void SetDrawing();
        virtual void SetColor(uint32_t lColor);
        virtual void SetColor(uint32_t lColor, bool bUseAlpha);
        virtual void SetColor(const ZVector3& vColor);
        virtual void SetAlpha(uint8_t lAlpha);
        virtual void SetScale(const Vector2& vScale, bool bMakeUnique);
        virtual void SetAlignment(uint8_t lAlignment);
        virtual uint8_t GetAlignment() const;
        virtual void SetPos(float x, float y, float z);
        virtual void SetType(uint8_t lType);
        virtual uint32_t GetDrawMode() const;
        virtual void SetPriority(uint8_t lPrio);

        // methods
        ZWINOBJ(const char* psName, ZBaseGeom* pBaseGeom);
        void SetModified(bool bModified);

        void GetType(EType& eType);
        void SetType(const EType& eType);
        void GetDrawMode(EDrawMode& eDrawMode);
        void SetDrawMode(const EDrawMode& eDrawMode);
        void GetAlignment(EAlignment& eAlignment);
        void SetAlignment(const EAlignment& eAlignment);

        void CalcPosScale(ZCameraSpace* pCameraSpace, const ZMat3x3& mMat, const ZVector3& vPos,
            ZVector3& vOutPos, ZVector3& vOutScale, ZMat3x3& mOutMat, ZDrawBuffer* pDrawBuffer);

        // members
        uint32_t m_dwFaceColor;
        uint32_t m_dwDrawMode;
        ZVector2 m_v2Scale;
        SSpriteArrayElementUV *m_pSprites;
        uint16_t m_iNumSprites;
        uint16_t m_iNumSpriteArrays;
        uint32_t *m_pPrims;
        SSpriteArray *m_pSpriteArrays;
        ZVector2 m_vAlignmentOffset;
        ZVector3 m_vLastCamPos;
        ZMat3x3 m_mCache;
        ZVector3 m_vCache;
        ZVector3 m_vCacheScale;
        uint8_t m_dwAlignment;
        uint8_t m_dwType;
        uint8_t m_dwAmount;
        uint8_t m_iPriority;
        uint8_t m_bModified : 1;
        uint8_t m_bStaticSprites : 1;
        uint8_t m_bAnimateAlpha : 1;
        uint8_t m_bScalePS2_512_448 : 1;
        uint8_t : 4;
        RE_ADD_PADDING(3);
    }; // Size is 0x88 (verified)
    RE_VERIFY_SIZE(ZWINOBJ, 0x88);
    RE_VERIFY_OFFSET(ZWINOBJ, m_dwFaceColor, 0x10);
    RE_VERIFY_OFFSET(ZWINOBJ, m_vAlignmentOffset, 0x30);
    RE_VERIFY_OFFSET(ZWINOBJ, m_dwAlignment, 0x80);
}
