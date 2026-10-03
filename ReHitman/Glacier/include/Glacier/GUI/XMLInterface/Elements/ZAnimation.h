#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>


namespace Glacier
{
    // fwds
    class ZWINOBJ;

    class ZAnimation : public IGUIElement
    {
    public:
        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual ZGUIElementLink Setup(float*, ZResourceManager*, ZWINGROUP*) override;
        virtual void ReleaseResources(ZResourceManager*) override;
        virtual void Update(bool) override;

        // methods
        ZAnimation();
        ~ZAnimation();

        // members
        ZWINGROUP* m_pGraphicGroup; // +0x68
        ZWINOBJ** m_papWinObjs;     // +0x6c
        int32_t m_iNumOfFrames;     // +0x70
        int32_t m_iCurrentFrame;    // +0x74
        float m_fDeltaTime;         // +0x78
        int32_t m_iFramesPerSecond; // +0x7c
    };
    RE_VERIFY_SIZE(ZAnimation, 0x80);
    RE_VERIFY_OFFSET(ZAnimation, m_pGraphicGroup, 0x68);
    RE_VERIFY_OFFSET(ZAnimation, m_papWinObjs, 0x6c);
    RE_VERIFY_OFFSET(ZAnimation, m_iNumOfFrames, 0x70);
    RE_VERIFY_OFFSET(ZAnimation, m_iCurrentFrame, 0x74);
    RE_VERIFY_OFFSET(ZAnimation, m_fDeltaTime, 0x78);
    RE_VERIFY_OFFSET(ZAnimation, m_iFramesPerSecond, 0x7c);
}
