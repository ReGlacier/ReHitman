#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/GUI/ZWINDOW.h>
#include <Glacier/GUI/CWinEvent.h>

namespace Hitman::BloodMoney
{
    class CMenuWindow : public Glacier::CWinEvent<Glacier::ZWINDOW>
    {
    public:
        // vtbl (no new entries)
        // data
        Glacier::ZREF m_rFadeDown;
        float m_fOtherVis;
        float m_fVisibility;
        float m_fPushTime;
        bool m_bDeactivateFrmupdWhenFaded;
    };
    RE_VERIFY_SIZE(CMenuWindow, 0x44);
}
