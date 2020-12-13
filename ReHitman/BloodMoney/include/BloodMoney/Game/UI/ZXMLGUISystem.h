#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZWinEvents.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/ZRTTI.h>

#include <BloodMoney/Game/UI/CWinEvent.h>
#include <BloodMoney/Game/UI/UIFWD.h>

namespace Hitman::BloodMoney
{
    class ZXMLGUISystem : public CWinEvent<ZWINDOW>
    {
    public:
        int m_field30; //0x0030
        Glacier::ZGROUP* m_windowGroups[18]; //0x0034

        /// vftable
        virtual void SetFocus();
        virtual void AddOtherWindowCount(int);
    };
}