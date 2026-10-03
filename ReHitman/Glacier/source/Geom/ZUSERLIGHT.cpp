#include <Glacier/Geom/ZUSERLIGHT.h>
#include <cmath>


namespace Glacier
{
    ZUSERLIGHT::ZUSERLIGHT(const char* psName, ZBaseGeom* pBaseGeom)
        : ZLIGHT(psName, pBaseGeom)
    {
        // Do nothing
    }

    ZUSERLIGHT::~ZUSERLIGHT() = default;

    const RTP::ZPropertyInfo& ZUSERLIGHT::GetProperties() const
    {
        return ZUSERLIGHT::Info;
    }

    uint32_t ZUSERLIGHT::GetObjectId() const
    {
        return ZUSERLIGHT::m_Id;
    }

    void ZUSERLIGHT::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZUSERLIGHT::m_Id;
        mask = ZUSERLIGHT::m_Mask;
    }

    ZGEOMCLASSINFO* ZUSERLIGHT::GetOldClassInfo() const
    {
        return ZUSERLIGHT::m_OldClassInfo;
    }

    void ZUSERLIGHT::CalcCenSize()
    {
        if (BaseGeom()->m_lPrim)
        {
            const float fFarRange = m_fFarRange;

            ZVector3 vFarEnd { fFarRange, fFarRange, fFarRange };
            ZVector3 vNearCorner { -fFarRange, -fFarRange, -fFarRange };

            ZVector3 vCen = (vFarEnd + vNearCorner) * 0.5f;
            SetCen(vCen);

            ZVector3 vSize = vFarEnd - vCen;
            const float fBoundRadius = std::sqrt(vSize.x * vSize.x + vSize.y * vSize.y + vSize.z * vSize.z) + 1.0f;
            SetRadius(fBoundRadius);
            SetSize(vSize);
        }
    }

#   pragma region " --- RTTI --- "
    DECLARE_GEOM_CLASS_IMPL(
        ZUSERLIGHT,
        ZLIGHT,
        0x00972CA8,
        "ZUSERLIGHT",
        0x0076BE60,
        nullptr,
        0x008143C4,
        0x00972B00,
        0x00972B04
    );
#   pragma endregion

}
