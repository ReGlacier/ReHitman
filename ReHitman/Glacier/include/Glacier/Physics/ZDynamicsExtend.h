#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Physics/Fysix/Fysix.h>
#include <Glacier/ZMessageResolver.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>


namespace Glacier
{
    struct SGenericMapper
    {
        virtual ~SGenericMapper() = default;
    };

    struct SBodyProperty
    {
        bool active;
        bool visible;
        RE_ADD_PADDING(2);
        float syncFrame;
        uint16_t sleepFrame;
        RE_ADD_PADDING(2);
    };

    class ZDynamicsExtend
    {
    public:
        // static
        STATIC_CLASS_VAR(ZDynamicsExtend, float, m_fMaxSteps);
        STATIC_CLASS_VAR(ZDynamicsExtend, ZMessageResolver, m_msgCollision);
        STATIC_CLASS_VAR(ZDynamicsExtend, ZMessageResolver, m_msgFracture);

        static constexpr uint8_t m_wMaxFrames = 0xA;
        static constexpr float m_fSyncPhysics = 0.016666666;


        // vtbl
        virtual ~ZDynamicsExtend();
        virtual bool Create(const Fysix::PP_* pp, const SGenericMapper* mpr, uint16_t mappings);
        virtual bool Init(uint16_t body, const ZGEOM* geom);
        virtual bool Move(uint16_t body, ZGEOM* geom, const float& dt);
        virtual bool Update(uint16_t body, ZGEOM* geom) = 0;
        virtual bool CreateMapper(const SGenericMapper* mpr, uint16_t mappings) = 0;
        virtual void DestroyMapper() = 0;
        virtual const REFTAB* GetCollisionFaces(uint16_t body, ZGEOM* geom) = 0;
        virtual bool Collision(uint16_t body, ZGEOM* geom) = 0;
        virtual bool Fracture(uint16_t body, ZGEOM* geom) = 0;

        // methods
        ZDynamicsExtend();

        void WakeUp(uint16_t body);
        void Stabilize(uint16_t body, uint16_t iterations);
        bool Awake(uint16_t body) const;

        // members
        SGenericMapper* m_pMapper;
        SBodyProperty* m_pProps;
        uint16_t m_wMappings;
        RE_ADD_PADDING(2);
        Fysix::ZConstraintSystem m_kConSys;
        float m_fSleepEnergy;
    };
}
