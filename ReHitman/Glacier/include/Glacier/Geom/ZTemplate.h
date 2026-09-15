#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
	struct ZTemplate : public ZGROUP
	{
	public:
		// RTTI
		DECLARE_GEOM_CLASS(ZTemplate, 0x1000E9u);

		// vtbl
		~ZTemplate() override;

		// ZSerializable

		// RTP::cBase
		const RTP::ZPropertyInfo& GetProperties() const override;

		// ZGEOM
		uint32_t GetObjectId() const override;
		void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
		ZGEOMCLASSINFO* GetOldClassInfo() const override;

		// ZGROUP

		// methods
		ZTemplate(const char* psName, ZBaseGeom* pBaseGeom);

		// members
		// none
	};
	RE_VERIFY_SIZE(ZTemplate, 0x4C); // Verified
}
