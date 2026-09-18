#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/System/ZGUIBase.h>
#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>


namespace Glacier
{
    // fwds
    class ZButtonGraphicPart;

    enum ECheckStatus
    {
        eCHECK_OFF = 0,
        eCHECK_ON = 1,
        eCHECK_HALF = 2,
    };

    class ZButtonGraphic : public ZGUIBase
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual void addElement(const char* pName, ZGUIBase* pEntry) override;
        virtual void endElement(const char* pTag);

        // methods
        ZButtonGraphic();

        ZButtonGraphicPart* GetGraphicPart(int iIndex);
        void GetTextOffSet(Glacier::Vector2* pOffset);
        void GetButtonSize(Glacier::Vector2* pSize);
        int32_t GetNumOfGraphicElements();
        EAlignment GetTextAlignment();
        bool GraphcisOnly() const;

        // members
        ZButtonGraphicPart* m_apGraphicPart[8]; // +0x4C
        int32_t m_iNumOfGraphicElements; // +0x6C
        Glacier::Vector2 m_v2TextOffset; // +0x70
        Glacier::Vector2 m_v2Size; // +0x78
        EAlignment m_eTextAlignment; // +0x80
        ECheckStatus m_eCheckStatus; // +0x84
        bool m_bGraphicsOnly; // +0x88
    };
    RE_VERIFY_SIZE(ZButtonGraphic, 0x8C); // Verified by PC alloc (140 bytes)
    RE_VERIFY_OFFSET(ZButtonGraphic, m_apGraphicPart, 0x4C);
    RE_VERIFY_OFFSET(ZButtonGraphic, m_iNumOfGraphicElements, 0x6C);
    RE_VERIFY_OFFSET(ZButtonGraphic, m_v2TextOffset, 0x70);
    RE_VERIFY_OFFSET(ZButtonGraphic, m_v2Size, 0x78);
    RE_VERIFY_OFFSET(ZButtonGraphic, m_eTextAlignment, 0x80);
    RE_VERIFY_OFFSET(ZButtonGraphic, m_eCheckStatus, 0x84);
    RE_VERIFY_OFFSET(ZButtonGraphic, m_bGraphicsOnly, 0x88);
}
