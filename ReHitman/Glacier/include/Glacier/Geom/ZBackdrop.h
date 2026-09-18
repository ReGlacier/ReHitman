#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class ZBackdrop : public ZROOM
    {
    public:
        // runtime
        DECLARE_GEOM_CLASS(ZBackdrop, 0x10003Fu);

        // vtbl
        ~ZBackdrop() override;

        // ZSerializable

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit() override;

        // ZGROUP
        // ZTreeGroup
        void SetDynamicContainer(const bool& dynamic_container) override;

        // ZROOM
        // ZBackdrop

        // methods
        ZBackdrop(const char* psName, ZBaseGeom* pBaseGeom);

#       pragma region " --- RTTI Methods --- "
        void GetGeomList(REFTAB32& aGeomList);
        void SetGeomList(const REFTAB32& aGeomList);
        void GetNotInRoomTree(bool& bNotInRoomTree);
        void SetNotInRoomTree(const bool& bNotInRoomTree);
#       pragma endregion

        // members
        ZStackArray<64, ZGROUP*> m_Groups; //+0x144
    };
    RE_VERIFY_SIZE(ZBackdrop, 0x248); // Verified PC alloc
    RE_VERIFY_OFFSET(ZBackdrop, m_Groups, 0x144);
}
