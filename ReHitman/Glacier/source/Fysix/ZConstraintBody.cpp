#include <Glacier/Fysix/Fysix.h>

namespace Glacier::Fysix
{
    ZConstraintBody::~ZConstraintBody()
    {
        // TODO: Finish me
    }

    bool ZConstraintBody::IsLocked(uint16_t par)
    {
        // TODO: Finish me
        return false;
    }

    int ZConstraintBody::GetParticleMass(float& mass, uint16_t par)
    {
        // TODO: Finish me
        return 0;
    }

    int ZConstraintBody::GetParticleRadius(float& radius, uint16_t par)
    {
        // TODO: Finish me
        return 0;
    }
    void ZConstraintBody::Linkage(const ZConstraintBody* body, uint16_t srcLinkPar, uint16_t bodyLinkPar)
    {
        // TODO: Finish me
    }

    int ZConstraintBody::SetReferencePosition(const float (*p0)[3])
    {
        // TODO: Finish me
        return 0;
    }

    int ZConstraintBody::LockParticle(uint16_t par)
    {
        // TODO: Finish me
        return 0;
    }
    
    int ZConstraintBody::ReleaseParticle(uint16_t par)
    {
        // TODO: Finish me
        return 0;
    }

    int ZConstraintBody::ReleaseAllParticles()
    {
        // TODO: Finish me
        return 0;
    }

    float ZConstraintBody::KineticEnergy()
    {
        // TODO: Finish me
        return 0.f;
    }

    int ZConstraintBody::GetParticlePos(float(&pos)[3], uint16_t par)
    {
        auto* pParticle = GetParticle(par);
        pos[0] = pParticle->x[0];
        pos[0] = pParticle->x[1];
        pos[0] = pParticle->x[2];

        return 1;
    }

    int ZConstraintBody::GetParticlePos(float(&pos)[3], uint16_t par, float fraction)
    {
        // TODO: Finish me
        return 0;
    }

    int ZConstraintBody::MoveParticle(uint16_t par, const float(*displacement)[3], bool moveFixed, bool resetVelocity)
    {
        // TODO: Finish me
        return 0;
    }

    void ZConstraintBody::HandleCollision(const REFTAB* faces, bool penetrations, bool collisions)
    {
        // TODO: Finish me
    }
    
    void ZConstraintBody::Simulate(SSimResult& result, const float(*p0)[3], const REFTAB* faces)
    {
        // TODO: Finish me
    }
    
    void ZConstraintBody::Stabilize(bool integrate, uint16_t iterations)
    {
        // TODO: Finish me
    }
    
    float ZConstraintBody::AverageStrain(uint16_t& id)
    {
        // TODO: Finish me
        return 0.f;
    }

    void ZConstraintBody::FractureInfo(uint16_t& id, float(&vel)[3])
    {
        id = m_wFractId;
        vel[0] = m_FracVel[0];
        vel[1] = m_FracVel[1];
        vel[2] = m_FracVel[2];
    }

    int ZConstraintBody::GetParticleVel(float(&vel)[3], uint16_t par)
    {
        // TODO: Finish me
        return 0;
    }
}