#include <Glacier/GameBase/ZPlayer.h>

namespace Glacier
{
#   pragma region " --- RTTI --- "
    namespace cProperties
    {
    }

    // PC 0x73DB90 static init:
    // ZNonResourceClassInfo::Ctor(&Info, "ZPlayer", 0x768, "ZLNKWHANDS", 0x80200014, ..., factory, &m_Id, &m_Mask);
    // ZOldTypeInfo::sub_4EE6C0(&g_ZPlayerOldClassInfo, (int)0x80200014, &Info);
    DECLARE_GEOM_CLASS_PURE_IMPL(
        ZPlayer,
        ZLNKWHANDS,
        0x0099C9B0,
        "ZPlayer",
        0x007703A4,
        nullptr, // TODO: Finish me (property node chain at PC 0x00809DA0: ZPlayer::Property_*)
        0x00809DA0,
        0x0099C954,
        0x0099C958
    );
#   pragma endregion

    bool ZPlayer::IsDead() const
    {
        return (m_CurrentStatus & 0x00040000u) != 0;
    }
}
