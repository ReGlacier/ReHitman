#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/ZWINGROUP.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class ZBUTTON;
    class ZCHAROBJ;
    class ZCONTROL;
    class ZFRAME;
    class ZLINEOBJ;
    class ZPANEL;

    class ZWinGfxHandler : public ZWINGROUP
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZWinGfxHandler, 0x100047u);

        // vtbl
        ~ZWinGfxHandler() override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit() override;
        void CopyData(const ZGEOM* pSource) override;

        // ZWinGfxHandler
        virtual ZCHAROBJ* CreateText(ZWINGROUP* pGroup, const char* psText, uint32_t lResourceId);
        virtual ZLINEOBJ* CreateMultiLineText(ZWINGROUP* pGroup, const char* psText, uint32_t lWidth);
        virtual ZBUTTON* CreateTextButton(ZWINGROUP* pGroup, const char* psText, uint32_t lControlId, uint32_t lAlignment);
        virtual ZFRAME* CreateFrame(ZWINGROUP* pGroup, uint32_t lWidth, uint32_t lHeight, uint32_t lFrame);
        virtual ZPANEL* CreateSimpleDialog(ZWINGROUP* pGroup, ZCONTROL* pControl, const char* psText,
            uint32_t lControlId, uint32_t lUnused, uint32_t lWidth);

        // methods
        ZWinGfxHandler(const char* psName, ZBaseGeom* pBaseGeom);

        // members
        ZREF m_rHoriScrollBar;
        ZREF m_rVertScrollBar;
        ZREF m_rCheckButton;
        ZREF m_rFrame0;
        ZREF m_rFrame1;
        ZAUDIOREF m_rClickSound;
        ZAUDIOREF m_rHoverSound;
        ZREF m_rDefaultFont;
        uint32_t m_dwNormalColor;
        uint32_t m_dwHoverColor;
        uint32_t m_dwFocusColor;
        uint32_t m_dwPushColor;
        uint32_t m_dwDisabledColor;
        uint32_t m_dwCheckColor;
        char* m_szYes;
        char* m_szNo;
    };
    RE_VERIFY_SIZE(ZWinGfxHandler, 0x94); // Verified PC alloc
}
