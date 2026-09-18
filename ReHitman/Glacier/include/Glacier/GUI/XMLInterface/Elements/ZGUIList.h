#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/XMLInterface/Elements/IGUIElement.h>
#include <Glacier/GUI/XMLInterface/Elements/ZGUIGroup.h>
#include <Glacier/ZSTL/ZStaticVector.h>
#include <Glacier/ZSTL/zstring.h>


namespace Glacier
{
    // fwds
    class ZBUTTON;
    class ZWINOBJ;
    class ZLINEOBJ;
    class ZButtonGraphic;
    class ZColorSet;
    class ZResourceManager;
    class ZListDataInterface;
    class ZWINGROUP;

    class ZGUIList : public IGUIElement
    {
    public:
        static constexpr int32_t MAX_NUM_OF_VISIBLE_BUTTONS = 17;
        static constexpr int32_t NUM_OF_COLUMNS = 8;

        // vtbl
        virtual void readParams(const char** ppParams, ZMenuElements* pElems) override;
        virtual void addElement(const char* pName, ZGUIBase* pEntry) override;
        virtual void Update(bool) override;
        virtual void Click(eZWUserEvents, int, ZXMLGUISystem*) override;
        virtual ZGUIElementLink Setup(float*, ZResourceManager*, ZWINGROUP*) override;
        virtual void ReleaseResources(ZResourceManager*) override;
        virtual bool SetFocus(bool) override;
        virtual void Cancel() override;
        virtual void Invalidate() override;

        // methods
        ZGUIList();
        ~ZGUIList();

        // members
        int32_t m_iIdTop;                                           // +0x68
        int32_t m_iNumOfButtonsUsed;                                // +0x6c
        int32_t m_iLineDistance;                                    // +0x70
        int32_t m_iLastFocusId;                                     // +0x74
        int32_t m_iLastButtonFocus;                                 // +0x78
        ZBUTTON* m_paButtons[MAX_NUM_OF_VISIBLE_BUTTONS];           // +0x7c
        ZStaticVector<ZWINOBJ*, 8> m_VecCheckedWinObjs[MAX_NUM_OF_VISIBLE_BUTTONS];   // +0xc0
        ZStaticVector<ZWINOBJ*, 8> m_VecUncheckedWinObjs[MAX_NUM_OF_VISIBLE_BUTTONS]; // +0x324
        ZStaticVector<ZLINEOBJ*, 8> m_VecLineObjects[MAX_NUM_OF_VISIBLE_BUTTONS][NUM_OF_COLUMNS]; // +0x588
        ZWINGROUP* m_pWinBorderGraphic;                             // +0x18a8
        char m_szBorderName[64];                                    // +0x18ac
        ZListDataInterface* m_pDataInterface;                       // +0x18ec
        zstring m_sWindow;                                          // +0x18f0
        char m_szInterface[64];                                     // +0x18fc
        ZGUIGroup m_ListTemplate;                                   // +0x193c
        bool m_bUseTemplate;                                        // +0x19f4
        ZVector2 m_v2Size;                                          // +0x19f8
        int32_t m_iNumOfVisibleButtons;                             // +0x1a00
        ZButtonGraphic* m_pBorderGraphic;                           // +0x1a04
        ZBUTTON* m_pButtonBorder;                                   // +0x1a08
        ZResourceManager* m_pResourceManager;                       // +0x1a0c
        ZColorSet* m_pIconColorSet;                                 // +0x1a10
        bool m_bListFocus;                                          // +0x1a14
        bool m_bInvalidate;                                         // +0x1a15
        ZVector2 m_v2DefaultPos;                                    // +0x1a18
        ZWINGROUP* m_pParent;                                       // +0x1a20
        bool m_bDisabled;                                           // +0x1a24
        bool m_bLinkOut;                                            // +0x1a25
        bool m_bPlaceArrowsButtonRight;                             // +0x1a26 - PC only
        bool m_bUseDoubleArrowPos;                                  // +0x1a27 - PC only
        ZVector2 m_vArrowsPos;                                      // +0x1a28 - PC only
        ZVector2 m_vArrowsPos2;                                     // +0x1a30 - PC only
        ZBUTTON* m_paArrowButtons[2];                               // +0x1a38
        ZButtonGraphic* m_paBorderGraphicArrows[2];                 // +0x1a40

