#include <Glacier/LoaderSequence/ZLoader_Sequence_Setup.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/RTP/PropertyTypes.h>
#include <Glacier/RTP/VirtualTables.h>

namespace Glacier
{
    // PC 0x4FF450. ZRTString defaults to null and REFTAB32::ctor zeroes the
    // table, matching the original body.
    ZLoader_Sequence_Setup::ZLoader_Sequence_Setup(const char* psName, Glacier::ZBaseGeom* pBaseGeom)
        : Glacier::ZSTDOBJ(psName, pBaseGeom)
    {
    }

    // PC 0x4FF500 (complete object destructor; deleting dtor at 0x4FF5D0).
    // Member destruction order (m_Pictures, then m_Loader_Script_File) matches
    // the original REFTAB32::dtor + interned-string release sequence.
    ZLoader_Sequence_Setup::~ZLoader_Sequence_Setup() = default;

    // PC 0x4FF4B0.
    const Glacier::RTP::ZPropertyInfo& ZLoader_Sequence_Setup::GetProperties() const
    {
        return Info;
    }

    // PC 0x4FF4D0.
    uint32_t ZLoader_Sequence_Setup::GetObjectId() const
    {
        return m_Id;
    }

    // PC 0x4FF4E0.
    void ZLoader_Sequence_Setup::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = m_Id;
        mask = m_Mask;
    }

    // PC 0x4FF4C0.
    Glacier::ZGEOMCLASSINFO* ZLoader_Sequence_Setup::GetOldClassInfo() const
    {
        return m_OldClassInfo;
    }

#   pragma region " --- RTTI --- "
    // PC chain (verified from the image): Info@0x008115BC First = 0x008115A8
    //   -> 0x00811594 -> null; both nodes are ZDataProperty (0x14 bytes) with
    //   filter 1, table 0x008067A4 (ZFILENAME, encoded offset 0x0C) and table
    //   0x008066C4 (REFTAB32, encoded offset 0x10).
    // The PC name-string fields are zero in the image (RTP fills them at
    // static-init time and the originals only survive in debug PS2/XBOX
    // builds), so the names follow the project convention of naming each
    // property after its member (see ZBaseCamera / ZCONTROL).
    namespace cProperties
    {
        static RTP::ZDataProperty<REFTAB32> Pictures {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "m_Pictures",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__4,
            .m_Offset = CLASS_PROPERTY(ZLoader_Sequence_Setup, m_Pictures)
        };
        static RTP::ZDataProperty<ZFILENAME> Loader_Script_File {
            .m_Node = {
                .m_Next = Pictures,
                .m_Name = "m_Loader_Script_File",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__30,
            .m_Offset = reinterpret_cast<ZFILENAME*>(
                CLASS_PROPERTY(ZLoader_Sequence_Setup, m_Loader_Script_File))
        };
    }
    DECLARE_GEOM_CLASS_IMPL(
        ZLoader_Sequence_Setup,
        Glacier::ZSTDOBJ,
        0x0097B960,
        "ZLoader_Sequence_Setup",
        0x0076FEC4,
        cProperties::Loader_Script_File,
        0x008115BC,
        0x0097B910,
        0x0097B914);
#   pragma endregion
}
