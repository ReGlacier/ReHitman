#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/System/ZGUIBase.h>
#include <Glacier/GUI/XMLInterface/Elements/ZButtonGraphic.h>


namespace Glacier
{
    class ZButtonGraphicPart : public ZGUIBase
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual void addElement(const char* pName, ZGUIBase* pEntry) override;

        // methods
        ZButtonGraphicPart();

        // members
        Glacier::Vector2 m_v2Size; // +0x4C
        uint32_t m_iType; // +0x54
        int32_t m_iPriority; // +0x58
        bool m_bAnimateAlpha; // +0x5C
        bool m_bFrame; // +0x5D
        ECheckStatus m_eCheckStatus; // +0x60

    private:
        void ReadStatus(ECheckStatus& rStatus, const char** ppParams, const char* pAttrName);
    };
    RE_VERIFY_SIZE(ZButtonGraphicPart, 0x64);
    RE_VERIFY_OFFSET(ZButtonGraphicPart, m_v2Size, 0x4C);
    RE_VERIFY_OFFSET(ZButtonGraphicPart, m_iType, 0x54);
    RE_VERIFY_OFFSET(ZButtonGraphicPart, m_iPriority, 0x58);
    RE_VERIFY_OFFSET(ZButtonGraphicPart, m_bAnimateAlpha, 0x5C);
    RE_VERIFY_OFFSET(ZButtonGraphicPart, m_bFrame, 0x5D);
    RE_VERIFY_OFFSET(ZButtonGraphicPart, m_eCheckStatus, 0x60);
}
