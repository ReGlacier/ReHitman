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

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // ZWINGROUP
        bool WndMessage(struct ZWMEVENT* event) override;

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
    RE_VERIFY_SIZE(ZWINDOW, 0x5C); // Verified PC alloc

    RE_VERIFY_OFFSET(ZWINDOW, m_rDefaultFocus, 0x54);
    RE_VERIFY_OFFSET(ZWINDOW, m_rWindows, 0x58);
}
