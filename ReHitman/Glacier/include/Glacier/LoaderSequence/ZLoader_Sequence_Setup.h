#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/ZSTL/ZRTStringObject.h>
#include <Glacier/ZSTL/REFTAB32.h>

namespace Glacier
{
    // Loading-sequence setup entity placed in a level's GMS; owns the script
    // file name and the referenced pictures.
    // PC vtable 0x0076FCEC. The RTTI dump lists five bases (ZSTDOBJ, ZGEOM,
    // RTP::cBase, ZSerializable, ZSerializableBase); those are the flattened
    // ZSTDOBJ inheritance chain, not multiple inheritance — the class keeps a
    // single vptr at +0x00.
    //
    // NOTE: XBOX_KL1 declares CopyData/ClassInit/ClassInit2 overrides as well,
    // but the PC vtable keeps the inherited ZSTDOBJ implementations — do not
    // declare them here.
    class ZLoader_Sequence_Setup : public Glacier::ZSTDOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZLoader_Sequence_Setup, 0x20011Bu);

        // vtbl
        ~ZLoader_Sequence_Setup() override; // PC 0x4FF500 (complete), 0x4FF5D0 (deleting)

        // RTP::cBase
        const Glacier::RTP::ZPropertyInfo& GetProperties() const override; // rev @ 0x4FF4B0

        // ZGEOM
        uint32_t GetObjectId() const override;                             // rev @ 0x4FF4D0
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override; // rev @ 0x4FF4E0
        Glacier::ZGEOMCLASSINFO* GetOldClassInfo() const override;         // rev @ 0x4FF4C0

        // methods
        ZLoader_Sequence_Setup(const char* psName, Glacier::ZBaseGeom* pBaseGeom); // rev @ 0x4FF450

        // data
        Glacier::ZRTString m_Loader_Script_File; // +0x10
        Glacier::REFTAB32 m_Pictures;            // +0x14
    };
    RE_VERIFY_SIZE(ZLoader_Sequence_Setup, 0xC0);
    RE_VERIFY_OFFSET(ZLoader_Sequence_Setup, m_Loader_Script_File, 0x10);
    RE_VERIFY_OFFSET(ZLoader_Sequence_Setup, m_Pictures, 0x14);
}
