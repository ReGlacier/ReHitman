#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/ZOpenWindow.h>


namespace Glacier
{
    class ZOpenWindowTimer : public ZOpenWindow
    {
    public:
        // vtbl
        virtual ZGUIElementLink Setup(float*, ZResourceManager*, ZWINGROUP*) override;
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual void Update(bool) override;
        virtual bool SetFocus(bool) override;

        // methods
        ZOpenWindowTimer();
        ~ZOpenWindowTimer();

        // members
        float m_fElapsedTime; // +0xb8
        float m_fTime;        // +0xbc
    };
    RE_VERIFY_SIZE(ZOpenWindowTimer, 0xc0); // Verified PC alloc (0xbc on PS2)
    RE_VERIFY_OFFSET(ZOpenWindowTimer, m_fElapsedTime, 0xb8);
    RE_VERIFY_OFFSET(ZOpenWindowTimer, m_fTime, 0xbc);
}
