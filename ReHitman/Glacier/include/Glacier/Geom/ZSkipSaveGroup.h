#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
	struct ZSkipSaveGroup : public ZGROUP
	{
	public:
	    // RTTI
        DECLARE_GEOM_CLASS(ZROOM, 0x101770u);

        // vtbl
        ~ZSkipSaveGroup() override;
        // ZSerializable

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // ZGROUP

        // methods
        ZSkipSaveGroup(const char* psName, ZBaseGeom* pBaseGeom);

        // members
	};
	RE_VERIFY_SIZE(ZSkipSaveGroup, 0x4C); // Verified
}