    private:
        void SetupTextOnButtons();
    };
    RE_VERIFY_SIZE(ZGUIList, 0x1a48); // Verified PC alloc (PS2 is 0x18cc)
    RE_VERIFY_OFFSET(ZGUIList, m_iIdTop, 0x68);
    RE_VERIFY_OFFSET(ZGUIList, m_iNumOfButtonsUsed, 0x6c);
    RE_VERIFY_OFFSET(ZGUIList, m_iLineDistance, 0x70);
    RE_VERIFY_OFFSET(ZGUIList, m_iLastFocusId, 0x74);
    RE_VERIFY_OFFSET(ZGUIList, m_iLastButtonFocus, 0x78);
    RE_VERIFY_OFFSET(ZGUIList, m_paButtons, 0x7c);
    RE_VERIFY_OFFSET(ZGUIList, m_VecCheckedWinObjs, 0xc0);
    RE_VERIFY_OFFSET(ZGUIList, m_VecUncheckedWinObjs, 0x324);
    RE_VERIFY_OFFSET(ZGUIList, m_VecLineObjects, 0x588);
    RE_VERIFY_OFFSET(ZGUIList, m_pWinBorderGraphic, 0x18a8);
    RE_VERIFY_OFFSET(ZGUIList, m_szBorderName, 0x18ac);
    RE_VERIFY_OFFSET(ZGUIList, m_pDataInterface, 0x18ec);
    RE_VERIFY_OFFSET(ZGUIList, m_sWindow, 0x18f0);
    RE_VERIFY_OFFSET(ZGUIList, m_szInterface, 0x18fc);
    RE_VERIFY_OFFSET(ZGUIList, m_ListTemplate, 0x193c);
    RE_VERIFY_OFFSET(ZGUIList, m_bUseTemplate, 0x19f4);
    RE_VERIFY_OFFSET(ZGUIList, m_v2Size, 0x19f8);
    RE_VERIFY_OFFSET(ZGUIList, m_iNumOfVisibleButtons, 0x1a00);
    RE_VERIFY_OFFSET(ZGUIList, m_pBorderGraphic, 0x1a04);
    RE_VERIFY_OFFSET(ZGUIList, m_pButtonBorder, 0x1a08);
    RE_VERIFY_OFFSET(ZGUIList, m_pResourceManager, 0x1a0c);
    RE_VERIFY_OFFSET(ZGUIList, m_pIconColorSet, 0x1a10);
    RE_VERIFY_OFFSET(ZGUIList, m_bListFocus, 0x1a14);
    RE_VERIFY_OFFSET(ZGUIList, m_bInvalidate, 0x1a15);
    RE_VERIFY_OFFSET(ZGUIList, m_v2DefaultPos, 0x1a18);
    RE_VERIFY_OFFSET(ZGUIList, m_pParent, 0x1a20);
    RE_VERIFY_OFFSET(ZGUIList, m_bDisabled, 0x1a24);
    RE_VERIFY_OFFSET(ZGUIList, m_bLinkOut, 0x1a25);
    RE_VERIFY_OFFSET(ZGUIList, m_bPlaceArrowsButtonRight, 0x1a26);
    RE_VERIFY_OFFSET(ZGUIList, m_bUseDoubleArrowPos, 0x1a27);
    RE_VERIFY_OFFSET(ZGUIList, m_vArrowsPos, 0x1a28);
    RE_VERIFY_OFFSET(ZGUIList, m_vArrowsPos2, 0x1a30);
    RE_VERIFY_OFFSET(ZGUIList, m_paArrowButtons, 0x1a38);
    RE_VERIFY_OFFSET(ZGUIList, m_paBorderGraphicArrows, 0x1a40);
}
