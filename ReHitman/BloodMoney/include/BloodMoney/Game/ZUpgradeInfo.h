#pragma once

#include <BloodMoney/Game/UI/IGUIElement.h>

namespace Hitman::BloodMoney
{
    // Type forwardings for class ZUpgradeInfo
    class ZGUIBase;
    class ZXMLGUISystem;
    class ZWINGROUP;
    class ZResourceManager;

    // Class definition ZUpgradeInfo
    class ZUpgradeInfo : public IGUIElement {
    public:
        //vftable
        //api
        void SetupTextGraphic();

        //data (total size is 0x110, base size is 0x6c)
        int m_field6C;
    };
}