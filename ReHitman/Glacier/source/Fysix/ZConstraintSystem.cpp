#include <Glacier/Fysix/Fysix.h>


namespace Glacier::Fysix
{
    ZConstraintSystem::ZConstraintSystem()
    {
        m_pBodies = nullptr;
        m_fTimeStepSqr = 0.0016f;
        m_fInvTimeStep = 25.f;
        m_pPars = nullptr;
        m_fDamping = 1.0f;
        m_pGrps = nullptr;
        m_pConsIndex = nullptr;
        m_pConsProps = nullptr;
        m_wBodyPars = 0;
        m_wBodsCount = 0;
        m_wParsCount = 0;
        m_wGrpsCount = 0;
        m_wConsCount = 0;
        m_iBodyType = eVOID;
        m_wIterations = 1;
    }

    ZConstraintSystem::~ZConstraintSystem()
    {
        Clear();
    }
    
    void ZConstraintSystem::Clear()
    {
        // TODO: Finish me
    }

    void ZConstraintSystem::Init(const PP_& pp)
    {
        // TODO: Finish me
    }

    int ZConstraintSystem::InitGroups()
    {
        // TODO: Finish me
        return 0;
    }

    int ZConstraintSystem::InitConstraints()
    {
        // TODO: Finish me
        return 0;
    }

    bool ZConstraintSystem::InitManifold(uint16_t type)
    {
        if (m_iBodyType == eVOID)
        {
            m_iBodyType = static_cast<EManifold>(type);
            
            if (type > eVOID && type <= eSOLID)
            {
                return true;
            }
        }

        return false;
    }

    void ZConstraintSystem::Integration(const ZConstraintBody& body, const float (&moved)[3])
    {
        // TODO: Finish me
    }

    ZConstraintBody* ZConstraintSystem::GetBody(uint16_t body)
    {
        if (body >= m_wBodsCount)
        {
            return nullptr;
        } 

        return &m_pBodies[body];
    }
}