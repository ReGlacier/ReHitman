#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/ZSTL/zstring.h>


namespace Glacier
{
    // fwds
    class IOptionsInterface;

    class ZOption : public IGUIElement
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual ZGUIElementLink Setup(float*, ZResourceManager*, ZWINGROUP*) override;
        virtual void Cancel() override;
        virtual int32_t GetOptionValue();
        virtual void SetOptionValue(int32_t iValue);

        // methods
        ZOption();

        int32_t GetUpperBound();
        void SetUpperBound(int32_t iUpperBound);
        bool IsOptionEnabled() const;

        // members
        int32_t m_iLowerBound;                 // +0x68
        int32_t m_iUpperBound;                 // +0x6c
        int32_t m_iStep;                       // +0x70
        int32_t m_iBackup;                     // +0x74
        bool m_bTakeFocus;                     // +0x78
        int32_t m_iOption;                     // +0x7c
        IOptionsInterface* m_pOptionInterface; // +0x80
        zstring m_sOption;                     // +0x84
    };
    RE_VERIFY_SIZE(ZOption, 0x90);
    RE_VERIFY_OFFSET(ZOption, m_iLowerBound, 0x68);
    RE_VERIFY_OFFSET(ZOption, m_iUpperBound, 0x6c);
    RE_VERIFY_OFFSET(ZOption, m_iStep, 0x70);
    RE_VERIFY_OFFSET(ZOption, m_iBackup, 0x74);
    RE_VERIFY_OFFSET(ZOption, m_bTakeFocus, 0x78);
    RE_VERIFY_OFFSET(ZOption, m_iOption, 0x7c);
    RE_VERIFY_OFFSET(ZOption, m_pOptionInterface, 0x80);
    RE_VERIFY_OFFSET(ZOption, m_sOption, 0x84);
}
