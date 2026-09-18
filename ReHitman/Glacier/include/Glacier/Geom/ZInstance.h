#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZGROUP.h>


namespace Glacier
{
	class ZInstance : public ZGROUP
	{
	public:
	    // RTTI
		DECLARE_GEOM_CLASS(ZInstance, 0x1000EAu);

		// vtbl
		~ZInstance() override;

		// ZSerializable

		// RTP::cBase
		const RTP::ZPropertyInfo& GetProperties() const override;

		// ZGEOM
		uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
		void ClassInit() override;
		void CopyData(const ZGEOM* Source) override;

		// ZGROUP

		// methods
		ZInstance(const char* psName, ZBaseGeom* pBaseGeom);

		// members
		ZREF m_rTemplate;
	};
	RE_VERIFY_SIZE(ZInstance, 0x50); // Verified PC alloc
}
