#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
	struct ZEditorGroup : public ZGROUP
	{
	public:
	    // RTTI
        DECLARE_GEOM_CLASS(ZEditorGroup, 0x10002Bu);

        // vtbl
        ~ZEditorGroup() override;

        // ZSerializable

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // ZGROUP

        // methods
        ZEditorGroup(const char* psName, ZBaseGeom* pBaseGeom);

        // members
        // none
	};
	RE_VERIFY_SIZE(ZEditorGroup, 0x4C); // Verified
}
