#include <Glacier/GUI/ZKerningFont.h>


namespace Glacier
{
    ZKerningFont::ZKerningFont(const char* psName, ZBaseGeom* pBaseGeom)
        : ZTTFONT(psName, pBaseGeom)
    {
        // TODO: Finish me
    }

#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(
        ZKerningFont,
        ZTTFONT,
        0x009A2EC0,
        "ZKerningFont",
        0x0077FDEC,
        nullptr,  // TODO: Finish me
        0x0080ECEC,
        0x009A2E70,
        0x009A2E74
    );
#   pragma endregion
}
