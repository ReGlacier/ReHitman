#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/ZCHAROBJ.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class ZExtCharObj : public ZCHAROBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZExtCharObj, 0x200039u);

        // vtbl
        ~ZExtCharObj() override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // methods
        ZExtCharObj(const char* psName, ZBaseGeom* pBaseGeom);

        // data
    };
    RE_VERIFY_SIZE(ZExtCharObj, 0xA0);
}
