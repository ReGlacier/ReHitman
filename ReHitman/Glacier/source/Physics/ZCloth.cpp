#include <Glacier/Physics/ZCloth.h>
#include <Glacier/Physics/ZCommonAlgorithms.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZCloth::ZCloth(const char* psName, ZBaseGeom* pBaseGeom)
        : ZSTDOBJ(psName, pBaseGeom),
          m_pBoxes(nullptr),
          m_pCollisionPlanes(nullptr)
    {
    }

    ZCloth::~ZCloth()
    {
        ZUniMemory::Delete(m_pBoxes);
        ZUniMemory::Delete(m_pCollisionPlanes);
        ZUniMemory::Delete(m_pCollisionPlanes2);
    }

    bool ZCloth::PostLoad(ISerializerStream& stream)
    {
        if (!m_iActorCollision)
            m_iActorCollision = HERO;

        if (!m_iMaxNrVisible)
            m_iMaxNrVisible = 1;

        return true;
    }

    const RTP::ZPropertyInfo& ZCloth::GetProperties() const
    {
        return ZCloth::Info;
    }

    uint32_t ZCloth::GetObjectId() const
    {
        return ZCloth::m_Id;
    }

    void ZCloth::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZCloth::m_Id;
        mask = ZCloth::m_Mask;
    }

    ZGEOMCLASSINFO* ZCloth::GetOldClassInfo() const
    {
        return ZCloth::m_OldClassInfo;
    }

    void ZCloth::CalcCenSize()
    {
        m_baseGeom->CalcCenSize(1);
        const float maxSize = (std::max)({m_baseGeom->m_vSize.x, m_baseGeom->m_vSize.y, m_baseGeom->m_vSize.z});
        SetSize(ZVector3(maxSize));
        SetRadius(m_baseGeom->m_vSize.Length() + 1.0f);
    }

    void ZCloth::CorrectOwnerDrawMatrix(ZMat3x3& mat, ZVector3& pos, ZBaseGeom* pOwnerBaseGeom, uint32_t boneId)
    {
        // Do nothing
    }

    bool ZCloth::WantDrawBufferControl() const
    {
        return true;
    }

    void ZCloth::DrawUpdate()
    {
        // Do nothing
    }

    bool ZCloth::DrawBufferViewUpdate(ZDrawBuffer* pDrawBuffer, ZCameraSpace* pCameraSpace)
    {
        return true;
    }

    bool ZCloth::DrawBufferViewUpdate(ZDrawBuffer* pDrawBuffer, ZBaseGeom* pOwnerBaseGeom, uint32_t boneId)
    {
        return true;
    }

    void ZCloth::HandleTensions(float* pParticles)
    {
    }

    void ZCloth::ClassInit()
    {
        Initialize(true);
    }

    int32_t ZCloth::ClassCommand(ZMSGID msg, void* pData)
    {
        if (m_msgChangeWindSpeed == msg)
        {
            m_WindSpeed = static_cast<float*>(pData);
        }
        else if (m_msgChangeElasticity == msg)
        {
            m_fElasticity = 1.0f - std::clamp(*static_cast<float*>(pData), 0.0f, 1.0f);
        }

        return 0;
    }

    void ZCloth::Initialize(bool initializeProperties)
    {
        m_OldPosition.Reset();

        if (*m_pDataBlockPtr == 1)
        {
            for (uint32_t i = 0; i < m_lNrParticles; ++i)
            {
                m_pParticlesMass[i] = 1.0f;
                m_pParticlesInvMass[i] = 1.0f / m_pParticlesMass[i];
            }

            uint32_t anchoredParticles = 0;
            for (uint32_t i = 0; i < m_lNrParticles; ++i)
            {
                ZVector3 particle(&m_pParticles[i * 3]);
                GetRootPoint(particle);

                for (uint32_t boxIndex = 0; boxIndex < m_lnrBoxes; ++boxIndex)
                {
                    ZGEOM* pBox = m_pBoxes[boxIndex];
                    ZVector3 localParticle = particle;
                    pBox->GetLocalPoint(localParticle);
                    localParticle -= pBox->BaseGeom()->m_vCen;

                    if (pBox->CheckPointInsideBound(localParticle))
                    {
                        m_pParticlesMass[i] = 1.0e31f;
                        m_pParticlesInvMass[i] = 0.0f;
                        ++anchoredParticles;
                    }
                }
            }

            for (uint32_t i = 0; i < m_lNrParticles; ++i)
            {
                ZVector3 jitter;
                ZCommonAlgorithms::RandomUnitVector(jitter.Get());
                jitter = jitter * 0.1f;

                m_pParticles[i * 3] += jitter.x;
                m_pParticles[i * 3 + 1] += jitter.y;
                m_pParticles[i * 3 + 2] += jitter.z;
            }

            for (uint32_t i = 0; i < m_lNrConstraints; ++i)
            {
                ClothConstraint& constraint = m_pConstraints[i];
                const float mass1 = m_pParticlesMass[constraint.ix1];
                const float mass2 = m_pParticlesMass[constraint.ix2];
                constraint.m_fWeightedInvMassSum = mass2 / (mass1 + mass2);
            }

            if (anchoredParticles)
                *m_pDataBlockPtr = 2;
        }

        m_lCopyListLen = 0;
        m_iOrgPrim = Prim();

        if (initializeProperties)
        {
            m_fElasticity = 1.0f - std::clamp(m_fElasticity, 0.0f, 1.0f);
            m_fDamping = std::clamp(m_fDamping, 0.0f, 1.0f);

            // TODO: Finish this place after ZHM3GameData will be reversed
            // m_rHero = g_pGameData->m_pHitman->GetRef();
        }

        m_pCollisionPlanes2 = m_lNrCollisionPlanesInAll
            ? ZUniMemory::Allocate(sizeof(float) * 15 * m_lNrCollisionPlanesInAll)
            : nullptr;
        m_fSyncTimeStep = 0.0f;
    }

#   pragma region " --- Static vars --- "
    STATIC_CLASS_VAR_IMPL(ZCloth, ZMessageResolver, m_msgChangeWindSpeed, 0x009A3BB4, {"ChangeWindSpeed"});
    STATIC_CLASS_VAR_IMPL(ZCloth, ZMessageResolver, m_msgChangeElasticity, 0x009A3BC0, {"ChangeElasticity"});
#   pragma endregion

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZCloth,
        ZSTDOBJ,
        0x009A3BB0,
        "ZCloth",
        0x00782AB4,
        nullptr, // TODO: Finish me
        0x00809940,
        0x009A3B54,
        0x009A3B58
    );
#   pragma endregion
}
