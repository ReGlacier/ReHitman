#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZMessageResolver.h>
#include <Glacier/Geom/ZBoxPrimitive.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>


namespace Glacier
{
    class ZWaterBox : public ZBoxPrimitive
    {
    public:
        // constants
        static constexpr uint32_t m_TypeId = 0x2000D2u;

        // static
#       pragma region " --- Static members --- "
        STATIC_CLASS_VAR(ZWaterBox, const char*, FactoryName);
        STATIC_CLASS_VAR(ZWaterBox, RTP::ZPropertyInfo, Info);
        STATIC_CLASS_VAR(ZWaterBox, ZGEOMCLASSINFO*, m_OldClassInfo);
        DECLARE_ID_AND_MASK(ZWaterBox);
#       pragma endregion

        STATIC_CLASS_VAR(ZWaterBox, ZMessageResolver, m_msgSetCurrent);
        STATIC_CLASS_VAR(ZWaterBox, ZMessageResolver, m_msgRemoveGeom);
        STATIC_CLASS_VAR(ZWaterBox, ZMessageResolver, m_msgWaterBoxEnter);
        STATIC_CLASS_VAR(ZWaterBox, ZMessageResolver, m_msgWaterBoxLeave);

        // vtbl
        ~ZWaterBox() override;

        // ZSerializable
        bool PostLoad(ISerializerStream& stream) override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit() override;
        void ClassFrameUpdate() override;
        int32_t ClassCommand(ZMSGID Msg, void* pData) override;
        void CopyData(const ZGEOM* Source) override;

        // methods
        ZWaterBox(const char* psName, ZBaseGeom* pBaseGeom);

        void RemoveLostGeoms();
        const REFTAB32& Geoms() const { return m_Geoms; }
        auto& Current() { return m_vCurrent; }
        bool Exists(ZREF rGeom) const;
        bool Add(ZREF rGeom);
        void Remove(ZREF rGeom);

        // members
        REFTAB32 m_Geoms;
        float m_vCurrent[3];
    };

    RE_VERIFY_SIZE(ZWaterBox, 0xD4);
    RE_VERIFY_OFFSET(ZWaterBox, m_Geoms, 0x1C); // Verified PC Ctor
}
