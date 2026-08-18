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

    bool ZDynamicsExtend::Create(const Fysix::PP_* pp, const SGenericMapper* mpr, uint16_t mappings)
    {
        if (m_pMapper)
            return true;

        if (!CreateMapper(mpr, mappings))
            return false;

        if (m_kConSys.Init(*pp))
        {
            const uint16_t bodsCount = m_kConSys.m_wBodsCount;
            if (bodsCount)
            {
                m_pProps = ZUniMemory::NewArray<SBodyProperty>(bodsCount);

                for (uint16_t i = 0; i < bodsCount; ++i)
                {
                    m_pProps[i].active = false;
                    m_pProps[i].visible = false;
                    m_pProps[i].syncFrame = 0.0f;
                    m_pProps[i].sleepFrame = 0;
                }
            }
            else
            {
                m_pProps = nullptr;
            }
        }
        else
        {
            DestroyMapper();
        }

        return m_pMapper != nullptr;
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

        pBody->SetReferencePosition(*reinterpret_cast<const float(*)[3]>(vRefPos.Get()));
        pBody->Stabilize(true, 10);

        return true;
    }

    bool ZDynamicsExtend::Move(uint16_t body, ZGEOM* geom, const float& dt)
    {
        SBodyProperty* pProp = &m_pProps[body];

        if (!pProp->active)
            return false;

        ZMat3x3 mMat;
        ZVector3 vPos;
        geom->GetRootTM(mMat, vPos);

        ZVector3 vRefPos = geom->Cen();
        vmmul(vRefPos.Get(), mMat.Get());
        vadd(vRefPos.Get(), vPos.Get());

        pProp->syncFrame += dt;
        if (pProp->syncFrame > m_fMaxSteps)
            pProp->syncFrame = m_fMaxSteps;

        Fysix::ZConstraintBody* pBody = m_kConSys.GetBody(body);

        constexpr float fTimeStep = 0.016666668f;

        while (pProp->syncFrame >= fTimeStep)
        {
            pProp->syncFrame -= fTimeStep;

            const REFTAB* faces = GetCollisionFaces(body, geom);
            Fysix::SSimResult result;
            pBody->Simulate(result, *reinterpret_cast<const float(*)[3]>(vRefPos.Get()), faces);

            if (result.collision)
                Collision(body, geom);

            if (result.fracture)
                Fracture(body, geom);

            if (m_fSleepEnergy == 0.0f || result.energy >= m_fSleepEnergy)
            {
                pProp->sleepFrame = 0;
            }
            else
            {
                ++pProp->sleepFrame;
                if (pProp->sleepFrame > m_wMaxFrames)
                {
                    pProp->active = false;
                    return true;
                }
            }
        }

        return true;
    }

    void ZDynamicsExtend::WakeUp(uint16_t body)
    {
        SBodyProperty* pProp = &m_pProps[body];

        if (!pProp->active)
        {
            pProp->active = true;
            pProp->sleepFrame = 0;
            pProp->syncFrame = 0.f;
        }
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