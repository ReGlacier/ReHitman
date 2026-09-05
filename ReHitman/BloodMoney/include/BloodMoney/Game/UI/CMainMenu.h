#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/CBaseEvent.h>

namespace Glacier
{
    class IView;
}

namespace Hitman::BloodMoney
{
    class CMainMenu : public Glacier::CBaseEvent<Glacier::ZGROUP>
    {
    public:
        // vtbl
        // data
        Glacier::ZCAMERA* m_pAnimCamera;
        Glacier::ZCAMERA* m_pOverlayCamera;
        Glacier::ZCAMERA* m_pLogoCamera;
        Glacier::IView* m_pView1;
        Glacier::IView* m_pView2;
        Glacier::IView* m_pView3;
        Glacier::REFTAB m_rtCameras;
        Glacier::ZREF m_rScopeCamera;
        Glacier::ZREF m_rScopeGroup;
        Glacier::REFTAB m_rtTargets;
        Glacier::REFTAB m_rtOverlays;
        Glacier::ZREF m_rLogoCamera;
        Glacier::ZREF m_rLogoGroup;
    };
    RE_VERIFY_SIZE(CMainMenu, 0xAC); // verified
}
