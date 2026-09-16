#include <Glacier/Gui/ZWINDOW.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/RTP/VirtualTables.h>


namespace Glacier
{
    ZWINDOW::ZWINDOW(const char* psName, ZBaseGeom* pBaseGeom)
        : ZWINGROUP(psName, pBaseGeom)
        , m_rDefaultFocus(0)
    {
    }

    ZWINDOW::~ZWINDOW() = default;

    const RTP::ZPropertyInfo& ZWINDOW::GetProperties() const
    {
        return ZWINDOW::Info;
    }

    uint32_t ZWINDOW::GetObjectId() const
    {
        return ZWINDOW::m_Id;
    }

    void ZWINDOW::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZWINDOW::m_Id;
        mask = ZWINDOW::m_Mask;
    }

    ZGEOMCLASSINFO* ZWINDOW::GetOldClassInfo() const
    {
        return ZWINDOW::m_OldClassInfo;
    }

    bool ZWINDOW::WndMessage(struct ZWMEVENT* event)
    {
        switch (event->Message)
        {
            case ZWM_WINDOWOPEN:
                OnWindowOpen(event->Param1, event->Param2 != 0);
                return true;
            case ZWM_WINDOWCLOSE:
                OnWindowClose(event->Param1, event->Param2 != 0);
                return true;
            default:
                return false;
        }
    }

    void ZWINDOW::OnWindowOpen(uint32_t, bool)
    {
        Hide(false);
    }

    void ZWINDOW::OnWindowClose(uint32_t, bool)
    {
        ZWINDOWS* pSystem = GetSystem();
        if (pSystem)
        {
            ZWINGROUP* pFocused = pSystem->GetFocusedControl();
            ZGEOM* pParent = pFocused;

            while (pParent && pParent != this)
            {
                pParent = pParent->Parent();
            }

            if (pParent == this)
            {
                pSystem = GetSystem();
                pSystem->ReleaseFocusedControl(pFocused, false);
            }
        }

        Hide(true);
    }

    ZWINGROUP* ZWINDOW::GetDefaultFocus()
    {
        return static_cast<ZWINGROUP*>(ZGEOM::RefToPtr(m_rDefaultFocus));
    }

    void ZWINDOW::SetDefaultFocus(ZWINGROUP* pWinGroup)
    {
        m_rDefaultFocus = pWinGroup->GetRef();
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZDataProperty<ZGEOMREF> Windows
        {
            .m_Node = { .m_Next = nullptr, .m_Name = "m_rWindows", .m_Filter = 1 },
            .m_VirtualTable = &RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZWINDOW, m_rWindows))
        };

        static RTP::ZDataProperty<ZGEOMREF> DefaultFocus
        {
            .m_Node = { .m_Next = Windows, .m_Name = "m_rDefaultFocus", .m_Filter = 1 },
            .m_VirtualTable = &RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZWINDOW, m_rDefaultFocus))
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZWINDOW,
        ZWINGROUP,
        0x009A2F18,
        "ZWINDOW",
        0x00780084,
        cProperties::DefaultFocus,
        0x00815858,
        0x009A2EC8,
        0x009A2ECC
    );
#   pragma endregion
}
