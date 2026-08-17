#pragma once

/**
 * Base part of Fysix physics engine (been best havok, still best after havok)
 */

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>


namespace Glacier::Fysix
{
    //fwds
    class REFTAB;
    class ZConstraintBody;
    class ZConstraintGroup;
    class ZConstraintSystem;

    enum EManifold
    {
        eVOID = 0,
        eCURVE = 1,
        eSURFACE = 2,
        eSOLID = 3
    };

    enum EGroupType
    {
        eNONE = 0,
        eLENGHT = 1,
        eBEND = 2,
        eBREAK = 3
    };

    struct PP_Par
    {
        float x;
        float y;
        float z;
        float m;
        float r;
        uint32_t fixed;
    };

    struct PP_Grp
    {
        uint16_t type;
        uint16_t enabled;
        float g;
        float m;
        float e;
    };

    struct PP_Con
    {
        uint32_t g;
        uint16_t x1;
        uint16_t x2;
    };

    struct PP_Bdy
    {
        float x;
        float y;
        float z;
        float r0x;
        float r0y;
        float r0z;
        float r1x;
        float r1y;
        float r1z;
        float r2x;
        float r2y;
        float r2z;
    };

    struct SLoadSavePar
    {
        float pos[3];
        float mass;
        uint32_t degree;
    };

    struct SLoadSaveCon
    {
        uint16_t index1;
        uint16_t index2;
    };

    struct SLoadSave
    {
        float ref_point[3];
        uint16_t used_pars;
        SLoadSavePar* pars;
        uint16_t own_cons;
        SLoadSaveCon* cons;
    };

    template <typename T>
    struct SLinkedObj
    {
        // methods
        SLinkedObj(SLinkedObj<T>* p)
        {
            obj = ZUniMemory::New<T>();
            p->next = this;
        }

        SLinkedObj()
        {
            obj = ZUniMemory::New<T>();
            next = nullptr;
        }

        ~SLinkedObj()
        {
            if (next)
            {
                ZUniMemory::Delete(next);
            }

            ZUniMemory::Delete(obj);
        }

        // members
        T* obj { nullptr };
        SLinkedObj<T>* next { nullptr };
    };

    struct PP_
    {
        // methods
        ~PP_();

        // members
        float fx;
        float fy;
        float fz;
        uint16_t type;
        uint16_t itors;
        float damping;
        uint16_t p_cnt;
        uint16_t g_cnt;
        uint16_t c_cnt;
        uint16_t b_cnt;
        uint32_t ghost;
        SLinkedObj<PP_Par>* particles;
        SLinkedObj<PP_Grp>* groups;
        SLinkedObj<PP_Con>* constraints;
        SLinkedObj<PP_Bdy>* bodies;
    };

    struct SConstraintIndex
    {
        uint16_t ix1;
        uint16_t ix2;
    };

    struct SConstraintProps
    {
        float restLengthSqr;
        float weightedInvMass;
    };

    struct SConstraint
    {
        SConstraintIndex indices;
        SConstraintProps property;
    };

    struct SParticle
    {
        float x[3];
        float m;
        float o[3];
        float r;
        uint32_t d;
        uint32_t Align[3];
    };

    struct SLinkage
    {
        SConstraint con;
        const ZConstraintBody* target;
    };

    struct SSimResult
    {
        float energy;
        bool collision;
        bool fracture;
        RE_ADD_PADDING(2);
    };

    class ZConstraintBody
    {
    public:
        // vtbl
        virtual ~ZConstraintBody();

        // methods
        void Save(SLoadSave& save);
        void Load(const SLoadSave& load);
        bool IsLocked(uint16_t par);
        int GetParticleMass(float& mass, uint16_t par);        
        int GetParticleRadius(float& radius, uint16_t par);
        void Linkage(const ZConstraintBody* body, uint16_t srcLinkPar, uint16_t bodyLinkPar);
        int SetReferencePosition(const float (*p0)[3]);
        int LockParticle(uint16_t par);
        int ReleaseParticle(uint16_t par);
        int ReleaseAllParticles();
        float KineticEnergy();
        int GetParticlePos(float(&pos)[3], uint16_t par);
        int GetParticlePos(float(&pos)[3], uint16_t par, float fraction);
        int MoveParticle(uint16_t par, const float(*displacement)[3], bool moveFixed, bool resetVelocity);
        void HandleCollision(const REFTAB* faces, bool penetrations, bool collisions);
        void Simulate(SSimResult& result, const float(*p0)[3], const REFTAB* faces);
        void Stabilize(bool integrate, uint16_t iterations);
        float AverageStrain(uint16_t& id);
        void FractureInfo(uint16_t& id, float(&vel)[3]);
        int GetParticleVel(float(&vel)[3], uint16_t par);
        SParticle* GetParticle(uint16_t par);

        // members
        ZConstraintSystem* m_pOwner;
        SLinkage* m_sLinkage;
        SConstraintIndex* m_pIndex;
        uint16_t m_wParsOffset;
        uint16_t m_wGhostCount;
        uint16_t m_wFractId;
        RE_ADD_PADDING(2);
        float m_FracVel[3];
        float m_LastPos[3];
    };

    class ZConstraintGroup
    {
    public:
        // vtbl
        virtual ~ZConstraintGroup();

        // methods
        void NormalSolver(const uint32_t lCount, const ZConstraintBody& body, const SConstraintProps& prop, const SConstraintIndex& idx);
        void QuickSolver(uint32_t lCount, const ZConstraintBody& body, const SConstraintProps& prop, const SConstraintIndex& idx);
        void LinkSolver(const SLinkage& con);
        void HandleFracture(const ZConstraintBody& body, uint16_t& id, float(&velocity)[3]);
        void Strain(const ZConstraintBody& body, uint16_t& amount, uint16_t& strainPar);

        // members
        ZConstraintSystem* m_pOwner;
        EGroupType m_eType;
        bool m_bEnable;
        bool m_bQuick;
        uint16_t m_wConsStart;
        uint16_t m_wConsCount;
        RE_ADD_PADDING(2);
        float m_fStrength;
        float m_fAberration;
        float m_fStrain;
        float m_fQuickAberration;
        float m_fQuickStrain;
        float m_fQuickStrength;
    };

    class ZConstraintSystem
    {
    public:
        // vtbl
        virtual ~ZConstraintSystem();

        // methods
        ZConstraintSystem();

        void Clear();
        void Init(const PP_& pp);
        int InitGroups();
        int InitConstraints();
        bool InitManifold(uint16_t type);
        void Integration(const ZConstraintBody& body, const float (&moved)[3]);
        ZConstraintBody* GetBody(uint16_t body);

        // members
        ZConstraintBody* m_pBodies;
        SParticle* m_pPars;
        ZConstraintGroup* m_pGrps;
        SConstraintIndex* m_pConsIndex;
        SConstraintProps* m_pConsProps;
        uint16_t m_wBodyPars;
        uint16_t m_wBodsCount;
        uint16_t m_wParsCount;
        uint16_t m_wGrpsCount;
        uint16_t m_wConsCount;
        RE_ADD_PADDING(2);
        float m_vPersistentForces[3];
        EManifold m_iBodyType;
        uint16_t m_wIterations;
        RE_ADD_PADDING(2);
        float m_fTimeStepSqr;
        float m_fInvTimeStep;
        float m_fDamping;
    };
}