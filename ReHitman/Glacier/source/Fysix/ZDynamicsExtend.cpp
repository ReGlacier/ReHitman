#include <Glacier/Fysix/ZDynamicsExtend.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZDynamicsExtend::ZDynamicsExtend()
        : m_kConSys()
    {
        m_pMapper = nullptr;
        m_pProps = nullptr;
        m_wMappings = 0;
        m_fSleepEnergy = 0.f;
    }

    ZDynamicsExtend::~ZDynamicsExtend()
    {
        ZUniMemory::Delete(m_pProps);
        m_kConSys.Clear();
    }

    bool ZDynamicsExtend::Create(const Fysix::PP_* ZDynamicsExtend, const SGenericMapper* mpr, uint16_t mappings)
    {
        // TODO: Finish me
        return false;
    }

    bool ZDynamicsExtend::Init(uint16_t body, const ZGEOM* geom)
    {
        Fysix::ZConstraintBody* pBody = m_kConSys.GetBody(body);
        ZASSERT(pBody);

        ZMat3x3 mMat;
        ZVector3 vPos;

        geom->GetRootTM(mMat, vPos);

        ZVector3 vRefPos = geom->Cen();
        vmmul(vRefPos.Get(), mMat.Get());
        vadd(vRefPos.Get(), vPos.Get());

        pBody->SetReferencePosition(reinterpret_cast<const float(*)[3]>(vRefPos.Get()));
        pBody->Stabilize(true, 10);

        return true;
    }

    bool ZDynamicsExtend::Move(uint16_t body, ZGEOM* geom, const float& dt)
    {
        // TODO: Finish me
        return false;
    }

    void ZDynamicsExtend::WakeUp(uint16_t body)
    {
        // TODO: Finish me
    }

    void ZDynamicsExtend::Stabilize(uint16_t body, uint16_t iterations)
    {
        auto* pBody = m_kConSys.GetBody(body);
        ZASSERT(pBody);

        pBody->Stabilize(false, iterations);
    }

    STATIC_CLASS_VAR_IMPL(ZDynamicsExtend, float, m_fMaxSteps, 0x009A37B0, 0.084999986f);
    STATIC_CLASS_VAR_IMPL(ZDynamicsExtend, ZMessageResolver, m_msgCollision, 0x009A3798, ZMessageResolver { "Collision" });
    STATIC_CLASS_VAR_IMPL(ZDynamicsExtend, ZMessageResolver, m_msgFracture, 0x009A37A4, ZMessageResolver { "Fracture" });
}