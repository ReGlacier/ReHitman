#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/GUI/ZWinEvents.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/ZRTTI.h>
#include <Glacier/GUI/ZWINGROUP.h>


namespace Glacier
{
    class ZWINDOW : public ZWINGROUP
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZWINDOW, 0x100031u);

        // vtbl
        ~ZWINDOW() override;
        // ZWINDOW
        virtual void OnWindowOpen(uint32_t, bool);
        virtual void OnWindowClose(uint32_t, bool);
        virtual ZWINGROUP* GetDefaultFocus();
        virtual void SetDefaultFocus(ZWINGROUP* pWinGroup);

        // methods
        ZWINDOW(const char* psName, ZBaseGeom* pBaseGeom);

        // members
        ZREF m_rDefaultFocus;
        ZREF m_rWindows;
    };
}
