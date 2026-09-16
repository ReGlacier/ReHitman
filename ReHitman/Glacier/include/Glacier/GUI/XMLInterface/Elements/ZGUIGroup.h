#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/ZSTL/ZStaticVector.h>


namespace Glacier
{
    class ZGUIGroup : public IGUIElement
    {
    public:
        // vtbl
        virtual ZGUIElementLink Setup(float*, ZResourceManager*, ZWINGROUP*) override;
        virtual void ReleaseResources(ZResourceManager*) override;
        virtual void addElement(const char* pName, ZGUIBase* pEntry) override;
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual void GetSize(ZVector2& rSize);

        // methods
        ZGUIGroup();
        ~ZGUIGroup();

        void Clear();
        const ZStaticVector<IGUIElement*, 16>& GetGroup();
        ENavigation GetNavigation();

        // members
        ZStaticVector<IGUIElement*, 16> m_Group; // +0x68
        ZVector2 m_v2Size;                       // +0xac
        ENavigation m_eNavigation;               // +0xb4
    };
    RE_VERIFY_SIZE(ZGUIGroup, 0xb8);
    RE_VERIFY_OFFSET(ZGUIGroup, m_Group, 0x68);
    RE_VERIFY_OFFSET(ZGUIGroup, m_v2Size, 0xac);
    RE_VERIFY_OFFSET(ZGUIGroup, m_eNavigation, 0xb4);
}
