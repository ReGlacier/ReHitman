#include <Glacier/Gui/ZWINDOW.h>


namespace Glacier
{
    ZWINDOW::ZWINDOW(const char* psName, ZBaseGeom* pBaseGeom)
        : ZWINGROUP(psName, pBaseGeom)
    {
        // TODO: Finish me
    }

    ZWINDOW::~ZWINDOW()
    {
        // TODO: Finish me
    }

    void ZWINDOW::OnWindowOpen(uint32_t, bool)
    {
        // TODO: Finish me
    }

    void ZWINDOW::OnWindowClose(uint32_t, bool)
    {
        // TODO: Finish me
    }

    ZWINGROUP* ZWINDOW::GetDefaultFocus()
    {
        // TODO: Finish me
        return nullptr;
    }

    void ZWINDOW::SetDefaultFocus(ZWINGROUP* pWinGroup)
    {
        // TODO: Finish me
    }

#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(
        ZWINDOW,
        ZWINGROUP,
        0x009A2F18,
        "ZWINDOW",
        0x00780084,
        nullptr, // TODO: Finish me
        0x00815858,
        0x009A2EC8,
        0x009A2ECC
    );
#   pragma endregion
}
