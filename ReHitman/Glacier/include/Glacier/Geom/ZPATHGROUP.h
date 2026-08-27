#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
	class ZPATHGROUP : public ZGROUP
	{
	public:
	    // RTTI
		DECLARE_GEOM_CLASS(ZPATHGROUP, 0x1000DEu);

		// vtbl
		~ZPATHGROUP() override;

		// ZSerializable

		// RTP::cBase
		const RTP::ZPropertyInfo& GetProperties() const override;

		// ZGEOM
		uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit() override;

		// ZPATHGROUP
		virtual void CreatePath();
		virtual void RemovePath();

		// method
		ZPATHGROUP(const char* psName, ZBaseGeom* pBaseGeom);

		// members
		REFTAB* m_prtLines;
	};
	RE_VERIFY_SIZE(ZPATHGROUP, 0x50); // Verified
}
