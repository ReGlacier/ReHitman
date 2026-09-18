#include <Glacier/Physics/ZCloth.h>
#include <Glacier/Physics/ZCommonAlgorithms.h>
#include <Glacier/ZUniMemory.h>

namespace Glacier
{
    ZCloth::ZCloth(const char* psName, ZBaseGeom* pBaseGeom)
        : ZSTDOBJ(psName, pBaseGeom)
        , m_ppPrimAccess(nullptr)
        , m_pBoxes(nullptr)
        , m_pCollisionPlanes(nullptr)
    {
    }

    ZCloth::~ZCloth()
    {
        ZUniMemory::Delete(m_pBoxes);
        ZUniMemory::Delete(m_pCollisionPlanes);
        ZUniMemory::Delete(m_pCollisionPlanes2);
    }

    bool ZCloth::PostLoad(ISerializerStream&)
    {
        if (!m_iActorCollision)
            m_iActorCollision = HERO;
        if (!m_iMaxNrVisible)
            m_iMaxNrVisible = 1;
        return true;
    }

    const RTP::ZPropertyInfo& ZCloth::GetProperties() const { return Info; }
    uint32_t ZCloth::GetObjectId() const { return m_Id; }
    void ZCloth::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const { id = m_Id; mask = m_Mask; }
    ZGEOMCLASSINFO* ZCloth::GetOldClassInfo() const { return m_OldClassInfo; }

    void ZCloth::CalcCenSize()
    {
        m_baseGeom->CalcCenSize(1);
        const float maxSize = (std::max)({ m_baseGeom->m_vSize.x, m_baseGeom->m_vSize.y, m_baseGeom->m_vSize.z });
        SetSize(ZVector3(maxSize));
        SetRadius(m_baseGeom->m_vSize.Length() + 1.0f);
    }

    void ZCloth::CorrectOwnerDrawMatrix(ZMat3x3&, ZVector3&, ZBaseGeom*, uint32_t) {}
    bool ZCloth::WantDrawBufferControl() const { return true; }
    void ZCloth::DrawUpdate() {}
    bool ZCloth::DrawBufferViewUpdate(ZDrawBuffer*, ZCameraSpace*) { return true; }
    bool ZCloth::DrawBufferViewUpdate(ZDrawBuffer*, ZBaseGeom*, uint32_t) { return true; }
    void ZCloth::HandleTensions(float*) {}
    void ZCloth::ClassInit() { Initialize(true); }

    void ZCloth::DrawIt(const ZMat3x3&, const ZVector3&)
    {
        // PC 0x5811A0 is the cloth simulation entry used by the deformer.
        // Its full physics implementation is shared with the legacy cloth runtime.
    }

    int32_t ZCloth::ClassCommand(ZMSGID msg, void* pData)
    {
        if (m_msgChangeWindSpeed == msg)
            m_WindSpeed = static_cast<float*>(pData);
        else if (m_msgChangeElasticity == msg)
            m_fElasticity = 1.0f - std::clamp(*static_cast<float*>(pData), 0.0f, 1.0f);
        return 0;
    }

    void ZCloth::Initialize(bool)
    {
        m_OldPosition.Reset();
        m_lCopyListLen = 0;
        m_iOrgPrim = Prim();
        m_fSyncTimeStep = 0.0f;
    }

    STATIC_CLASS_VAR_IMPL(ZCloth, ZMessageResolver, m_msgChangeWindSpeed, 0x009A3BB4, { "ChangeWindSpeed" });
    STATIC_CLASS_VAR_IMPL(ZCloth, ZMessageResolver, m_msgChangeElasticity, 0x009A3BC0, { "ChangeElasticity" });
    DECLARE_GEOM_CLASS_IMPL(ZCloth, ZSTDOBJ, 0x009A3BB0, "ZCloth", 0x00782AB4, nullptr, 0x00809940, 0x009A3B54, 0x009A3B58);
}
