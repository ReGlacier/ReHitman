#include <Glacier/Geom/ZEnvSampler.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/RTP/VirtualTables.h>

namespace Glacier
{
    ZEnvSampler::ZEnvSampler(const char* n, ZBaseGeom* b) : ZSTDOBJ(n, b) {}
    ZEnvSampler::~ZEnvSampler() = default;
    const RTP::ZPropertyInfo& ZEnvSampler::GetProperties() const { return Info; }
    uint32_t ZEnvSampler::GetObjectId() const { return m_Id; }
    void ZEnvSampler::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const { id = m_Id; mask = m_Mask; }
    ZGEOMCLASSINFO* ZEnvSampler::GetOldClassInfo() const { return m_OldClassInfo; }
    void ZEnvSampler::CalcCenSize() { if (m_baseGeom) m_baseGeom->CalcCenSize(true); }
    void ZEnvSampler::ClassInit()
    {
        if (!m_baseGeom || !m_baseGeom->ParentGroup()) return;
        auto* p = m_baseGeom->ParentGroup(); ZMat3x3 m; ZVector3 pos, size, center;
        p->GetRootTM(m, pos); p->GetCen(center); p->GetSize(size); vmmul(center, m); center += pos;
        SetSize(size); SetRadius(std::sqrt(size.x * size.x + size.y * size.y + size.z * size.z) + 1.0f);
        GetRootTM(m, pos); center -= pos; vmmul(center, m); SetCen(center);
    }
    void ZEnvSampler::CopyData(const ZGEOM* source)
    {
        ZGEOM::CopyData(source); if (!source || (source->GetObjectId() & m_Mask) != m_Id) return;
        auto* p = static_cast<const ZEnvSampler*>(source); m_lIdentifier = p->m_lIdentifier;
        m_fCameraFar = p->m_fCameraFar; m_fFogNear = p->m_fFogNear; m_fFogFar = p->m_fFogFar;
        m_lFogColor = p->m_lFogColor; m_eUpdateFrequency = p->m_eUpdateFrequency; m_bCurrentRoomOnly = p->m_bCurrentRoomOnly;
    }
    namespace cProperties
    {
        static RTP::ZDataProperty<bool> CurrentRoomOnly{{nullptr, "m_bCurrentRoomOnly", 1}, VirtualTable_DP__1, CLASS_PROPERTY(ZEnvSampler, m_bCurrentRoomOnly)};
        static RTP::ZEnumProperty UpdateFrequency{{CurrentRoomOnly, "m_eUpdateFrequency", 1}, VirtualTable_EP, CLASS_PROPERTY(ZEnvSampler, m_eUpdateFrequency), nullptr};
        static RTP::ZDataProperty<ZCOLOR> FogColor{{UpdateFrequency, "m_lFogColor", 1}, VirtualTable_DP__2, CLASS_PROPERTY(ZEnvSampler, m_lFogColor)};
        static RTP::ZDataProperty<float> FogFar{{FogColor, "m_fFogFar", 1}, VirtualTable_DP__11, CLASS_PROPERTY(ZEnvSampler, m_fFogFar)};
        static RTP::ZDataProperty<float> FogNear{{FogFar, "m_fFogNear", 1}, VirtualTable_DP__11, CLASS_PROPERTY(ZEnvSampler, m_fFogNear)};
        static RTP::ZDataProperty<float> CameraFar{{FogNear, "m_fCameraFar", 1}, VirtualTable_DP__11, CLASS_PROPERTY(ZEnvSampler, m_fCameraFar)};
        static RTP::ZDataProperty<uint32_t> Identifier{{CameraFar, "m_lIdentifier", 1}, VirtualTable_DP__6, CLASS_PROPERTY(ZEnvSampler, m_lIdentifier)};
    }
    DECLARE_GEOM_CLASS_IMPL(ZEnvSampler, ZSTDOBJ, 0x00972608, "ZEnvSampler", 0x00769064, cProperties::Identifier, 0x0080952C, 0x00972600, 0x00972604);
}
