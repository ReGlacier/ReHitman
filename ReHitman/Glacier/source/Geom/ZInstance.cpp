#include <Glacier/Geom/ZInstance.h>


namespace Glacier
{
    ZInstance::ZInstance(const char* psName, ZBaseGeom* pBaseGeom)
        : ZGROUP(psName, pBaseGeom)
        , m_rTemplate(0)
    {
    }

    ZInstance::~ZInstance() = default;

    const RTP::ZPropertyInfo& ZInstance::GetProperties() const
    {
        return ZInstance::Info;
    }

	uint32_t ZInstance::GetObjectId() const
	{
	    return ZInstance::m_Id;
	}

    void ZInstance::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZInstance::m_Id;
        mask = ZInstance::m_Mask;
    }

    ZGEOMCLASSINFO* ZInstance::GetOldClassInfo() const
    {
        return ZInstance::m_OldClassInfo;
    }

	void ZInstance::ClassInit()
	{
	    ZGROUP::ClassInit();
	}

	void ZInstance::CopyData(const ZGEOM* Source)
	{
		ZGROUP::CopyData(Source);

	    if (const auto* pSource = geom_cast<ZInstance>(Source))
		{
		    m_rTemplate = pSource->m_rTemplate;
		}
	}

#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(
        ZInstance,
        ZGROUP,
        0x00972AD8,
        "ZInstance",
        0x0076AE4C,
        nullptr,
        0x0080F564,
        0x00972A38,
        0x00972A3C
    );
#   pragma endregion
}
