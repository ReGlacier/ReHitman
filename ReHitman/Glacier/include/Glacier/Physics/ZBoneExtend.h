#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Physics/ZDynamicsExtend.h>
#include <Glacier/Physics/ZFastBoxColi.h>


namespace Glacier
{
    class ZLNKOBJ;
    struct ZBone;

    struct SBoneMapper : SGenericMapper
    {
        // methods
        SBoneMapper();
        ~SBoneMapper();

        // members
        uint16_t m_wBone;
        uint16_t m_wPar;
        uint16_t m_wLinkBone;
        uint16_t m_wLinksCount;
        uint16_t* m_pLinks;
    };
    RE_VERIFY_SIZE(SBoneMapper, 0x10);

    struct SLinkDeformation
    {
        float m_fDeformation;
        uint16_t m_wBone;
        RE_ADD_PADDING(2);
    };
    RE_VERIFY_SIZE(SLinkDeformation, 0x8);

    struct SBoneCollision
    {
        // types
        struct SRegions
        {
            // methods
            SRegions();
            
            // members
            ZVector3 center;
            ZVector3 size;
            bool changed;
            bool enabled;
            RE_ADD_PADDING(2);
        };
        RE_VERIFY_SIZE(SRegions, 0x1C); // Verified PC alloc

        // methods
        SBoneCollision() = default;
        SBoneCollision(uint16_t bodies);
        ~SBoneCollision();

        // members
        SBoneCollision::SRegions* regions;
        ZFastBoxColi* coliBox;
        uint16_t last;
    };
    RE_VERIFY_SIZE(SBoneCollision, 0xC); // Verified PC alloc

    struct SBodyFracture
    {
        uint16_t frac_obj;
        uint16_t frac_bone;
        RE_ADD_PADDING(2);
        ZVector3 frac_vel;
    };
    RE_VERIFY_SIZE(SBodyFracture, 0x14);

    class ZBoneExtend : public ZDynamicsExtend
    {
    public:
        // vtbl
        ~ZBoneExtend() override;
        bool Create(const Fysix::PP_* ZDynamicsExtend, const SGenericMapper* mpr, uint16_t mappings) override;
        bool Update(uint16_t body, ZGEOM* geom) override;
        bool CreateMapper(const SGenericMapper* mpr, uint16_t mappings) override;
        void DestroyMapper() override;
        const REFTAB* GetCollisionFaces(uint16_t body, ZGEOM* geom) override;
        bool Collision(uint16_t body, ZGEOM* geom) override;
        bool Fracture(uint16_t body, ZGEOM* geom) override;

        // methods
        ZBoneExtend();

        bool FindCollisionRegion(uint16_t body);
        void SetCollisionRegion(uint16_t body, const ZVector3& center, const ZVector3& size);
        void RemoveCollisionRegion(uint16_t body);
        bool UpdateBones(uint16_t body, const ZLNKOBJ* linkObj, ZBone* bones);
        uint16_t GetLinks();
        bool LinkDeformations(uint16_t body, SLinkDeformation* deformations);
        bool Hit(uint16_t body);
        bool Hit(uint16_t body, const ZVector3& pos, const ZVector3& dir);
        bool Push(uint16_t body, const ZVector3& vel);
        bool Grab(uint16_t body, uint16_t& par, const ZVector3& pos, const float& radius);
        bool Grab(uint16_t body, const ZVector3& pos, uint16_t par);
        bool Release(uint16_t body, const ZVector3& pos, const float& radius);
        bool Release(uint16_t body, uint16_t par);
        bool ReleaseSystem(uint16_t body);
        float Strain(uint16_t body, uint16_t* bone);
        bool Linkage(uint16_t body, uint16_t bone, const ZBoneExtend* other, uint16_t otherBody, uint16_t otherBone);
        uint16_t FindNearestParticle(uint16_t body, const ZVector3& pos, const float& maxDist);
        uint16_t FindBone(uint16_t par);
        uint16_t FindPar(uint16_t bone);
        bool ObstacleCollision(uint16_t body, const ZGEOM* geom, const ZVector3& size);

        // members
        SBoneCollision* m_pCollision;
    };
    RE_VERIFY_SIZE(ZBoneExtend, 0x5C); // PC Verified
}